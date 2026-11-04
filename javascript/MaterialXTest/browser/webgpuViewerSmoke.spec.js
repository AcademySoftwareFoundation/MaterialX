import { test, expect } from '@playwright/test';
import crypto from 'crypto';
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import {
    WEBGPU_SMOKE_CASES,
    WEBGPU_SMOKE_GEOM,
    webgpuViewerUrl,
} from './webgpuSmokeCases.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const testRoot = path.resolve(__dirname, '..');
const screenshotDir = path.join(testRoot, '_build', 'webgpu-smoke');
const wasmDir = path.resolve(testRoot, '..', 'build', 'bin');
const viewerDir = path.resolve(testRoot, '..', 'MaterialXView');
const distDir = path.join(viewerDir, 'dist');
const SETTLE_MS = 8_000;

// Content types for the files the viewer loads. WASM must be application/wasm for streaming
// compilation; anything else is served as binary.
const CONTENT_TYPES = {
    '.html': 'text/html',
    '.js': 'text/javascript',
    '.mjs': 'text/javascript',
    '.json': 'application/json',
    '.wasm': 'application/wasm',
    '.mtlx': 'application/xml',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.glb': 'model/gltf-binary',
};

// Requests to this origin are answered from dist/ by Playwright (page.route), so no server is
// started. A localhost origin keeps the page a secure context, which navigator.gpu requires.
const VIEWER_ORIGIN = 'http://127.0.0.1:8080';

async function serveDist(route)
{
    let file;
    try
    {
        file = path.resolve(distDir, '.' + decodeURIComponent(new URL(route.request().url()).pathname));
    }
    catch
    {
        return route.fulfill({ status: 400 });
    }
    if (!file.startsWith(distDir + path.sep))
    {
        return route.fulfill({ status: 403 });
    }
    let body;
    try
    {
        body = fs.readFileSync(file);
    }
    catch
    {
        return route.fulfill({ status: 404 });
    }
    return route.fulfill({
        body,
        contentType: CONTENT_TYPES[path.extname(file).toLowerCase()] || 'application/octet-stream',
    });
}

function fileHash(file)
{
    return crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
}

// A dist/ bundle that predates the WASM or viewer sources renders stale shaders.
function assertDistIsCurrent()
{
    const rebuild = 'Run `npm run build` in javascript/MaterialXView.';
    for (const file of ['JsMaterialXGenShader.wasm', 'JsMaterialXGenShader.data', 'JsMaterialXGenShader.js'])
    {
        const built = path.join(wasmDir, file);
        const shipped = path.join(distDir, file);
        expect(fs.existsSync(shipped), `${shipped} is missing. ${rebuild}`).toBe(true);
        expect(fileHash(shipped), `dist/${file} differs from build/bin/${file}. ${rebuild}`).toBe(fileHash(built));
    }
    // Viewer sources can only be compared by mtime, and webpack doesn't rewrite an unchanged
    // bundle, so a touched-but-unchanged source (e.g. after a branch switch) would look newer
    // forever. Warn instead of failing.
    const bundleTime = fs.statSync(path.join(distDir, 'main.webgpu.js')).mtimeMs;
    const sourceDir = path.join(viewerDir, 'source');
    const newer = fs.readdirSync(sourceDir)
        .filter((file) => fs.statSync(path.join(sourceDir, file)).mtimeMs > bundleTime);
    if (newer.length)
    {
        console.warn(`source/{${newer.join(', ')}} newer than dist/main.webgpu.js. ` +
            `If they changed: ${rebuild}`);
    }
}

async function assertWebGPUAvailable(page)
{
    const status = await page.evaluate(async () =>
    {
        if (!navigator.gpu) return 'navigator.gpu missing';
        try
        {
            const adapter = await navigator.gpu.requestAdapter();
            if (!adapter) return 'no adapter';
            await adapter.requestDevice();
            return 'ok';
        }
        catch (e)
        {
            return String(e);
        }
    });
    expect(status, 'Chromium must create a WebGPU device (use --enable-unsafe-webgpu)').toBe('ok');
}

async function waitForViewerCanvas(page)
{
    await page.waitForSelector('#webglcanvas', { state: 'visible', timeout: 30_000 });
    await expect(page.locator('#webglcanvas')).toHaveAttribute('width', /.+/);
    await expect(page.getByRole('button', { name: 'WebGPU' })).toBeVisible();
}

// Fraction of black and magenta (shader error) pixels in the center of the canvas.
async function sampleCenter(page)
{
    const box = await page.locator('#webglcanvas').boundingBox();
    const w = box.width * 0.15;
    const h = box.height * 0.15;
    const png = await page.screenshot({
        clip: { x: box.x + (box.width - w) / 2, y: box.y + (box.height - h) / 2, width: w, height: h },
    });
    return page.evaluate(async (b64) =>
    {
        const img = new Image();
        img.src = `data:image/png;base64,${b64}`;
        await img.decode();
        const canvas = document.createElement('canvas');
        canvas.width = img.width;
        canvas.height = img.height;
        const ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0);
        const d = ctx.getImageData(0, 0, canvas.width, canvas.height).data;
        let black = 0;
        let magenta = 0;
        for (let i = 0; i < d.length; i += 4)
        {
            const r = d[i], g = d[i + 1], b = d[i + 2];
            if (r < 8 && g < 8 && b < 8) black++;
            else if (r > 200 && g < 60 && b > 200) magenta++;
        }
        const n = d.length / 4;
        return { black: black / n, magenta: magenta / n };
    }, png.toString('base64'));
}

test.describe('WebGPU MaterialXView smoke', () =>
{
    // Opt-in is handled in playwright.config.js: without it this project has no tests.

    test.beforeAll(() =>
    {
        fs.mkdirSync(screenshotDir, { recursive: true });
        assertDistIsCurrent();
    });

    for (const smokeCase of WEBGPU_SMOKE_CASES)
    {
        test(`${smokeCase.label} via ?file=&geom=`, async ({ page }, testInfo) =>
        {
            const errors = [];
            page.on('console', (msg) =>
            {
                const text = msg.text();
                if (msg.type() === 'error' || text.includes('running under WebGL2 backend'))
                {
                    errors.push(`[console.${msg.type()}] ${text}`);
                }
            });
            page.on('pageerror', (err) => errors.push(`[pageerror] ${String(err)}`));

            await page.route(`${ VIEWER_ORIGIN }/**`, serveDist);
            const target = webgpuViewerUrl(VIEWER_ORIGIN, smokeCase.file, WEBGPU_SMOKE_GEOM);
            await page.goto(target, { waitUntil: 'domcontentloaded' });
            expect(page.url()).toContain('index-webgpu.html');
            expect(new URL(page.url()).searchParams.get('file')).toBe(smokeCase.file);
            expect(new URL(page.url()).searchParams.get('geom')).toBe(WEBGPU_SMOKE_GEOM);

            await assertWebGPUAvailable(page);
            await waitForViewerCanvas(page);
            await page.waitForTimeout(SETTLE_MS);

            const shotPath = path.join(screenshotDir, `${smokeCase.id}.png`);
            await page.screenshot({ path: shotPath, fullPage: false });
            await testInfo.attach(smokeCase.id, { path: shotPath, contentType: 'image/png' });

            expect(errors, 'console errors').toEqual([]);

            const center = await sampleCenter(page);
            expect(center.magenta, 'shader error color at the center of the shaderball').toBeLessThan(0.05);
            expect(center.black, 'black shaderball (material failed to render)').toBeLessThan(0.5);
        });
    }
});
