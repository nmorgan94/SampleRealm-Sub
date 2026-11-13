// The engine's note behaviour, which only these tests cover
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { blockSize, sampleRate } from '@samplerealm/modules/testing/engine.ts';
import { engine as freshEngine } from './engine.ts';

const noteOn = 0x90;
const noteOff = 0x80;
const pitchWheel = 0xe0;

function engine() {
  const { engine: exports, set, renderBlocks } = freshEngine();

  // A pure sine, so zero crossings give the pitch
  set('oscOctave', 0);
  set('saturationDrive', 0);

  const play = (seconds: number) => Array.from(renderBlocks(Math.round(seconds * sampleRate))[0]!);

  return { set, midi: exports.midi, play };
}

const peak = (samples: number[]) => Math.max(...samples.map(Math.abs));

const frequency = (samples: number[]) => {
  let rising = 0;
  for (let i = 1; i < samples.length; i++) if (samples[i - 1]! < 0 && samples[i]! >= 0) rising++;
  return rising / (samples.length / sampleRate);
};

test('the ui info carries the version and every parameter as ParameterSpecs defines it', () => {
  const { info } = freshEngine();

  assert.match(info.version, /^\d+\.\d+\.\d+$/);
  assert.equal(new Set(info.params.map((p) => p.id)).size, info.params.length);
  assert.deepEqual(info.params.find((p) => p.id === 'envRelease'), {
    id: 'envRelease', name: 'Release', kind: 'real', min: 0.001, max: 5, defaultValue: 0.2,
    step: 0.001, skew: 0.3, unit: 's', choices: [],
  });
  assert.deepEqual(info.params.find((p) => p.id === 'glideMode')!.choices, ['Time', 'Rate']);
  assert.deepEqual(info.params.filter((p) => p.isPerformanceControl).map((p) => p.id), ['pitchBend']);
});

test('a held note sounds, finite and within full scale, an octave down with drive', () => {
  const e = engine();
  e.set('oscOctave', -1);
  e.set('saturationDrive', 0.4);
  e.midi(noteOn, 45, 100);

  const out = e.play(0.5);

  assert.ok(out.every(Number.isFinite));
  assert.ok(peak(out) > 0.1, `peak ${peak(out)}`);
  assert.ok(peak(out) <= 1, `peak ${peak(out)}`);
});

test('a note plays at its MIDI pitch', () => {
  const e = engine();
  e.midi(noteOn, 69, 100);

  assert.ok(Math.abs(frequency(e.play(0.5)) - 440) < 5);
});

test('a legato second note glides to its pitch instead of jumping', () => {
  const e = engine();
  e.set('glideTime', 0.4);
  e.midi(noteOn, 57, 100);
  e.play(0.2);
  e.midi(noteOn, 69, 100);

  const midGlide = frequency(e.play(0.4).slice(0.15 * sampleRate, 0.25 * sampleRate));
  const settled = frequency(e.play(0.5));

  assert.ok(midGlide > 240 && midGlide < 420, `mid-glide ${midGlide} Hz`);
  assert.ok(Math.abs(settled - 440) < 5, `settled ${settled} Hz`);
});

test('releasing the newer of two held notes returns to the older one', () => {
  const e = engine();
  e.midi(noteOn, 57, 100);
  e.midi(noteOn, 69, 100);
  e.play(0.2);
  e.midi(noteOff, 69, 0);

  assert.ok(Math.abs(frequency(e.play(0.5)) - 220) < 5);
});

test('releasing the only held note fades to silence within the release time', () => {
  const e = engine();
  e.set('envRelease', 0.2);
  e.midi(noteOn, 45, 100);
  e.play(0.3);
  e.midi(noteOff, 45, 0);

  const out = e.play(0.4);

  assert.ok(peak(out.slice(0, blockSize)) > 0.1);
  assert.equal(peak(out.slice(0.25 * sampleRate)), 0);
});

test('the pitch wheel at full up bends by the default two semitones', () => {
  const e = engine();
  e.midi(noteOn, 69, 100);
  e.midi(pitchWheel, 0x7f, 0x7f);
  e.play(0.1);

  const bent = 440 * 2 ** (2 / 12);
  assert.ok(Math.abs(frequency(e.play(0.5)) - bent) < 5);
});

test('moving the pitch bend parameter bends like the wheel', () => {
  const e = engine();
  e.midi(noteOn, 69, 100);
  e.set('pitchBend', -1);
  e.play(0.1);

  const bent = 440 * 2 ** (-2 / 12);
  assert.ok(Math.abs(frequency(e.play(0.5)) - bent) < 5);
});
