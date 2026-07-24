import { test, expect } from '@playwright/test';
import Module from './_build/JsMaterialXCore.js';

test.describe('Environ', () =>
{
    let mx;
    test.beforeAll(async () =>
    {
        mx = await Module();
    });

    test('Environment variables', () =>
    {
        expect(mx.getEnviron(mx.MATERIALX_SEARCH_PATH_ENV_VAR)).toBe('');
        mx.setEnviron(mx.MATERIALX_SEARCH_PATH_ENV_VAR, 'test');
        expect(mx.getEnviron(mx.MATERIALX_SEARCH_PATH_ENV_VAR)).toBe('test');
        mx.removeEnviron(mx.MATERIALX_SEARCH_PATH_ENV_VAR);
        expect(mx.getEnviron(mx.MATERIALX_SEARCH_PATH_ENV_VAR)).toBe('');
    });
});
