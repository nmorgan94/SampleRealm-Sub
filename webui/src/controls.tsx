// Mirrors Source/ui/CustomLookAndFeel.cpp and its controls
import { useId, useRef, useState, type ComponentProps, type PointerEvent as ReactPointerEvent, type ReactNode } from 'react';
import type { ParamInfo, SubBackend } from './backend.ts';
import { useParam, useShowValue } from './hooks.ts';
import { formatValue, fromProportion, parseValue, snap, toProportion } from './params.ts';

// JUCE's defaults
const ROTARY_START = -0.8 * Math.PI;
const ROTARY_SWEEP = 1.6 * Math.PI;
const PIXELS_FOR_FULL_DRAG = 250;
const FINE_DRAG = 4;

const STEPPER_PIXELS_PER_STEP = 3;

const pointAt = (cx: number, cy: number, radius: number, angle: number) =>
  [cx + radius * Math.sin(angle), cy - radius * Math.cos(angle)] as const;

const arcPath = (cx: number, cy: number, radius: number, from: number, to: number) => {
  const [x1, y1] = pointAt(cx, cy, radius, from);
  const [x2, y2] = pointAt(cx, cy, radius, to);
  return `M ${x1} ${y1} A ${radius} ${radius} 0 ${to - from > Math.PI ? 1 : 0} 1 ${x2} ${y2}`;
};

function KnobGraphic({ size, proportion }: { size: number; proportion: number }) {
  const id = useId();
  const centre = size / 2;
  const radius = (size - 8) / 2;
  const ring = radius * 0.16;
  const knobRadius = radius - ring - radius * 0.14;
  const angle = ROTARY_START + proportion * ROTARY_SWEEP;

  const segments = 24;
  const gap = 0.3;
  const span = ROTARY_SWEEP / segments;
  const highlight = knobRadius * 0.7;
  const [inX1, inY1] = pointAt(centre, centre, knobRadius * 0.15, angle);
  const [inX2, inY2] = pointAt(centre, centre, knobRadius * 0.82, angle);

  return (
    <svg width={size} height={size} viewBox={`0 0 ${size} ${size}`} aria-hidden>
      <defs>
        <linearGradient id={`${id}body`} x1="0" y1="0" x2="0" y2="1">
          <stop offset="0" stopColor="var(--color-panel-bright)" />
          <stop offset="1" stopColor="var(--color-panel-dark)" />
        </linearGradient>
        <linearGradient id={`${id}shine`} gradientUnits="userSpaceOnUse" x1={centre} y1={centre - knobRadius * 0.6} x2={centre} y2={centre}>
          <stop offset="0" stopColor="#fff" stopOpacity="0.08" />
          <stop offset="1" stopColor="#fff" stopOpacity="0" />
        </linearGradient>
      </defs>
      {Array.from({ length: segments }, (_, i) => {
        const from = ROTARY_START + span * (i + gap / 2);
        const to = ROTARY_START + span * (i + 1 - gap / 2);
        const lit = (i + 0.5) / segments <= proportion;
        const d = arcPath(centre, centre, radius, from, to);
        return (
          <g key={i}>
            {lit && <path d={d} stroke="var(--color-accent)" strokeOpacity={0.22} strokeWidth={ring + 5} fill="none" />}
            <path d={d} stroke={lit ? 'var(--color-accent)' : 'var(--color-panel-border)'} strokeWidth={ring} fill="none" />
          </g>
        );
      })}
      <circle cx={centre} cy={centre} r={knobRadius} fill={`url(#${id}body)`} />
      {Array.from({ length: 32 }, (_, i) => {
        const a = (2 * Math.PI * i) / 32;
        const [x1, y1] = pointAt(centre, centre, knobRadius * 0.86, a);
        const [x2, y2] = pointAt(centre, centre, knobRadius * 0.98, a);
        return <line key={i} x1={x1} y1={y1} x2={x2} y2={y2} stroke="var(--color-panel-border)" strokeOpacity={0.35} strokeWidth={1} />;
      })}
      <circle cx={centre} cy={centre} r={knobRadius} fill="none" stroke="var(--color-panel-border)" strokeWidth={1.2} />
      <ellipse cx={centre} cy={centre - knobRadius * 0.9 + highlight / 2} rx={highlight} ry={highlight / 2} fill={`url(#${id}shine)`} />
      <line x1={inX1} y1={inY1} x2={inX2} y2={inY2} stroke="var(--color-fg)" strokeWidth={2.2} />
      <circle cx={inX2} cy={inY2} r={2.6} fill="var(--color-accent)" />
    </svg>
  );
}

type DragStart = { x: number; y: number; value: number };

interface ParamDrag {
  valueAt: (e: ReactPointerEvent, start: DragStart, param: ParamInfo) => number;
  // Moves to the pointer on press
  jumps?: boolean;
  onStart?: () => void;
  onEnd?: () => void;
}

