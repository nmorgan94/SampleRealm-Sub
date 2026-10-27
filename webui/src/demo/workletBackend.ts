import { INIT_PRESET, type Level, type SubBackend, type UiInfo } from '../backend.ts';

const PRESETS_KEY = 'samplerealm-sub-presets';

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

type StoredPresets = Record<string, Record<string, number>>;

// Storage can throw, e.g. in a private window
const readPresets = (): StoredPresets => {
  try {
    return JSON.parse(localStorage.getItem(PRESETS_KEY) ?? '{}') as StoredPresets;
  } catch {
    return {};
  }
};

const writePresets = (presets: StoredPresets) => {
  try {
    localStorage.setItem(PRESETS_KEY, JSON.stringify(presets));
    return true;
  } catch {
    return false;
  }
};

export type WorkletBackend = SubBackend & {
  noteOn(note: number): void;
  noteOff(note: number): void;
  midi(bytes: ArrayLike<number>): void;
  attach(node: AudioWorkletNode): void;
};

export function workletBackend(info: UiInfo): WorkletBackend {
  const params = new Map(info.params.map((param, index) => [param.id, { param, index }]));
  const values = new Map(info.params.map((param) => [param.id, param.defaultValue]));
  const listeners = new Map<string, Set<(value: number) => void>>();
  const levelListeners = new Set<(level: Level) => void>();
  let node: AudioWorkletNode | null = null;
  // Sent once the audio starts
  let pendingMidi: number[][] = [];
  let clipUntil = 0;
  let currentPreset = INIT_PRESET;

  const entry = (id: string) => {
    const found = params.get(id);
    if (!found) throw new Error(`No parameter ${id}`);
    return found;
  };

  const notify = (id: string, value: number) => {
    values.set(id, value);
    listeners.get(id)?.forEach((listener) => listener(value));
  };

  const postParam = (id: string, value: number) => node?.port.postMessage({ type: 'param', index: entry(id).index, value });

  const setValue = (id: string, value: number) => {
    postParam(id, value);
    notify(id, value);
  };

  const applyAll = (next: Record<string, number>) => {
    for (const param of info.params) setValue(param.id, next[param.id] ?? param.defaultValue);
  };

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
    param: (id) => entry(id).param,
    getValue: (id) => values.get(id) ?? entry(id).param.defaultValue,
    setValue,
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
    presets: {
      list: async () => ({
        names: [INIT_PRESET, ...Object.keys(readPresets()).sort((a, b) => a.localeCompare(b))],
        current: currentPreset,
      }),
      load: async (name) => {
        const stored = name === INIT_PRESET ? {} : readPresets()[name];
        if (!stored) return false;
        applyAll(stored);
        currentPreset = name;
        return true;
      },
      save: async (name) => {
        if (!name || name === INIT_PRESET) return false;
        const saved = writePresets({ ...readPresets(), [name]: Object.fromEntries(values) });
        if (saved) currentPreset = name;
        return saved;
      },
      remove: async (name) => {
        const presets = readPresets();
        if (name === INIT_PRESET || !(name in presets)) return false;
        delete presets[name];
        return writePresets(presets);
      },
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
      values.forEach((value, id) => postParam(id, value));
      pendingMidi.forEach((bytes) => audioNode.port.postMessage({ type: 'midi', bytes }));
      pendingMidi = [];
    },
  };
}
