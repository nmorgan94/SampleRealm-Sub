import { browserStorage, type PresetStorage } from '@samplerealm/modules/demo/localPresets.ts';
import { workletBackend, type WorkletBackend } from '@samplerealm/modules/demo/workletBackend.ts';
import type { UiInfo } from '../backend.ts';

// This plugin's, as a site's demos share its storage. Unchanged, so saved presets stay
export const STORAGE_KEY = 'samplerealm-sub-user-presets';

const PITCH_WHEEL = 0xe0;

interface Options {
  // The factory preset files' text
  factoryPresets?: string[];
  // The page's own unless given
  storage?: PresetStorage;
}

// As PitchBendInput: the engine bends from the MIDI itself, and the on-screen wheel follows it
export function demoBackend(info: UiInfo, { factoryPresets = [], storage = browserStorage() }: Options = {}): WorkletBackend {
  const backend = workletBackend(info, { storageKey: STORAGE_KEY, factoryPresets, storage });

  return {
    ...backend,
    midi: (bytes) => {
      backend.midi(bytes);

      if (((bytes[0] ?? 0) & 0xf0) !== PITCH_WHEEL) return;
      const offset = (((bytes[2] ?? 0) << 7) | (bytes[1] ?? 0)) - 8192;
      backend.showValue('pitchBend', offset / (offset < 0 ? 8192 : 8191));
    },
  };
}
