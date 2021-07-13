//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved. See LICENSE.txt for license.
//

// Wrapping code in an anonymous function to prevent clashes with the main module and other pre / post JS.
(function () {
    onModuleReady(function() {
    Module.ValueElement.prototype.getValue = function () {
        const value = Module.ValueElement.prototype._getValue.apply(this);
        if (value) {
        return value.getData();
        }
        return value;
    }
    });
})();
