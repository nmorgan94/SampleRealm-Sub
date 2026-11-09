import { paramLookup, type Level, type SubBackend, type UiInfo } from '../backend.ts';
import { localPresets, type PresetStorage } from './localPresets.ts';
import { INIT_STATE, parseState, STORAGE_KEY, type PresetState } from './presetState.ts';

// About the plugin's 30-block clip hold
const CLIP_HOLD_MS = 400;

const NOTE_ON = 0x90;
const NOTE_OFF = 0x80;
const PITCH_WHEEL = 0xe0;
const VELOCITY = 100;

// WASI imports are never called
export const stubImports = () => {
  const stub = new Proxy({}, { get: () => () => 0 });
  return new Proxy({}, { get: () => stub }) as WebAssembly.Imports;
};

export type Engine = Record<string, CallableFunction> & { memory: WebAssembly.Memory };

export const readUiInfo = (engine: Engine) => {
  const bytes = new Uint8Array(engine.memory.buffer, engine.uiInfo());
  return JSON.parse(new TextDecoder().decode(bytes.subarray(0, bytes.indexOf(0)))) as UiInfo;
};

export async function loadEngine(url: string): Promise<{ wasm: ArrayBuffer; info: UiInfo }> {
  const response = await fetch(url);
  if (!response.ok) throw new Error(`${url}: ${response.status}`);

  const wasm = await response.arrayBuffer();
  const { instance } = await WebAssembly.instantiate(wasm, stubImports());
  const engine = instance.exports as Engine;
  engine._initialize?.();
  return { wasm, info: readUiInfo(engine) };
}

export type WorkletBackend = SubBackend & {
  noteOn(note: number): void;
  noteOff(note: number): void;
  midi(bytes: ArrayLike<number>): void;
  attach(node: AudioWorkletNode): void;
};

export function workletBackend(info: UiInfo, factoryPresets: string[] = [], storage?: PresetStorage): WorkletBackend {
  const params = new Map(info.params.map((param, index) => [param.id, { param, index, value: param.defaultValue }]));
  const entry = paramLookup(params);
  const listeners = new Map<string, Set<(value: number) => void>>();
  const levelListeners = new Set<(level: Level) => void>();
  let node: AudioWorkletNode | null = null;
  // Sent once the audio starts
  let pendingMidi: number[][] = [];
  let clipUntil = 0;

  const notify = (id: string, value: number) => {
    entry(id).value = value;
    listeners.get(id)?.forEach((listener) => listener(value));
  };

  const postParam = (id: string, value: number) => node?.port.postMessage({ type: 'param', index: entry(id).index, value });

  const apply = (id: string, value: number) => {
    postParam(id, value);
    notify(id, value);
  };

  const presets = localPresets<PresetState>({
    storageKey: STORAGE_KEY,
    storage,
    factory: factoryPresets,
    parse: parseState,
    init: INIT_STATE,
    capture: () => ({
      values: Object.fromEntries(info.params.filter(({ isPerformanceControl }) => !isPerformanceControl).map(({ id }) => [id, entry(id).value])),
    }),
    // Ids this engine doesn't have are skipped, as the APVTS skips them
    apply: ({ values }) => info.params.forEach(({ id, defaultValue }) => apply(id, values[id] ?? defaultValue)),
  });

  const midi = (bytes: ArrayLike<number>) => {
    const [status, data1 = 0, data2 = 0] = Array.from(bytes);
    if (node) node.port.postMessage({ type: 'midi', bytes: [status, data1, data2] });
    else pendingMidi.push([status, data1, data2]);

    // As PitchBendInput: the engine has the bend, this moves the on-screen wheel
    if ((status & 0xf0) === PITCH_WHEEL) {
      const offset = ((data2 << 7) | data1) - 8192;
      notify('pitchBend', offset / (offset < 0 ? 8192 : 8191));
    }
  };

  return {
    info,
    running: 'Web demo',
    presets,
    param: (id) => entry(id).param,
    getValue: (id) => entry(id).value,
    setValue: (id, value) => {
      apply(id, value);
      if (!entry(id).param.isPerformanceControl) presets.markModified();
    },
    beginGesture: () => {},
    endGesture: () => {},
    subscribe: (id, listener) => {
      const set = listeners.get(id) ?? new Set();
      listeners.set(id, set.add(listener));
      return () => set.delete(listener);
    },
    onLevel: (listener) => {
      levelListeners.add(listener);
      return () => levelListeners.delete(listener);
    },
    noteOn: (note) => midi([NOTE_ON, note, VELOCITY]),
    noteOff: (note) => midi([NOTE_OFF, note, 0]),
    midi,
    attach: (audioNode) => {
      node = audioNode;
      node.port.onmessage = ({ data }) => {
        if (data.type !== 'level') return;
        if (data.peak > 1) clipUntil = performance.now() + CLIP_HOLD_MS;
        const level = { peak: data.peak, clipping: performance.now() < clipUntil };
        levelListeners.forEach((listener) => listener(level));
      };
      // The engine starts at the plugin's defaults
      params.forEach(({ value }, id) => postParam(id, value));
      pendingMidi.forEach((bytes) => audioNode.port.postMessage({ type: 'midi', bytes }));
      pendingMidi = [];
    },
  };
}
