// Mirrors Source/ui/EnvelopeVisualizer.cpp
import { useId } from 'react';
import type { SubBackend } from './backend.ts';
import { driveEnvelopeModulation, shapeProgress, timeToWidthFraction } from './curves.ts';
import { useParam } from './hooks.ts';

const SUSTAIN_FRACTION = 0.3;
const STEPS_PER_SEGMENT = 24;

export function EnvelopeView({ backend, width, height }: { backend: SubBackend; width: number; height: number }) {
  const id = useId();
  const attack = useParam(backend, 'envAttack').value;
  const decay = useParam(backend, 'envDecay').value;
  const sustain = useParam(backend, 'envSustain').value;
  const release = useParam(backend, 'envRelease').value;
  const attackCurve = useParam(backend, 'envAttackCurve').value;
  const decayCurve = useParam(backend, 'envDecayCurve').value;
  const releaseCurve = useParam(backend, 'envReleaseCurve').value;
  const drive = useParam(backend, 'saturationDrive').value;
  const driveEnv = useParam(backend, 'saturationDriveEnv').value;

  const fractions = [timeToWidthFraction(attack), timeToWidthFraction(decay), SUSTAIN_FRACTION, timeToWidthFraction(release)];
  const total = fractions.reduce((sum, f) => sum + f, 0);
  const xs = fractions.reduce((acc, f) => [...acc, acc[acc.length - 1] + (width * f) / total], [0]);

  const points = [
    { x: xs[0], env: 0, curve: 0 },
    { x: xs[1], env: 1, curve: attackCurve },
    { x: xs[2], env: sustain, curve: decayCurve },
    { x: xs[3], env: sustain, curve: 0 },
    { x: xs[4], env: 0, curve: releaseCurve },
  ];

  const path = (levelForEnv: (env: number) => number) => {
    const y = (env: number) => height - levelForEnv(env) * height;
    let d = `M ${points[0].x} ${y(points[0].env)}`;

    for (let i = 1; i < points.length; i++) {
      for (let step = 1; step <= STEPS_PER_SEGMENT; step++) {
        const fraction = step / STEPS_PER_SEGMENT;
        const shaped = shapeProgress(fraction, points[i].curve);
        const x = points[i - 1].x + (points[i].x - points[i - 1].x) * fraction;
        d += ` L ${x} ${y(points[i - 1].env + (points[i].env - points[i - 1].env) * shaped)}`;
      }
    }

    return d;
  };

  const outline = path((env) => env);

  return (
    <svg width={width} height={height} className="pointer-events-none absolute inset-0" aria-hidden>
      <defs>
        <linearGradient id={`${id}fill`} x1="0" y1="0" x2="0" y2="1">
          <stop offset="0" stopColor="var(--color-accent)" stopOpacity="0.18" />
          <stop offset="1" stopColor="var(--color-accent)" stopOpacity="0" />
        </linearGradient>
      </defs>
      <path d={`${outline} L ${width} ${height} L 0 ${height} Z`} fill={`url(#${id}fill)`} />
      <path d={outline} fill="none" stroke="var(--color-accent)" strokeOpacity={0.4} strokeWidth={1.5} />
      <path
        d={path((env) => drive * driveEnvelopeModulation(env, driveEnv))}
        fill="none"
        stroke="var(--color-fg)"
        strokeOpacity={0.35}
        strokeWidth={1}
        strokeDasharray="4 4"
      />
    </svg>
  );
}
