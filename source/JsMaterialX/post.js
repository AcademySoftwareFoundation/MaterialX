//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//
var postRegistrations = [];

function onModuleReady(callback) {
    postRegistrations.push(callback);
}

// This callback should only be registered once in all post JS scripts.
// That's why we do it globally here, and let other scripts register their code via 'onModuleReady'.
Module.onRuntimeInitialized = function() {
    for (var callback of postRegistrations) {
        callback();
    }
};