function useParamDrag(backend: SubBackend, id: string, { valueAt, jumps, onStart, onEnd }: ParamDrag) {
  const drag = useRef<DragStart | null>(null);
  const state = useParam(backend, id);
  const { value, set, param } = state;

  const follow = (e: ReactPointerEvent, start: DragStart) => {
    const next = valueAt(e, start, param);
    if (next !== value) set(next);
  };

  const end = () => {
    if (!drag.current) return;
    drag.current = null;
    backend.endGesture(id);
    onEnd?.();
  };

  const handlers = {
    onPointerDown: (e: ReactPointerEvent) => {
      if (e.button !== 0) return;
      e.currentTarget.setPointerCapture(e.pointerId);
      drag.current = { x: e.clientX, y: e.clientY, value };
      backend.beginGesture(id);
      onStart?.();
      if (jumps) follow(e, drag.current);
    },
    onPointerMove: (e: ReactPointerEvent) => drag.current && follow(e, drag.current),
    onPointerUp: end,
    onPointerCancel: end,
  };

  const aria = {
    'aria-label': param.name,
    'aria-valuemin': param.min,
    'aria-valuemax': param.max,
    'aria-valuenow': value,
    'aria-valuetext': formatValue(param, value),
  };

  return { ...state, handlers, aria };
}

const rotaryValueAt = (e: ReactPointerEvent, start: DragStart, param: ParamInfo) => {
  const moved = e.clientX - start.x - (e.clientY - start.y);
  const travel = moved / (PIXELS_FOR_FULL_DRAG * (e.shiftKey ? FINE_DRAG : 1));
  return fromProportion(param, toProportion(param, start.value) + travel);
};

function Bubble({ text, visible }: { text: string; visible: boolean }) {
  return visible ? (
    <div className="pointer-events-none absolute bottom-full left-1/2 z-10 mb-1 -translate-x-1/2 rounded-sm border border-panel-border bg-panel px-1.5 py-0.5 font-sans text-xs whitespace-nowrap">
      {text}
    </div>
  ) : null;
}

interface RotaryProps {
  backend: SubBackend;
  id: string;
  size: number;
  popup?: boolean;
  resetTo?: number;
  onDragStart?: () => void;
  onDragEnd?: () => void;
}

export function Rotary({ backend, id, size, popup, resetTo, onDragStart, onDragEnd }: RotaryProps) {
  const [dragging, setDragging] = useState(false);
  const { value, commit, param, handlers, aria } = useParamDrag(backend, id, {
    valueAt: rotaryValueAt,
    onStart: () => {
      setDragging(true);
      onDragStart?.();
    },
    onEnd: () => {
      setDragging(false);
      onDragEnd?.();
    },
  });

  return (
    <div
      className="relative cursor-ns-resize touch-none"
      role="slider"
      {...aria}
      style={{ width: size, height: size }}
      {...handlers}
      onDoubleClick={resetTo === undefined ? undefined : () => commit(resetTo)}
    >
      <KnobGraphic size={size} proportion={toProportion(param, value)} />
      {popup && <Bubble text={formatValue(param, value)} visible={dragging} />}
    </div>
  );
}

interface LabeledCellProps {
  backend: SubBackend;
  id: string;
  width: number;
  height: number;
  control: (area: { width: number; height: number }, show: { start: () => void; end: () => void }) => ReactNode;
  accessory?: { node: ReactNode; width: number; height: number };
}

const LABEL_HEIGHT = 16;
const LABEL = 'h-4 text-center text-2xs leading-4 whitespace-nowrap';

export function LabeledCell({ backend, id, width, height, control, accessory }: LabeledCellProps) {
  const { value, commit, param } = useParam(backend, id);
  const show = useShowValue();
  const [editing, setEditing] = useState(false);

  const commitText = (text: string) => {
    setEditing(false);
    const parsed = parseValue(param, text);
    if (parsed !== null) commit(parsed);
  };

  const areaHeight = height - LABEL_HEIGHT - (accessory?.height ?? 0);

  return (
    <div className="flex flex-col" style={{ width, height }}>
      <div className="flex items-center justify-center" style={{ height: areaHeight }} onDoubleClick={() => setEditing(true)}>
        {control({ width, height: areaHeight }, show)}
      </div>
      {editing ? (
        <input
          className={`${LABEL} w-full border border-accent bg-panel outline-none`}
          autoFocus
          defaultValue={formatValue(param, value)}
          onFocus={(e) => e.currentTarget.select()}
          onBlur={(e) => commitText(e.currentTarget.value)}
          onKeyDown={(e) => {
            if (e.key === 'Enter') e.currentTarget.blur();
            if (e.key === 'Escape') setEditing(false);
          }}
        />
      ) : (
        <div className={`${LABEL} text-fg-dim`}>{show.showing ? formatValue(param, value) : param.name}</div>
      )}
      {accessory && (
        <div className="flex items-center justify-center" style={{ height: accessory.height }}>
          <div style={{ width: accessory.width, height: accessory.height }}>{accessory.node}</div>
        </div>
      )}
    </div>
  );
}

