import assert from 'node:assert/strict';
import { test } from 'node:test';
import { driveEnvelopeModulation, shapeProgress } from '../src/curves.ts';
import { engine } from './engine.ts';

// The C++ uses floats
const close = (a: number, b: number) => Math.abs(a - b) < 1e-5;
const grid = Array.from({ length: 21 }, (_, i) => i / 20);
const tensions = [-1, -0.5, -0.0005, 0, 0.3, 1];

test('the envelope drawing bends each stage exactly as the DSP does', () => {
  for (const progress of grid) {
    for (const tension of tensions) {
      const expected = engine.shapeProgress(progress, tension) as number;
      assert.ok(close(shapeProgress(progress, tension), expected), `progress ${progress}, tension ${tension}`);
    }
  }
});

test('the drawn drive curve follows the envelope exactly as the DSP does', () => {
  for (const env of grid) {
    for (const amount of [-1, -0.4, 0, 0.6, 1]) {
      const expected = engine.driveEnvelopeModulation(env, amount) as number;
      assert.ok(close(driveEnvelopeModulation(env, amount), expected), `env ${env}, amount ${amount}`);
    }
  }
});
