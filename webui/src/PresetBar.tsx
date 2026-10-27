// Mirrors Source/ui/PresetComboBox.cpp
import { useEffect, useState } from 'react';
import { INIT_PRESET, type SubBackend } from './backend.ts';
import { TextButton } from './controls.tsx';

const HEADING = 'mb-2 text-base font-bold';
const TEXT = 'mb-3 text-sm text-fg-dim';

type Dialog =
  | { kind: 'save'; name: string }
  | { kind: 'delete'; name: string }
  | { kind: 'failed'; title: string; message: string };

export function PresetBar({ backend }: { backend: SubBackend }) {
  const [current, setCurrent] = useState(INIT_PRESET);
  const [menu, setMenu] = useState<string[] | null>(null);
  const [dialog, setDialog] = useState<Dialog | null>(null);

  useEffect(() => {
    backend.presets.list().then(({ current }) => setCurrent(current || INIT_PRESET));
  }, [backend]);

  const outcome = (succeeded: boolean, name: string, action: string, pastTense: string) => {
    if (succeeded) setCurrent(name);
    else setDialog({ kind: 'failed', title: `Couldn't ${action} Preset`, message: `"${name}" couldn't be ${pastTense}.` });
  };

  const openMenu = async () => setMenu((await backend.presets.list()).names);

  const load = async (name: string) => {
    setMenu(null);
    outcome(await backend.presets.load(name), name, 'Load', 'loaded');
  };

  const save = async (name: string) => {
    setDialog(null);
    const trimmed = name.trim();
    if (trimmed) outcome(await backend.presets.save(trimmed), trimmed, 'Save', 'saved');
  };

  const remove = async (name: string) => {
    setDialog(null);
    await backend.presets.remove(name);
    await backend.presets.load(INIT_PRESET);
    setCurrent(INIT_PRESET);
  };

  return (
    <div className="absolute top-4.5 left-[274px] flex h-6 gap-1.5 font-sans font-normal">
      <button
        type="button"
        className="flex w-32.5 cursor-pointer items-center rounded-sm border border-panel-border bg-panel pl-1.5 text-left text-sm"
        onClick={openMenu}
        aria-haspopup="listbox"
      >
        <span className="flex-1 truncate">{current}</span>
        <svg className="mx-2.5 flex-none" width="10" height="6" viewBox="0 0 10 6" aria-hidden>
          <path d="M 1 1 L 5 5 L 9 1" fill="none" stroke="var(--color-accent)" strokeWidth="2" />
        </svg>
      </button>
      <TextButton onClick={() => setDialog({ kind: 'save', name: current === INIT_PRESET ? '' : current })}>Save</TextButton>
      <TextButton onClick={() => current.trim() && current !== INIT_PRESET && setDialog({ kind: 'delete', name: current })}>Delete</TextButton>

      {menu && (
        <>
          <div className="fixed inset-0 z-20" onPointerDown={() => setMenu(null)} />
          <ul className="absolute top-6.5 left-0 z-21 max-h-75 w-32.5 overflow-y-auto border border-panel-border bg-panel py-1" role="listbox">
            {menu.map((name) => (
              <li key={name} className="cursor-pointer px-2.5 py-0.5 text-sm hover:bg-accent/30" role="option" aria-selected={name === current} onClick={() => load(name)}>
                {name}
              </li>
            ))}
          </ul>
        </>
      )}

      {dialog && (
        <div className="fixed inset-0 z-20 flex items-center justify-center bg-black/45">
          <form
            className="min-w-70 rounded-md border border-panel-border bg-panel p-4"
            onSubmit={(e) => {
              e.preventDefault();
              if (dialog.kind === 'save') save(new FormData(e.currentTarget).get('presetName')?.toString() ?? '');
              else if (dialog.kind === 'delete') remove(dialog.name);
              else setDialog(null);
            }}
          >
            {dialog.kind === 'save' && (
              <>
                <h2 className={HEADING}>Save Preset</h2>
                <p className={TEXT}>Enter a name for this preset:</p>
                <input
                  className="mb-3 w-full rounded-sm border border-panel-border bg-background px-1.5 py-1 text-sm"
                  name="presetName"
                  defaultValue={dialog.name}
                  autoFocus
                  onFocus={(e) => e.currentTarget.select()}
                />
              </>
            )}
            {dialog.kind === 'delete' && (
              <>
                <h2 className={HEADING}>Delete Preset</h2>
                <p className={TEXT}>{`Delete preset "${dialog.name}"? This can't be undone.`}</p>
              </>
            )}
            {dialog.kind === 'failed' && (
              <>
                <h2 className={HEADING}>{dialog.title}</h2>
                <p className={TEXT}>{dialog.message}</p>
              </>
            )}
            <div className="flex justify-end gap-1.5">
              <TextButton type="submit" className="h-6.5" autoFocus={dialog.kind !== 'save'}>
                {dialog.kind === 'save' ? 'Save' : dialog.kind === 'delete' ? 'Delete' : 'OK'}
              </TextButton>
              {dialog.kind !== 'failed' && (
                <TextButton className="h-6.5" onClick={() => setDialog(null)}>
                  Cancel
                </TextButton>
              )}
            </div>
          </form>
        </div>
      )}
    </div>
  );
}
