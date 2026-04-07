import { test, expect } from '@playwright/test';
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const testRoot = path.resolve(__dirname, '..');

const MIME_TYPES = {
    '.html': 'text/html',
    '.js': 'application/javascript',
    '.wasm': 'application/wasm',
    '.data': 'application/octet-stream'
};

//
// Route handler that serves files from the local test build.
//
async function routeHandler(route)
{
    const url = new URL(route.request().url());

    if (url.pathname === '/')
    {
        return route.fulfill({
            contentType: 'text/html',
            body: '<!DOCTYPE html><html><body></body></html>'
        });
    }

    //
    // The Emscripten file packager may fetch .data relative to the page or
    // relative to the module. Always resolve it to _build/ to handle both cases.
    //
    let filePath;
    if (path.basename(url.pathname) === 'JsMaterialXGenShader.data')
    {
        filePath = path.join(testRoot, '_build', 'JsMaterialXGenShader.data');
    }
    else
    {
        filePath = path.join(testRoot, url.pathname);
    }

    filePath = path.resolve(filePath);
    if (!filePath.startsWith(testRoot + path.sep))
    {
        return route.fulfill({ status: 403 });
    }

    if (!fs.existsSync(filePath) || fs.statSync(filePath).isDirectory())
    {
        return route.fulfill({ status: 404 });
    }

    const ext = path.extname(filePath);
    return route.fulfill({
        body: fs.readFileSync(filePath),
        contentType: MIME_TYPES[ext] || 'application/octet-stream'
    });
}

test.describe('Generate Shaders', () =>
{
    test('Compile Shaders', async ({ page }) =>
    {
        page.on('console', (msg) =>
        {
            if (msg.type() === 'error')
            {
                console.error(msg.text());
            }
            else
            {
                console.log(msg.text());
            }
        });

        await page.route('**/*', routeHandler);
        await page.goto('http://materialx-test/');

        const { error, generators } = await page.evaluate(async () =>
        {
            const { default: MaterialX } = await import('/_build/JsMaterialXGenShader.js');
            const mx = await MaterialX();

            const doc = mx.createDocument();
            const ssName = 'SR_default';
            const ssNode = doc.addChildOfCategory('standard_surface', ssName);
            ssNode.setType('surfaceshader');
            const smNode = doc.addChildOfCategory('surfacematerial', 'Default');
            smNode.setType('material');
            const shaderInput = smNode.addInput('surfaceshader');
            shaderInput.setType('surfaceshader');
            shaderInput.setNodeName(ssName);

            const valid = doc.validate();
            shaderInput.delete();
            smNode.delete();
            ssNode.delete();
            if (!valid) return { error: 'Document validation failed', generators: [] };

            const canvas = document.createElement('canvas');
            const gl = canvas.getContext('webgl2');

            const generatorNames = [
                'EsslShaderGenerator', 'GlslShaderGenerator', 'MslShaderGenerator',
                'OslShaderGenerator', 'VkShaderGenerator', 'WgslShaderGenerator',
                'MdlShaderGenerator', 'SlangShaderGenerator'
            ];

            const elem = mx.findRenderableElement(doc);
            const generators = [];

            for (const name of generatorNames)
            {
                if (typeof mx[name] === 'undefined') continue;
                const gen = mx[name].create();
                const target = gen.getTarget();
                console.log('Generating shader for ' + target + '...');

                const genContext = new mx.GenContext(gen);
                const stdlib = mx.loadStandardLibraries(genContext);
                doc.importLibrary(stdlib);

                try
                {
                    const mxShader = gen.generate(elem.getNamePath(), elem, genContext);
                    const fShader = mxShader.getSourceCode('pixel');
                    const errors = [];

                    if (target === 'essl')
                    {
                        const vShader = mxShader.getSourceCode('vertex');

                        const glVS = gl.createShader(gl.VERTEX_SHADER);
                        gl.shaderSource(glVS, vShader);
                        gl.compileShader(glVS);
                        if (!gl.getShaderParameter(glVS, gl.COMPILE_STATUS))
                        {
                            errors.push('Vertex shader: ' + gl.getShaderInfoLog(glVS));
                        }

                        const glFS = gl.createShader(gl.FRAGMENT_SHADER);
                        gl.shaderSource(glFS, fShader);
                        gl.compileShader(glFS);
                        if (!gl.getShaderParameter(glFS, gl.COMPILE_STATUS))
                        {
                            errors.push('Fragment shader: ' + gl.getShaderInfoLog(glFS));
                        }

                        gl.deleteShader(glVS);
                        gl.deleteShader(glFS);
                    }

                    generators.push({ target, ok: errors.length === 0, errors });
                    mxShader.delete();
                }
                catch (errPtr)
                {
                    const msg = typeof mx.getExceptionMessage === 'function'
                        ? mx.getExceptionMessage(errPtr) : String(errPtr);
                    generators.push({ target, ok: false, errors: [msg] });
                }

                stdlib.delete();
                genContext.delete();
                gen.delete();
            }

            elem.delete();
            doc.delete();
            return { generators };
        });

        expect(error).toBeUndefined();
        expect(generators.length).toBeGreaterThan(0);
        for (const { target, ok, errors } of generators)
        {
            expect(ok, `${target} shader generation failed: ${errors.join('; ')}`).toBe(true);
        }
    });
});
