import { createRoot } from 'react-dom/client';
import { pluginBackend } from '@samplerealm/modules/plugin/pluginBackend.ts';
import { SubEditor } from '../SubEditor.tsx';
import './styles.css';

const root = createRoot(document.getElementById('root')!);

try {
  root.render(<SubEditor backend={pluginBackend()} />);
} catch (error) {
  console.error(error);
  root.render(<pre className="p-5 text-fg">{`The editor couldn't connect to the plugin.\n${error}`}</pre>);
}
