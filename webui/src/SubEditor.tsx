// Mirrors AudioPluginAudioProcessorEditor::resized()
import { memo, useState, type ReactNode } from 'react';
import type { SubBackend } from './backend.ts';
import { IncDecButtons, LabeledCell, LinearSlider, PillToggle, Rotary } from './controls.tsx';
import { EditorRootContext } from './editorRoot.ts';
import { EnvelopeView } from './EnvelopeView.tsx';
import { HeaderBar, type HeaderLayout } from './HeaderBar.tsx';
import { useManual } from './ManualOverlay.tsx';
import { MASTER_SECTION_WIDTH, MasterSection } from './MasterSection.tsx';
import { PitchWheel } from './PitchWheel.tsx';
import { usePrompt } from './PromptOverlay.tsx';

export const EDITOR_WIDTH = 700;
export const EDITOR_HEIGHT = 640;

// AudioPluginAudioProcessorEditor::headerLayout
const HEADER_LAYOUT: HeaderLayout = {
  height: 60,
  edge: 20,
  titleWidth: 200,
  titleHeight: 22,
  gap: 20,
  controlsHeight: 24,
  meterWidth: MASTER_SECTION_WIDTH,
  meterInset: 8,
};

const ROW_HEADER_HEIGHT = 22;
const ROW_TITLES = ['OSCILLATOR', 'ENVELOPE', 'TONE & PITCH'];

const ROW_HEIGHT = Math.floor((EDITOR_HEIGHT - HEADER_LAYOUT.height - 32) / 3);
const ROW_X = 16;
const ROW_WIDTH = EDITOR_WIDTH - 32;
const rowY = (row: number) => HEADER_LAYOUT.height + 16 + row * ROW_HEIGHT + 4;
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
  const [root, setRoot] = useState<HTMLDivElement | null>(null);
  const { prompt, overlay } = usePrompt();
  const manual = useManual();

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
    <EditorRootContext.Provider value={root}>
      <div ref={setRoot} className="relative overflow-hidden bg-linear-to-b from-background-top to-background-bottom font-display font-bold text-fg select-none" style={{ width: EDITOR_WIDTH, height: EDITOR_HEIGHT }}>
        <div className="absolute inset-x-0 h-0.5 bg-accent/60" style={{ top: HEADER_LAYOUT.height - 1 }} />
        <HeaderBar
          title="SampleRealm: SUB"
          width={EDITOR_WIDTH}
          layout={HEADER_LAYOUT}
          backend={backend}
          prompt={prompt}
          showManual={manual.showManual}
          meter={(bounds) => <MasterSection backend={backend} bounds={bounds} />}
        />

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
        {overlay}
        {manual.overlay}
      </div>
    </EditorRootContext.Provider>
  );
});
