import assert from 'node:assert/strict';
import { test } from 'node:test';
import { workletBackend } from '../../src/demo/workletBackend.ts';
import { info } from '../engine.ts';

test('a MIDI pitch wheel moves the on-screen wheel across its full range, as in the plugin', () => {
  const backend = workletBackend(info);
  const bends: number[] = [];
  backend.subscribe('pitchBend', (value) => bends.push(value));

  backend.midi([0xe0, 0x00, 0x00]);
  backend.midi([0xe3, 0x00, 0x40]);
  backend.midi([0xe0, 0x7f, 0x7f]);

  assert.deepEqual(bends, [-1, 0, 1]);
  assert.equal(backend.getValue('pitchBend'), 1);
});

test('other MIDI leaves the wheel alone', () => {
  const backend = workletBackend(info);
  let moved = false;
  backend.subscribe('pitchBend', () => (moved = true));

  backend.noteOn(45);
  backend.midi([0xb0, 1, 127]);

  assert.equal(moved, false);
});
