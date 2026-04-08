import { defineConfig } from '@playwright/test';

export default defineConfig({
    testDir: './browser',
    timeout: 120_000,
    use: {
        browserName: 'chromium',
        launchOptions: {
            args: ['--enable-gpu', '--enable-webgl']
        }
    }
});
