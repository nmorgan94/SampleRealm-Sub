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
}

export interface UiInfo {
  version: string;
  params: ParamInfo[];
}

export interface Level {
  peak: number;
  clipping: boolean;
}

export interface Presets {
  list(): Promise<{ names: string[]; current: string }>;
  load(name: string): Promise<boolean>;
  save(name: string): Promise<boolean>;
  remove(name: string): Promise<boolean>;
}

export interface SubBackend {
  info: UiInfo;
  param(id: string): ParamInfo;
  // Real values, not 0..1
  getValue(id: string): number;
  setValue(id: string, value: number): void;
  beginGesture(id: string): void;
  endGesture(id: string): void;
  subscribe(id: string, listener: (value: number) => void): () => void;
  onLevel(listener: (level: Level) => void): () => void;
  presets: Presets;
}

export const INIT_PRESET = 'Init';
