// This plugin's presets in the demo: its pitch bend and its shipped files. The rest is tested in SampleRealm-Modules
import assert from 'node:assert/strict';
import { readdirSync, readFileSync } from 'node:fs';
import { test } from 'node:test';
import { parsePreset } from '@samplerealm/modules/demo/localPresets.ts';
import { parseState } from '@samplerealm/modules/demo/workletBackend.ts';
import { memoryStorage } from '@samplerealm/modules/testing/presets.ts';
import { demoBackend, STORAGE_KEY } from '../../src/demo/demoBackend.ts';
import { info, param } from '../engine.ts';

// As Tests/PresetTests.cpp
test('the pitch wheel, from MIDI or on screen, doesn\'t mark the preset modified, but a control does', () => {
  const backend = demoBackend(info, { storage: memoryStorage() });

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
  const storage = memoryStorage();
  const { presets, setValue, getValue } = demoBackend(info, { storage });

  setValue('pitchBend', 0.5);
  await presets.save('Bent', '');
  const [saved] = JSON.parse(storage.getItem(STORAGE_KEY)!) as { values: Record<string, number> }[];
  assert.equal(saved!.values.pitchBend, undefined);
  assert.equal(saved!.values.pitchBendRange, param('pitchBendRange').defaultValue);

  await presets.load({ name: 'Bent', category: '', isFactory: false });
  assert.equal(getValue('pitchBend'), 0);
});

test('every shipped factory preset sets every parameter but the performance controls in range, and loads unmodified', async () => {
  const folder = new URL('../../../Assets/Presets/', import.meta.url);
  const files = readdirSync(folder).filter((file) => file.endsWith('.sub')).map((file) => readFileSync(new URL(file, folder), 'utf8'));
  assert.notEqual(files.length, 0);
  const { presets, getValue } = demoBackend(info, { factoryPresets: files, storage: memoryStorage() });

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
