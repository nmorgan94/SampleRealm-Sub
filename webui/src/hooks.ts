import { useCallback, useEffect, useRef, useState } from 'react';
import type { SubBackend } from './backend.ts';

const SHOW_VALUE_MS = 600;

export function useParam(backend: SubBackend, id: string) {
  const [value, setLocal] = useState(() => backend.getValue(id));

  useEffect(() => {
    setLocal(backend.getValue(id));
    return backend.subscribe(id, setLocal);
  }, [backend, id]);

  const set = useCallback(
    (next: number) => {
      setLocal(next);
      backend.setValue(id, next);
    },
    [backend, id]
  );

  const commit = useCallback(
    (next: number) => {
      backend.beginGesture(id);
      set(next);
      backend.endGesture(id);
    },
    [backend, id, set]
  );

  return { value, set, commit, param: backend.param(id) };
}

export function useShowValue() {
  const [showing, setShowing] = useState(false);
  const timer = useRef<number | undefined>(undefined);

  const start = useCallback(() => {
    window.clearTimeout(timer.current);
    setShowing(true);
  }, []);

  const end = useCallback(() => {
    window.clearTimeout(timer.current);
    timer.current = window.setTimeout(() => setShowing(false), SHOW_VALUE_MS);
  }, []);

  useEffect(() => () => window.clearTimeout(timer.current), []);

  return { showing, start, end };
}
