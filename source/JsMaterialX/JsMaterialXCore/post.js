Module.onRuntimeInitialized = function() {
  Module.ValueElement.prototype.getValue = function () {
    const value = Module.ValueElement.prototype._getValue.apply(this);
    if (value) {
      return value.getData();
    }
    return value;
  }
};

