import { defineConfig } from '@playwright/test';

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
            use: {
                browserName: 'chromium',
                launchOptions: {
                    args: ['--enable-gpu', '--enable-webgl']
                }
            }
        }
    ]
});
