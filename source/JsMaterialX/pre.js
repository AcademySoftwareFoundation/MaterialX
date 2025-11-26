//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
var postRegistrations = [];

function onModuleReady(callback) {
    if (Module.calledRun) {
        callback();
    } else {
        postRegistrations.push(callback);
    }
}

// Install once, before runtime init, as a pre-js script.
Module.onRuntimeInitialized = function() {
    try {
        if (typeof Module.getNamePathSeparator === 'function') {
            Module.NAME_PATH_SEPARATOR = Module.getNamePathSeparator();
        }
        if (typeof Module.getPathListSeparator === 'function') {
            Module.PATH_LIST_SEPARATOR = Module.getPathListSeparator();
        }
        if (typeof Module.getSearchPathEnvVar === 'function') {
            Module.MATERIALX_SEARCH_PATH_ENV_VAR = Module.getSearchPathEnvVar();
        }
        if (typeof Module.getEmptyString === 'function') {
            Module.EMPTY_STRING = Module.getEmptyString();
        }
        if (typeof Module.getMtlxExtension === 'function') {
            Module.MTLX_EXTENSION = Module.getMtlxExtension();
        }
        if (typeof Module.getGeomPathSeparator === 'function') {
            Module.GEOM_PATH_SEPARATOR = Module.getGeomPathSeparator();
        }
        if (typeof Module.getUniversalGeomName === 'function') {
            Module.UNIVERSAL_GEOM_NAME = Module.getUniversalGeomName();
        }
        if (typeof Module.getUdimToken === 'function') {
            Module.UDIM_TOKEN = Module.getUdimToken();
        }
        if (typeof Module.getUvTileToken === 'function') {
            Module.UV_TILE_TOKEN = Module.getUvTileToken();
        }
        if (typeof Module.getUdimSetProperty === 'function') {
            Module.UDIM_SET_PROPERTY = Module.getUdimSetProperty();
        }
        // Type strings and node strings
        if (typeof Module.getSurfaceShaderTypeString === 'function') Module.SURFACE_SHADER_TYPE_STRING = Module.getSurfaceShaderTypeString();
        if (typeof Module.getDisplacementShaderTypeString === 'function') Module.DISPLACEMENT_SHADER_TYPE_STRING = Module.getDisplacementShaderTypeString();
        if (typeof Module.getVolumeShaderTypeString === 'function') Module.VOLUME_SHADER_TYPE_STRING = Module.getVolumeShaderTypeString();
        if (typeof Module.getLightShaderTypeString === 'function') Module.LIGHT_SHADER_TYPE_STRING = Module.getLightShaderTypeString();
        if (typeof Module.getMaterialTypeString === 'function') Module.MATERIAL_TYPE_STRING = Module.getMaterialTypeString();
        if (typeof Module.getSurfaceMaterialNodeString === 'function') Module.SURFACE_MATERIAL_NODE_STRING = Module.getSurfaceMaterialNodeString();
        if (typeof Module.getVolumeMaterialNodeString === 'function') Module.VOLUME_MATERIAL_NODE_STRING = Module.getVolumeMaterialNodeString();
        if (typeof Module.getMultiOutputTypeString === 'function') Module.MULTI_OUTPUT_TYPE_STRING = Module.getMultiOutputTypeString();
        if (typeof Module.getNoneTypeString === 'function') Module.NONE_TYPE_STRING = Module.getNoneTypeString();
        // Generic strings and separators
        if (typeof Module.getDefaultTypeString === 'function') Module.DEFAULT_TYPE_STRING = Module.getDefaultTypeString();
        if (typeof Module.getFilenameTypeString === 'function') Module.FILENAME_TYPE_STRING = Module.getFilenameTypeString();
        if (typeof Module.getGeomNameTypeString === 'function') Module.GEOMNAME_TYPE_STRING = Module.getGeomNameTypeString();
        if (typeof Module.getStringTypeString === 'function') Module.STRING_TYPE_STRING = Module.getStringTypeString();
        if (typeof Module.getValueStringTrue === 'function') Module.VALUE_STRING_TRUE = Module.getValueStringTrue();
        if (typeof Module.getValueStringFalse === 'function') Module.VALUE_STRING_FALSE = Module.getValueStringFalse();
        if (typeof Module.getNamePrefixSeparator === 'function') Module.NAME_PREFIX_SEPARATOR = Module.getNamePrefixSeparator();
        if (typeof Module.getArrayValidSeparators === 'function') Module.ARRAY_VALID_SEPARATORS = Module.getArrayValidSeparators();
        if (typeof Module.getArrayPreferredSeparator === 'function') Module.ARRAY_PREFERRED_SEPARATOR = Module.getArrayPreferredSeparator();
        if (typeof Module.getNullEdge === 'function') Module.NULL_EDGE = Module.getNullEdge();
    } catch (e) {
        if (typeof console !== 'undefined' && console.warn) console.warn('Failed to fetch NAME_PATH_SEPARATOR:', e);
    }
    for (var i = 0; i < postRegistrations.length; i++) {
        try { postRegistrations[i](); } catch (e) { if (typeof console !== 'undefined' && console.error) console.error(e); }
    }
    postRegistrations.length = 0;
};
