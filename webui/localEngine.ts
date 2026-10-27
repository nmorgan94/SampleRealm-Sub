import { readdirSync, statSync } from 'node:fs';
import { resolve } from 'node:path';

const ENGINE_DIR = resolve(import.meta.dirname, '../wasm/build-wasm');

// Older versions stay in the build folder
export const newestEngine = () => {
  const [wasm] = readdirSync(ENGINE_DIR)
    .filter((file) => file.endsWith('.wasm'))
    .map((file) => resolve(ENGINE_DIR, file))
    .sort((a, b) => statSync(b).mtimeMs - statSync(a).mtimeMs);
  if (!wasm) throw new Error('Build wasm/ first: cmake --workflow --preset wasm');
  return wasm;
};
