import assert from 'node:assert/strict';
import { test } from 'node:test';
import { fromProportion, toProportion } from '@samplerealm/modules/editor/params.ts';
import { parseText, valueText } from '../src/oscMixText.ts';
import { param } from './engine.ts';

// From a Catch2 dump of the plugin's getText
const pluginText: [id: string, proportion: number, text: string][] = [
  ['masterGain', 0.5, '-27.0'],
  ['oscOctave', 0.77, '-1'],
  ['osc1Fine', 0.13, '-37.0'],
  ['oscMix', 0.13, '87 / 13'],
  ['envAttack', 0.13, '0.007'],
  ['envAttack', 0.77, '2.093'],
  ['envSustain', 0.77, '0.770'],
  ['glideTime', 0.5, '0.099'],
  ['glideMode', 0.77, 'Rate'],
  ['glideMode', 0.13, 'Time'],
  ['pitchBend', 0.77, '0.5400'],
  ['pitchBendRange', 0.5, '12'],
];

test('every value reads as the plugin shows it, at the same knob position', () => {
  for (const [id, proportion, text] of pluginText) {
    assert.equal(valueText(param(id), fromProportion(param(id), proportion)), text, `${id} at ${proportion}`);
  }
});

test('skewed knobs map as JUCE NormalisableRange does, both ways', () => {
  const release = param('envRelease');

  assert.equal(fromProportion(release, 0.5), 0.497);
  assert.ok(Math.abs(toProportion(release, fromProportion(release, 0.5)) - 0.5) < 0.001);
});

test('positions past either end clamp to the range', () => {
  const drive = param('saturationDrive');

  assert.equal(fromProportion(drive, 1.5), 1);
  assert.equal(fromProportion(drive, -0.5), 0);
});

test('typed text parses, the Osc Mix split included, and text without a number is ignored', () => {
  assert.equal(parseText(param('oscMix'), '30 / 70'), 0.7);
  assert.equal(parseText(param('oscMix'), 'half'), null);
  assert.equal(parseText(param('envRelease'), '0.5'), 0.5);
  assert.equal(parseText(param('envRelease'), '99'), 5);
  assert.equal(parseText(param('envRelease'), 'loud'), null);
});
