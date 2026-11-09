// Mirrors Source/ui/MasterSection.cpp
import type { SubBackend } from './backend.ts';
import { Rotary } from './controls.tsx';
import { centred, type Rect } from './layout.ts';
import { LevelMeter } from './LevelMeter.tsx';

const LABEL_WIDTH = 66;
const LABEL_GAP = 8;
const KNOB_SIZE = 44;
const METER_GAP = 10;
const METER_WIDTH = 14;
const METER_HEIGHT = 40;

export const MASTER_SECTION_WIDTH = LABEL_WIDTH + LABEL_GAP + KNOB_SIZE + METER_GAP + METER_WIDTH;

export function MasterSection({ backend, bounds }: { backend: SubBackend; bounds: Rect }) {
  const meterLeft = bounds.width - METER_WIDTH;
  const knobLeft = meterLeft - METER_GAP - KNOB_SIZE;

  return (
    <div className="absolute" style={bounds}>
      <span className="absolute top-0 left-0 text-right text-3xs text-fg-dim" style={{ width: LABEL_WIDTH, height: bounds.height, lineHeight: `${bounds.height}px` }}>
        MASTER
      </span>
      <div className="absolute" style={{ left: knobLeft, top: centred(bounds.height, KNOB_SIZE) }}>
        <Rotary backend={backend} id="masterGain" size={KNOB_SIZE} popup />
      </div>
      <LevelMeter backend={backend} left={meterLeft} top={centred(bounds.height, METER_HEIGHT)} />
    </div>
  );
}
