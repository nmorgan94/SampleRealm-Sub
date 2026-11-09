type ParamKind = 'real' | 'integer' | 'choice';

export interface ParamInfo {
  id: string;
  name: string;
  kind: ParamKind;
  min: number;
  max: number;
  defaultValue: number;
  step: number;
  skew: number;
  unit: string;
  choices: string[];
  isPerformanceControl: boolean;
}

export interface UiInfo {
  version: string;
  commit: string;
  build: 'Debug' | 'Release';
  params: ParamInfo[];
}

export interface Level {
  peak: number;
  clipping: boolean;
}

export interface PresetEntry {
  name: string;
  category: string;
  isFactory: boolean;
}

export interface PresetStatus {
  current: PresetEntry;
  modified: boolean;
}

// Mirrors Source/presets/PresetManager.h
export interface PresetBackend {
  status(): PresetStatus;
  onStatus(listener: (status: PresetStatus) => void): () => void;
  // Factory then user presets, rescanning the user ones
  list(): Promise<PresetEntry[]>;
  // The current preset if it's listed, without a rescan
  current(): Promise<PresetEntry | null>;
  // Resolves to whether it loaded
  load(preset: PresetEntry): Promise<boolean>;
  loadInit(): Promise<void>;
  loadNext(): Promise<void>;
  loadPrevious(): Promise<void>;
  exists(name: string): Promise<boolean>;
  // Each resolves to an error message, or null
  save(name: string, category: string): Promise<string | null>;
  rename(preset: PresetEntry, newName: string): Promise<string | null>;
  remove(preset: PresetEntry): Promise<string | null>;
  // Only where presets are files
  showFolder?(): void;
}

export const isSamePreset = (a: PresetEntry, b: PresetEntry) => a.name === b.name && a.isFactory === b.isFactory;

export interface SubBackend {
  info: UiInfo;
  // "VST3 in Ableton Live", or where else the editor is running
  running: string;
  presets: PresetBackend;
  param(id: string): ParamInfo;
  // Real values, not 0..1
  getValue(id: string): number;
  setValue(id: string, value: number): void;
  beginGesture(id: string): void;
  endGesture(id: string): void;
  subscribe(id: string, listener: (value: number) => void): () => void;
  onLevel(listener: (level: Level) => void): () => void;
}

export const paramLookup = <T>(entries: Map<string, T>) => (id: string) => {
  const found = entries.get(id);
  if (!found) throw new Error(`No parameter ${id}`);
  return found;
};

export const listenerSet = <Args extends unknown[]>() => {
  const listeners = new Set<(...args: Args) => void>();
  return {
    add: (listener: (...args: Args) => void) => {
      listeners.add(listener);
      return () => void listeners.delete(listener);
    },
    notify: (...args: Args) => listeners.forEach((listener) => listener(...args)),
  };
};
