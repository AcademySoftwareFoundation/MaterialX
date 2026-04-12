// Cross-platform clean and copy of build artifacts for testing.
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const testRoot = path.resolve(__dirname, '..');
const buildDir = path.join(testRoot, '_build');
const srcDir = path.resolve(testRoot, '..', 'build', 'bin');

fs.rmSync(buildDir, { recursive: true, force: true });
fs.mkdirSync(buildDir, { recursive: true });

for (const file of fs.readdirSync(srcDir))
{
    if (file.startsWith('JsMaterialX'))
    {
        fs.copyFileSync(path.join(srcDir, file), path.join(buildDir, file));
    }
}
