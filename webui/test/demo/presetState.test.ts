// This plugin's presets in the demo: its state, and what counts as an edit
import assert from 'node:assert/strict';
import { readdirSync, readFileSync } from 'node:fs';
import { test } from 'node:test';
import { parsePreset } from '../../src/demo/localPresets.ts';
import { parseState, STORAGE_KEY } from '../../src/demo/presetState.ts';
import { workletBackend } from '../../src/demo/workletBackend.ts';
import { presetActions } from '../../src/presetActions.ts';
import type { Prompt } from '../../src/PromptOverlay.tsx';
import { info, param } from '../engine.ts';

const storageWith = (stored: unknown) => {
  const items = new Map([[STORAGE_KEY, JSON.stringify(stored)]]);
  return { getItem: (key: string) => items.get(key) ?? null, setItem: (key: string, value: string) => void items.set(key, value) };
};

test('a stored preset loads past ids this engine doesn\'t have, and Init restores the defaults', async () => {
  const { presets, getValue, param } = workletBackend(info, [], storageWith([{ name: 'Other', category: '', values: { cutoff: 1, masterGain: -3, envAttack: 1.5 } }]));

  await presets.load({ name: 'Other', category: '', isFactory: false });
  assert.equal(getValue('masterGain'), -3);
  assert.equal(getValue('envAttack'), 1.5);
  assert.equal(presets.status().current.name, 'Other');

  await presets.loadInit();
  assert.equal(getValue('masterGain'), param('masterGain').defaultValue);
  assert.equal(getValue('envAttack'), param('envAttack').defaultValue);
  assert.deepEqual(presets.status(), { current: { name: 'Init', category: '', isFactory: true }, modified: false });
});

// As Tests/PresetTests.cpp
test('the pitch wheel, from MIDI or on screen, doesn\'t mark the preset modified, but a control does', () => {
  const backend = workletBackend(info);

  backend.midi([0xe0, 0x7f, 0x7f]);
  assert.equal(backend.getValue('pitchBend'), 1);
  assert.equal(backend.presets.status().modified, false);

  backend.setValue('pitchBend', -0.5);
  assert.equal(backend.presets.status().modified, false);

  backend.setValue('pitchBendRange', 12);
  assert.equal(backend.presets.status().modified, true);
});

// As Tests/PresetTests.cpp
test('a saved preset leaves out pitch bend, so loading it centres the wheel', async () => {
  const storage = storageWith([]);
  const { presets, setValue, getValue } = workletBackend(info, [], storage);

  setValue('pitchBend', 0.5);
  await presets.save('Bent', '');
  const [saved] = JSON.parse(storage.getItem(STORAGE_KEY)!) as { values: Record<string, number> }[];
  assert.equal(saved!.values.pitchBend, undefined);
  assert.equal(saved!.values.pitchBendRange, param('pitchBendRange').defaultValue);

  await presets.load({ name: 'Bent', category: '', isFactory: false });
  assert.equal(getValue('pitchBend'), 0);
});

// As Tests/PresetTests.cpp
test('choosing a preset that won\'t load says so, and keeps the current one', async () => {
  const storage = storageWith([]);
  const { presets } = workletBackend(info, [], storage);
  const messages: string[] = [];
  const prompt: Prompt = { askForText: () => {}, askToConfirm: () => {}, showMessage: (_title, message) => void messages.push(message) };
  const actions = presetActions(presets, prompt);

  await presets.save('A', '');
  await presets.save('B', '');
  const listed = await presets.list();
  const [a, b] = [listed.find(({ name }) => name === 'A')!, listed.find(({ name }) => name === 'B')!];

  await actions.load(a);
  assert.deepEqual(messages, []);

  const otherTab = workletBackend(info, [], storage).presets;
  assert.equal(await otherTab.remove(b), null);
  await presets.list();

  await actions.load(b);
  assert.deepEqual(messages, ['"B" couldn\'t be loaded.']);
  assert.equal(presets.status().current.name, 'A');
});

test('every shipped factory preset sets every parameter but the performance controls in range, and loads unmodified', async () => {
  const folder = new URL('../../../Assets/Presets/', import.meta.url);
  const files = readdirSync(folder).filter((file) => file.endsWith('.sub')).map((file) => readFileSync(new URL(file, folder), 'utf8'));
  assert.notEqual(files.length, 0);
  const { presets, getValue } = workletBackend(info, files);

  for (const xml of files) {
    const { name, category, values } = parsePreset(xml, parseState);

    await presets.load({ name, category, isFactory: true });
    assert.deepEqual(presets.status(), { current: { name, category, isFactory: true }, modified: false });

    for (const { id, min, max, defaultValue, isPerformanceControl } of info.params) {
      if (isPerformanceControl) {
        assert.equal(values[id], undefined, `${name}: ${id}`);
        assert.equal(getValue(id), defaultValue, `${name}: ${id}`);
        continue;
      }

      const value = values[id];
      assert.ok(value !== undefined && value >= min && value <= max, `${name}: ${id} is ${value}`);
      assert.equal(getValue(id), value, `${name}: ${id}`);
    }
  }
});
