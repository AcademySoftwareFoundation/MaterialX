import { defineConfig } from '@playwright/test';

// The webgpu-smoke project needs a built MaterialXView dist and a WebGPU-capable GPU, so it is
// opt-in: run `npm run test:webgpu-smoke`, or set MX_WEBGPU_SMOKE=1 for other entry points
// (`npx playwright test`, UI mode, the VS Code extension). npm sets npm_lifecycle_event to the
// running script's name on every platform, which avoids both parsing --project and setting env
// vars in the script (`VAR=1 cmd` doesn't work in cmd.exe). Without the opt-in the project has
// no tests: a plain `npx playwright test` leaves it out, and selecting it explicitly fails with
// "No tests found" instead of passing silently. The spec answers the viewer's requests from
// dist/ itself, so no server is started.
const WEBGPU_SMOKE_PROJECT = 'webgpu-smoke';
if (process.env.npm_lifecycle_event === 'test:webgpu-smoke')
{
    process.env.MX_WEBGPU_SMOKE = '1';
}
const runsWebGpuSmoke = process.env.MX_WEBGPU_SMOKE === '1';
// Playwright's "No tests found" can't be customized, so explain the opt-in when the project is
// named on the command line. This only affects the message, never whether the tests run. Workers
// re-evaluate the config, so only the main process prints it.
if (!runsWebGpuSmoke && !process.env.TEST_WORKER_INDEX &&
    process.argv.some((arg) => arg.includes(WEBGPU_SMOKE_PROJECT)))
{
    console.warn(`${WEBGPU_SMOKE_PROJECT} is opt-in: run \`npm run test:webgpu-smoke\` or set ` +
        'MX_WEBGPU_SMOKE=1 (after building MaterialXView), otherwise the project has no tests.');
}

export default defineConfig({
    // Stage the WASM build artifacts into _build before any project runs.
    globalSetup: './scripts/buildSetup.js',
    // Write test artifacts (and .last-run.json) under _build, which is already
    // gitignored and removed by clean_javascript_win.bat.
    outputDir: './_build/test-results',
    timeout: 120_000,
    projects: [
        {
            // Node-side unit tests for the JsMaterialXCore bindings. These run in
            // the Playwright worker process (Node) and never launch a browser.
            name: 'unit',
            testDir: '.',
            testIgnore: ['browser/**'],
        },
        {
            // Browser-based shader generation tests, which exercise the WebGL/WASM
            // build inside a real Chromium page.
            name: 'browser',
            testDir: './browser',
            testIgnore: ['**/webgpuViewerSmoke.spec.js'],
            use: {
                browserName: 'chromium',
                launchOptions: {
                    args: ['--enable-gpu', '--enable-webgl']
                }
            }
        },
        {
            // WebGPU MaterialXView smoke: deep-linked materials + screenshots. Opt-in (see above).
            name: WEBGPU_SMOKE_PROJECT,
            testDir: './browser',
            testMatch: 'webgpuViewerSmoke.spec.js',
            testIgnore: runsWebGpuSmoke ? [] : ['**/*'],
            use: {
                browserName: 'chromium',
                // The default headless shell cannot create a WebGPU device (no dxil.dll),
                // which makes three.js silently fall back to WebGL2.
                channel: 'chromium',
                viewport: { width: 1280, height: 720 },
                launchOptions: {
                    args: [
                        '--enable-gpu',
                        '--enable-unsafe-webgpu',
                    ],
                },
            },
        },
    ]
});