const INCDEC = 'flex-1 cursor-pointer border border-accent bg-panel font-sans text-sm text-accent hover:bg-panel-hover';

export function IncDecButtons({ backend, id, width, onChange }: { backend: SubBackend; id: string; width: number; onChange: () => void }) {
  const { value, commit, param } = useParam(backend, id);

  const step = (direction: number) => {
    const next = snap(param, value + direction * (param.step || 1));
    if (next === value) return;
    commit(next);
    onChange();
  };

  return (
    <div className="flex" style={{ width, height: 70 }}>
      <button type="button" className={`${INCDEC} rounded-l-md`} aria-label={`${param.name} down`} onClick={() => step(-1)} onDoubleClick={(e) => e.stopPropagation()}>
        -
      </button>
      <button type="button" className={`${INCDEC} rounded-r-md border-l-0`} aria-label={`${param.name} up`} onClick={() => step(1)} onDoubleClick={(e) => e.stopPropagation()}>
        +
      </button>
    </div>
  );
}

const LINEAR_INDENT = 12;

export function LinearSlider({ backend, id, width, height, onDragStart, onDragEnd }: {
  backend: SubBackend;
  id: string;
  width: number;
  height: number;
  onDragStart: () => void;
  onDragEnd: () => void;
}) {
  const trackWidth = width - LINEAR_INDENT * 2;
  const { value, param, handlers, aria } = useParamDrag(backend, id, {
    valueAt: (e, _, param) => fromProportion(param, (e.clientX - e.currentTarget.getBoundingClientRect().left - LINEAR_INDENT) / trackWidth),
    jumps: true,
    onStart: onDragStart,
    onEnd: onDragEnd,
  });
  const position = LINEAR_INDENT + toProportion(param, value) * trackWidth;
  const centre = width / 2;

  return (
    <div
      className="relative cursor-ew-resize touch-none"
      role="slider"
      {...aria}
      style={{ width, height }}
      {...handlers}
    >
      <div className="absolute inset-x-3 top-1/2 h-1 -translate-y-1/2 rounded-xs bg-panel-border" />
      <div className="absolute top-1/2 h-3.5 w-px -translate-y-1/2 bg-fg-dim" style={{ left: centre - 0.5 }} />
      <div className="absolute top-1/2 h-2 -translate-y-1/2 bg-accent/20" style={{ left: Math.min(centre, position), width: Math.abs(position - centre) }} />
      <div className="absolute top-1/2 h-1 -translate-y-1/2 bg-accent" style={{ left: Math.min(centre, position), width: Math.abs(position - centre) }} />
      <div
        className="absolute top-1/2 h-7 w-3.5 -translate-y-1/2 rounded-sm border border-panel-border bg-linear-to-b from-panel-bright to-panel-dark"
        style={{ left: position - 7 }}
      >
        <div className="absolute top-1/2 left-1/2 h-3.5 w-0.5 -translate-1/2 bg-accent" />
      </div>
    </div>
  );
}

export function PillToggle({ backend, id }: { backend: SubBackend; id: string }) {
  const { value, commit, param } = useParam(backend, id);
  const on = value >= 0.5;

  return (
    <button
      type="button"
      className={`size-full cursor-pointer rounded-full border font-display text-3xs hover:brightness-110 ${on ? 'border-accent bg-accent text-background' : 'border-panel-border bg-panel text-fg-dim'}`}
      aria-label={param.name}
      aria-pressed={on}
      onClick={() => commit(on ? 0 : 1)}
      onDoubleClick={(e) => e.stopPropagation()}
    >
      {param.choices[on ? 1 : 0]?.toUpperCase()}
    </button>
  );
}

export function Stepper({ backend, id, label }: { backend: SubBackend; id: string; label: string }) {
  const { value, commit, param, handlers, aria } = useParamDrag(backend, id, {
    valueAt: (e, start, param) => snap(param, start.value - Math.trunc((e.clientY - start.y) / STEPPER_PIXELS_PER_STEP)),
  });

  return (
    <div
      className="flex size-full cursor-ns-resize touch-none rounded-sm border border-panel-border bg-panel"
      role="spinbutton"
      {...aria}
      {...handlers}
      onDoubleClick={() => commit(param.defaultValue)}
    >
      <span className="flex flex-1 items-center justify-center text-3xs text-fg-dim">{label}</span>
      <span className="flex flex-1 items-center justify-center text-2xs">{Math.round(value)}</span>
    </div>
  );
}

export function TextButton({ className = '', ...props }: ComponentProps<'button'>) {
  return (
    <button
      type="button"
      className={`min-w-11 cursor-pointer rounded-md border border-panel-border bg-panel px-1.5 font-sans text-xs font-normal text-fg hover:bg-panel-hover ${className}`}
      {...props}
    />
  );
}
