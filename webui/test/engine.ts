// SUB_WASM from CTest, else the newest local build
import { readFileSync } from 'node:fs';
import { newestEngine } from '../localEngine.ts';
import type { ParamInfo } from '../src/backend.ts';
import { readUiInfo, stubImports, type Engine } from '../src/demo/workletBackend.ts';

const instance = new WebAssembly.Instance(new WebAssembly.Module(readFileSync(process.env.SUB_WASM ?? newestEngine())), stubImports());

export const engine = instance.exports as Engine;
engine._initialize?.();

export const info = readUiInfo(engine);

export const param = (id: string): ParamInfo => {
  const found = info.params.find((p) => p.id === id);
  if (!found) throw new Error(`No parameter ${id}`);
  return found;
};
