// What this plugin's presets hold, for localPresets.ts
import { xmlTags } from './localPresets.ts';

export const STORAGE_KEY = 'samplerealm-sub-user-presets';

export interface PresetState {
  values: Record<string, number>;
}

export const INIT_STATE: PresetState = { values: {} };

export const parseState = (xml: string): PresetState => ({
  values: Object.fromEntries(xmlTags(xml, 'PARAM').map(({ id, value }) => [id, Number(value)])),
});
