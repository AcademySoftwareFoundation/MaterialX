//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Wrapping code in an anonymous function to prevent clashes with the main module and other pre / post JS.
(function () {
    onModuleReady(function () {
        // Generate a Mermaid graph string for a generated Shader using JS-side APIs.
        function _mxEscapeMermaidLabel(value) {
            return String(value)
                .replace(/\\/g, "\\\\")
                .replace(/"/g, '\\"');
        }

        function _mxGetTextColor(fillColor) {
            var color = fillColor.replace("#", "");
            if (color.length !== 6) {
                return "#000000";
            }

            var red = parseInt(color.substring(0, 2), 16);
            var green = parseInt(color.substring(2, 4), 16);
            var blue = parseInt(color.substring(4, 6), 16);
            var brightness = (0.299 * red) + (0.587 * green) + (0.114 * blue);
            return brightness >= 186 ? "#000000" : "#ffffff";
        }

        function _mxGetClassificationData() {
            var n = Module.ShaderNode;
            var colorByClass = [
                [n.MATERIAL, "#7ccba2"],
                [n.SHADER, "#0e7865"],
                [n.SURFACE, "#4f9fc5"],
                [n.VOLUME, "#3d84b8"],
                [n.LIGHT, "#2f6fa3"],
                [n.UNLIT, "#94d2bd"],
                [n.CLOSURE, "#f4a261"],
                [n.BSDF, "#e76f51"],
                [n.BSDF_R, "#f4c06a"],
                [n.BSDF_T, "#f2a65a"],
                [n.EDF, "#f6bd60"],
                [n.VDF, "#f28482"],
                [n.LAYER, "#f7a072"],
                [n.MIX, "#f5b971"],
                [n.TEXTURE, "#c27ff8"],
                [n.FILETEXTURE, "#be5ae6"],
                [n.SAMPLE2D, "#7a558a"],
                [n.SAMPLE3D, "#7a4485"],
                [n.CONSTANT, "#888888"],
                [n.CONDITIONAL, "#e8400d"],
                [n.GEOMETRIC, "#4B0336"],
                [n.DOT, "#e9f5db"]
            ];

            var labelByClass = [
                [n.MATERIAL, "MATERIAL"],
                [n.SHADER, "SHADER"],
                [n.SURFACE, "SURFACE"],
                [n.VOLUME, "VOLUME"],
                [n.LIGHT, "LIGHT"],
                [n.UNLIT, "UNLIT"],
                [n.CLOSURE, "CLOSURE"],
                [n.BSDF, "BSDF"],
                [n.BSDF_R, "BSDF_R"],
                [n.BSDF_T, "BSDF_T"],
                [n.EDF, "EDF"],
                [n.VDF, "VDF"],
                [n.LAYER, "LAYER"],
                [n.MIX, "MIX"],
                [n.TEXTURE, "TEXTURE"],
                [n.FILETEXTURE, "FILETEXTURE"],
                [n.SAMPLE2D, "SAMPLE2D"],
                [n.SAMPLE3D, "SAMPLE3D"],
                [n.CONSTANT, "CONSTANT"],
                [n.CONDITIONAL, "CONDITIONAL"],
                [n.GEOMETRIC, "GEOMETRIC"],
                [n.DOT, "DOT"]
            ];

            return {
                colorByClass: colorByClass,
                labelByClass: labelByClass,
                textureClass: n.TEXTURE
            };
        }

        function _mxGetClassificationFill(classificationBits, classData) {
            var textureColor = "#c27ff8";
            var nonTextureBits = classificationBits & ~classData.textureClass;
            for (var i = 0; i < classData.colorByClass.length; ++i) {
                var entry = classData.colorByClass[i];
                if (nonTextureBits & entry[0]) {
                    return entry[1];
                }
            }
            return textureColor;
        }

        function _mxGetClassificationLabel(classificationBits, classData) {
            var labels = [];
            for (var i = 0; i < classData.labelByClass.length; ++i) {
                var entry = classData.labelByClass[i];
                if (classificationBits & entry[0]) {
                    labels.push(entry[1]);
                }
            }
            if (labels.length) {
                return labels.join("|");
            }
            return "UNKNOWN";
        }

        Module.createMermaidGraph = function(shader, showInputValues = false) {
            if (arguments.length < 1 || arguments.length > 2) {
                throw new Error("Function createMermaidGraph called with an invalid number of arguments (" +
                    arguments.length + ") - expects 1 to 2!");
            }
            if (!shader || typeof shader.getNodes !== "function") {
                throw new Error("createMermaidGraph expects a Shader instance.");
            }
            if (typeof Module.ShaderNode === "undefined") {
                throw new Error("createMermaidGraph requires JsMaterialXGenShader.");
            }

            var classData = _mxGetClassificationData();
            var lines = ["graph TB"];
            var strokeColor = "#222222";
            var nodes = shader.getNodes();

            for (var ni = 0; ni < nodes.length; ++ni) {
                var node = nodes[ni];
                var nodeId = node.getUniqueId();
                var classification = node.getClassification();
                var fill = _mxGetClassificationFill(classification, classData);
                var textColor = _mxGetTextColor(fill);
                var classLabel = _mxGetClassificationLabel(classification, classData);

                lines.push("    " + nodeId + "[\"" + _mxEscapeMermaidLabel(nodeId + " (" + classLabel + ")") + "\"]");
                lines.push("    style " + nodeId + " fill:" + fill + ",stroke:" + strokeColor + ",stroke-width:1px,color:" + textColor);
            }

            for (var n2i = 0; n2i < nodes.length; ++n2i) {
                var currentNode = nodes[n2i];
                var currentNodeId = currentNode.getUniqueId();
                var outputs = currentNode.getOutputs();

                for (var oi = 0; oi < outputs.length; ++oi) {
                    var output = outputs[oi];
                    var connections = output.getConnections();
                    for (var ci = 0; ci < connections.length; ++ci) {
                        var inputPort = connections[ci];
                        if (!inputPort) {
                            continue;
                        }

                        var inputNode = inputPort.getNode();
                        if (!inputNode) {
                            continue;
                        }

                        var upstreamId = inputNode.getUniqueId();
                        if (upstreamId === currentNodeId) {
                            continue;
                        }

                        var connectorLabel = _mxEscapeMermaidLabel(output.getName() + " --> " + inputPort.getName());
                        lines.push("    " + currentNodeId + " --\"" + connectorLabel + "\"--> " + upstreamId);
                    }
                }

                if (showInputValues) {
                    var inputs = currentNode.getInputs();
                    for (var ii = 0; ii < inputs.length; ++ii) {
                        var input = inputs[ii];
                        if (!input) {
                            continue;
                        }

                        var valueString = input.getValueString();
                        if (!valueString) {
                            continue;
                        }

                        var valueNodeId = currentNodeId + "/" + input.getName();
                        var valueLabel = _mxEscapeMermaidLabel(input.getName() + " = " + valueString);
                        lines.push("    " + valueNodeId + "[\"" + valueLabel + "\"] --> " + currentNodeId);
                    }
                }
            }

            return lines.join("\n");
        };
    });
})();
