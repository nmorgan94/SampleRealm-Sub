import { engineAt } from '@samplerealm/modules/testing/engine.ts';
import { localEnginePath } from '../localEngine.ts';
import { paramLookup } from '../src/backend.ts';

const monoEngine = engineAt(localEnginePath());

// Makes a fresh engine from the last wasm build, prepared for its one channel, or unprepared for null
export const engine = (rate?: number | null) => monoEngine(1, rate);

export const { info } = engine();

export const param = paramLookup(new Map(info.params.map((p) => [p.id, p])));
