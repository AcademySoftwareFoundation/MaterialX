// jsTraversal
addWrapper(function(Module, api) {
    /** Setup the Edge class */
    api.Edge = wrapperFactory(Module.Edge);

    /** Setup the TreeIterator class */
    api.TreeIterator = wrapperFactory(Module.TreeIterator);

    var _next1 = Module.TreeIterator.prototype.next;
    api.TreeIterator.prototype.next = function() {
        try {
            return _next1.apply(this, arguments);
        } catch (err) {
            let message = err.message ? err.message : '';
            if (typeof err === 'number') {
                message = Module.getExceptionMessage(err);
            }
            if (message && message.indexOf('Could not get the next element.') !== -1) {
                return null;
            }
            throw err;
        }
    };

    /** Setup the GraphIterator class */
    api.GraphIterator = wrapperFactory(Module.GraphIterator);
    var _next2 = Module.GraphIterator.prototype.next;
    api.GraphIterator.prototype.next = function() {
        try {
            return _next2.apply(this, arguments);
        } catch (err) {
            let message = err.message ? err.message : '';
            if (typeof err === 'number') {
                message = Module.getExceptionMessage(err);
            }
            if (message && message.indexOf('Could not get the next element.') !== -1) {
                return null;
            }
            throw err;
        }
    };

    /** Setup the InheritanceIterator class */
    api.InheritanceIterator = wrapperFactory(Module.InheritanceIterator);

    var _next3 = Module.InheritanceIterator.prototype.next;
    api.InheritanceIterator.prototype.next = function() {
        try {
            return _next3.apply(this, arguments);
        } catch (err) {
            let message = err.message ? err.message : '';
            if (typeof err === 'number') {
                message = Module.getExceptionMessage(err);
            }
            if (message && message.indexOf('Could not get the next element.') !== -1) {
                return null;
            }
            throw err;
        }
    };
});
