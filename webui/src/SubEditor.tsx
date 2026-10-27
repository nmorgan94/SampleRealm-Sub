// Mirrors AudioPluginAudioProcessorEditor::resized()
import { memo, type ReactNode } from 'react';
import type { SubBackend } from './backend.ts';
import { IncDecButtons, LabeledCell, LinearSlider, PillToggle, Rotary } from './controls.tsx';
import { EnvelopeView } from './EnvelopeView.tsx';
import { LevelMeter } from './LevelMeter.tsx';
import { PitchWheel } from './PitchWheel.tsx';
import { PresetBar } from './PresetBar.tsx';

export const EDITOR_WIDTH = 700;
export const EDITOR_HEIGHT = 640;

const TITLE_BAR_HEIGHT = 60;
const ROW_HEADER_HEIGHT = 22;
const ROW_TITLES = ['OSCILLATOR', 'ENVELOPE', 'TONE & PITCH'];

const ROW_HEIGHT = Math.floor((EDITOR_HEIGHT - TITLE_BAR_HEIGHT - 32) / 3);
const ROW_X = 16;
const ROW_WIDTH = EDITOR_WIDTH - 32;
const rowY = (row: number) => TITLE_BAR_HEIGHT + 16 + row * ROW_HEIGHT + 4;
const PANEL_HEIGHT = ROW_HEIGHT - 8;

const CONTENT_WIDTH = ROW_WIDTH - 20;
const CONTENT_HEIGHT = PANEL_HEIGHT - ROW_HEADER_HEIGHT - 8;
const CELL_WIDTH = Math.floor(CONTENT_WIDTH / 4) - 12;
const CELL_HEIGHT = CONTENT_HEIGHT - 12;

const CURVE_KNOB_SIZE = 32;
const PILL_WIDTH = 52;
const PILL_HEIGHT = 15;

function Cell({ column, children }: { column: number; children: ReactNode }) {
  return (
    <div className="absolute" style={{ left: column * Math.floor(CONTENT_WIDTH / 4) + 6, top: 6 }}>
      {children}
    </div>
  );
}

function Row({ row, children, background }: { row: number; children: ReactNode; background?: ReactNode }) {
  return (
    <section className="absolute rounded-xl border border-panel-border bg-panel/60" style={{ left: ROW_X, top: rowY(row), width: ROW_WIDTH, height: PANEL_HEIGHT }}>
      <h2 className="flex items-center px-3.5 text-2xs text-fg-dim" style={{ height: ROW_HEADER_HEIGHT }}>
        {ROW_TITLES[row]}
      </h2>
      <div className="relative mx-2.5 my-1" style={{ width: CONTENT_WIDTH, height: CONTENT_HEIGHT }}>
        {background}
        {children}
      </div>
    </section>
  );
}

// The demo re-renders on every key press
export const SubEditor = memo(function SubEditor({ backend }: { backend: SubBackend }) {
  const knob = (id: string, column: number, accessory?: { node: ReactNode; width: number; height: number }) => (
    <Cell column={column}>
      <LabeledCell
        backend={backend}
        id={id}
        width={CELL_WIDTH}
        height={CELL_HEIGHT}
        control={(area, show) => (
          <Rotary backend={backend} id={id} size={Math.min(area.width, area.height)} onDragStart={show.start} onDragEnd={show.end} />
        )}
        accessory={accessory}
      />
    </Cell>
  );

  const curveKnob = (id: string) => ({
    node: <Rotary backend={backend} id={id} size={CURVE_KNOB_SIZE} popup resetTo={0} />,
    width: CURVE_KNOB_SIZE,
    height: CURVE_KNOB_SIZE,
  });

  return (
    <div className="relative overflow-hidden bg-linear-to-b from-background-top to-background-bottom font-display font-bold text-fg select-none" style={{ width: EDITOR_WIDTH, height: EDITOR_HEIGHT }}>
      <header className="absolute inset-x-0 top-0 after:absolute after:inset-x-0 after:top-[59px] after:h-0.5 after:bg-accent/60" style={{ height: TITLE_BAR_HEIGHT }}>
        <h1 className="absolute top-0 left-5 h-15 text-lg leading-15">SampleRealm: SUB</h1>
        <PresetBar backend={backend} />
        <span className="absolute left-[538px] h-15 w-[66px] text-right text-3xs leading-15 text-fg-dim">MASTER</span>
        <div className="absolute top-2 left-[612px]">
          <Rotary backend={backend} id="masterGain" size={44} popup />
        </div>
        <LevelMeter backend={backend} />
      </header>

      <Row row={0}>
        <Cell column={0}>
          <LabeledCell
            backend={backend}
            id="oscOctave"
            width={CELL_WIDTH}
            height={CELL_HEIGHT}
            control={(area, show) => (
              <IncDecButtons
                backend={backend}
                id="oscOctave"
                width={area.width}
                onChange={() => {
                  show.start();
                  show.end();
                }}
              />
            )}
          />
        </Cell>
        {knob('osc1Fine', 1)}
        <Cell column={2}>
          <LabeledCell
            backend={backend}
            id="oscMix"
            width={CELL_WIDTH}
            height={CELL_HEIGHT}
            control={(area, show) => (
              <LinearSlider backend={backend} id="oscMix" width={area.width} height={area.height} onDragStart={show.start} onDragEnd={show.end} />
            )}
          />
        </Cell>
        {knob('osc2Fine', 3)}
      </Row>

      <Row row={1} background={<EnvelopeView backend={backend} width={CONTENT_WIDTH} height={CONTENT_HEIGHT} />}>
        {knob('envAttack', 0, curveKnob('envAttackCurve'))}
        {knob('envDecay', 1, curveKnob('envDecayCurve'))}
        {knob('envSustain', 2)}
        {knob('envRelease', 3, curveKnob('envReleaseCurve'))}
      </Row>

      <Row row={2}>
        <Cell column={0}>
          <PitchWheel backend={backend} width={CELL_WIDTH} height={CELL_HEIGHT} />
        </Cell>
        {knob('saturationDrive', 1)}
        {knob('saturationDriveEnv', 2)}
        {knob('glideTime', 3, { node: <PillToggle backend={backend} id="glideMode" />, width: PILL_WIDTH, height: PILL_HEIGHT })}
      </Row>

      <span className="absolute right-2.5 bottom-3 text-3xs font-normal text-[#00d9ff]/40">{`v${backend.info.version}`}</span>
    </div>
  );
});
