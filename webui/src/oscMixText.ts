// Mirrors Parameters::withValueText: Osc Mix reads "70 / 30", the rest as the shared params.ts
import { formatValue, parseValue, snap } from '@samplerealm/modules/editor/params.ts';
import type { ParamInfo } from './backend.ts';

export const valueText = (param: ParamInfo, value: number) => {
  if (param.id !== 'oscMix') return formatValue(param, value);

  const osc2Percent = Math.round(value * 100);
  return `${100 - osc2Percent} / ${osc2Percent}`;
};

export const parseText = (param: ParamInfo, text: string) => {
  if (param.id !== 'oscMix') return parseValue(param, text);

  const osc2Percent = parseFloat(text.slice(text.lastIndexOf('/') + 1));
  return Number.isNaN(osc2Percent) ? null : snap(param, osc2Percent / 100);
};
