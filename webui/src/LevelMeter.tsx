// Mirrors Source/ui/LevelMeter.cpp
import { useEffect, useState } from 'react';
import type { Level, SubBackend } from './backend.ts';

const toNormalised = (peak: number) => {
  const db = peak > 0 ? Math.max(-60, 20 * Math.log10(peak)) : -60;
  return Math.min(1, Math.max(0, (db + 60) / 60));
};

export function LevelMeter({ backend }: { backend: SubBackend }) {
  const [level, setLevel] = useState<Level>({ peak: 0, clipping: false });

  useEffect(() => backend.onLevel(setLevel), [backend]);

  return (
    <div className="absolute top-2.5 left-[666px] flex h-10 w-3.5 flex-col gap-1" aria-hidden>
      <div className={`h-1.5 rounded-xs ${level.clipping ? 'bg-[red]' : 'bg-panel-border'}`} />
      <div className="relative flex-1 overflow-hidden rounded-xs border border-panel-border bg-panel">
        <div className="absolute inset-x-0 bottom-0 rounded-xs bg-accent" style={{ height: `${toNormalised(level.peak) * 100}%` }} />
      </div>
    </div>
  );
}
