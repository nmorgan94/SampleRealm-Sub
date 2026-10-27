// Mirrors Source/ui/PitchWheel.cpp
import { useEffect, useId, useRef, useState } from 'react';
import type { SubBackend } from './backend.ts';
import { Stepper } from './controls.tsx';

const BEND = 'pitchBend';
const RANGE = 'pitchBendRange';

const SPRING_HZ = 60;
const SPRING_DECAY_PER_TICK = 0.45;
const SPRING_SETTLE = 0.001;

const TRACK_WIDTH = 20;
const THUMB_HEIGHT = 12;
const NAME_HEIGHT = 16;
const STEPPER_WIDTH = 96;
const STEPPER_HEIGHT = 20;
const STEPPER_GAP = 4;

export function PitchWheel({ backend, width, height }: { backend: SubBackend; width: number; height: number }) {
  const id = useId();
  const [bend, setBend] = useState(() => backend.getValue(BEND));
  const drag = useRef<{ y: number; bend: number } | null>(null);
  const spring = useRef<number | undefined>(undefined);
  const bendRef = useRef(bend);

  const wheelHeight = height - NAME_HEIGHT - STEPPER_HEIGHT - STEPPER_GAP;
  const travel = (wheelHeight - THUMB_HEIGHT) / 2;
  const busy = () => drag.current !== null || spring.current !== undefined;

  const send = (value: number) => {
    bendRef.current = value;
    setBend(value);
    backend.setValue(BEND, value);
  };

  // Ignored while held or springing back
  useEffect(() => backend.subscribe(BEND, (value) => !busy() && setBend((bendRef.current = value))), [backend]);

  // Otherwise closing mid-drag leaves the bend off-centre
  useEffect(
    () => () => {
      if (!busy()) return;
      window.clearInterval(spring.current);
      backend.setValue(BEND, 0);
      backend.endGesture(BEND);
    },
    [backend]
  );

  const springBack = () => {
    spring.current = window.setInterval(() => {
      let next = bendRef.current * SPRING_DECAY_PER_TICK;
      const settled = Math.abs(next) < SPRING_SETTLE;
      if (settled) next = 0;
      send(next);

      if (settled) {
        window.clearInterval(spring.current);
        spring.current = undefined;
        backend.endGesture(BEND);
      }
    }, 1000 / SPRING_HZ);
  };

  const end = () => {
    if (!drag.current) return;
    drag.current = null;
    springBack();
  };

  const trackX = (width - TRACK_WIDTH) / 2;
  const thumbY = wheelHeight / 2 - bend * travel - THUMB_HEIGHT / 2;

  return (
    <div className="flex flex-col items-center" style={{ width, height }}>
      <svg
        width={width}
        height={wheelHeight}
        className="cursor-ns-resize touch-none"
        role="slider"
        aria-label="Pitch Bend"
        aria-valuemin={-1}
        aria-valuemax={1}
        aria-valuenow={bend}
        onPointerDown={(e) => {
          if (e.button !== 0) return;
          e.currentTarget.setPointerCapture(e.pointerId);
          // Continues the open gesture
          const wasSpringing = spring.current !== undefined;
          window.clearInterval(spring.current);
          spring.current = undefined;
          drag.current = { y: e.clientY, bend: bendRef.current };
          if (!wasSpringing) backend.beginGesture(BEND);
        }}
        onPointerMove={(e) => {
          if (!drag.current) return;
          const draggedUp = drag.current.y - e.clientY;
          send(Math.min(1, Math.max(-1, drag.current.bend + draggedUp / travel)));
        }}
        onPointerUp={end}
        onPointerCancel={end}
      >
        <defs>
          <linearGradient id={`${id}track`} x1="0" y1="0" x2="0" y2="1">
            <stop offset="0" stopColor="var(--color-panel-bright)" />
            <stop offset="1" stopColor="var(--color-panel-dark)" />
          </linearGradient>
        </defs>
        <rect x={trackX} y={0} width={TRACK_WIDTH} height={wheelHeight} rx={TRACK_WIDTH / 2} fill={`url(#${id}track)`} />
        <rect x={trackX + 0.5} y={0.5} width={TRACK_WIDTH - 1} height={wheelHeight - 1} rx={TRACK_WIDTH / 2} fill="none" stroke="var(--color-panel-border)" />
        <line x1={trackX} y1={wheelHeight / 2} x2={trackX + TRACK_WIDTH} y2={wheelHeight / 2} stroke="var(--color-panel-border)" />
        <rect x={trackX} y={thumbY} width={TRACK_WIDTH} height={THUMB_HEIGHT} rx={THUMB_HEIGHT * 0.4} fill="var(--color-accent)" />
      </svg>
      <div style={{ width: STEPPER_WIDTH, height: STEPPER_HEIGHT, marginTop: STEPPER_GAP }}>
        <Stepper backend={backend} id={RANGE} label="RANGE" />
      </div>
      <div className="h-4 text-2xs leading-4 text-fg-dim">Pitch</div>
    </div>
  );
}
