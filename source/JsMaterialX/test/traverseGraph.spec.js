import { expect } from 'chai';
import { traverse, initMaterialX } from './testHelpers';

describe('Traverse Graph', () => {
    let mx, doc, image2, constant, multiply, contrast, mix, output;
    before(async () => {
        mx = await initMaterialX();
        // Create a document.
        doc = mx.createDocument();
        // Create a node graph with the following structure:
        //
        // [image1] [constant]     [image2]
        //        \ /                 |
        //    [multiply]          [contrast]         [noise3d]
        //             \____________  |  ____________/
        //                          [mix]
        //                            |
        //                         [output]
        //
        const nodeGraph = doc.addNodeGraph();
        const image1 = nodeGraph.addNode('image');
        image2 = nodeGraph.addNode('image');
        constant = nodeGraph.addNode('constant');
        multiply = nodeGraph.addNode('multiply');
        contrast = nodeGraph.addNode('contrast');
        const noise3d = nodeGraph.addNode('noise3d');
        mix = nodeGraph.addNode('mix');
        output = nodeGraph.addOutput();
        multiply.setConnectedNode('in1', image1);
        multiply.setConnectedNode('in2', constant);
        contrast.setConnectedNode('in', image2);
        mix.setConnectedNode('fg', multiply);
        mix.setConnectedNode('bg', contrast);
        mix.setConnectedNode('mask', noise3d);
        output.setConnectedNode(mix);
    });

    it('Validate the document', () => {
        expect(doc.validate()).to.be.true;
    });

    it('Traverse the document tree (implicit iterator)', () => {
        const elements = doc.traverseTree();
        let nodeCount = 0;
        traverse(elements, (elem) => {
            // Display the filename of each image node.
            if (elem instanceof mx.Node) {
                nodeCount++;
            }
        });
        expect(nodeCount).to.equal(7);
    });

    it('Traverse the document tree (explicit iterator)', () => {
        const treeIter = doc.traverseTree();
        let nodeCount = 0;
        let maxElementDepth = 0;
        traverse(treeIter, (elem) => {
            // Display the filename of each image node.
            if (elem instanceof mx.Node) {
                nodeCount++;
            }
            maxElementDepth = Math.max(maxElementDepth, treeIter.getElementDepth());
        });

        expect(nodeCount).to.equal(7);
        expect(maxElementDepth).to.equal(3);
    });

    it('Traverse the document tree (prune subtree)', () => {
        const treeIter = doc.traverseTree();
        let nodeCount = 0;
        traverse(treeIter, (elem) => {
            // Display the filename of each image node.
            if (elem instanceof mx.Node) {
                nodeCount++;
            }
            if (elem instanceof mx.NodeGraph) {
                treeIter.setPruneSubtree(true);
            }
        });

        expect(nodeCount).to.equal(0);
    });

    it('Traverse upstream from the graph output (implicit iterator)', () => {
        let nodeCount = 0;
        traverse(output.traverseGraph(), (edge) => {
            const upstreamElem = edge.getUpstreamElement();
            const connectingElem = edge.getConnectingElement();
            const downstreamElem = edge.getDownstreamElement();
            if (upstreamElem instanceof mx.Node) {
                nodeCount++;
                if (downstreamElem instanceof mx.Node) {
                    expect(connectingElem instanceof mx.Input).to.be.true;
                }
            }
        });
        expect(nodeCount).to.equal(7);
    });

    it('Traverse upstream from the graph output (explicit iterator)', () => {
        let nodeCount = 0;
        let maxElementDepth = 0;
        let maxNodeDepth = 0;
        const graphIter = output.traverseGraph();
        traverse(graphIter, (edge) => {
            const upstreamElem = edge.getUpstreamElement();
            if (upstreamElem instanceof mx.Node) {
                nodeCount++;
            }
            maxElementDepth = Math.max(maxElementDepth, graphIter.getElementDepth());
            maxNodeDepth = Math.max(maxNodeDepth, graphIter.getNodeDepth());
        });

        expect(nodeCount).to.equal(7);
        expect(maxElementDepth).to.equal(3);
        expect(maxNodeDepth).to.equal(3);
    });

    it('Traverse upstream from the graph output (prune subgraph)', () => {
        let nodeCount = 0;
        const graphIter = output.traverseGraph();
        traverse(graphIter, (edge) => {
            const upstreamElem = edge.getUpstreamElement();
            if (upstreamElem instanceof mx.Node) {
                nodeCount++;
            }
            if (upstreamElem.getCategory() === 'multiply') {
                graphIter.setPruneSubgraph(true);
            }
        });
        expect(nodeCount).to.equal(5);
    });

    function isCycle(cb) {
        try {
            return cb();
        } catch (exceptionPtr) {
            const message = mx.getExceptionMessage(exceptionPtr);
            if (message.indexOf('Encountered cycle') !== -1) {
                return true;
            }
            return false;
        }
    }
    it('Create and detect a cycle', () => {
        multiply.setConnectedNode('in2', mix);

        expect(
            isCycle(() => {
                return output.hasUpstreamCycle();
            })
        ).to.be.true;
        expect(
            isCycle(() => {
                return doc.validate();
            })
        ).to.be.true;
        multiply.setConnectedNode('in2', constant);
        expect(
            isCycle(() => {
                return output.hasUpstreamCycle();
            })
        ).to.be.false;
        expect(
            isCycle(() => {
                return doc.validate();
            })
        ).to.be.true;
    });

    it('Create and detect a loop', () => {
        contrast.setConnectedNode('in', contrast);
        expect(
            isCycle(() => {
                return output.hasUpstreamCycle();
            })
        ).to.be.true;
        expect(
            isCycle(() => {
                return doc.validate();
            })
        ).to.be.true;
        contrast.setConnectedNode('in', image2);
        expect(
            isCycle(() => {
                return output.hasUpstreamCycle();
            })
        ).to.be.false;
        expect(
            isCycle(() => {
                return doc.validate();
            })
        ).to.be.true;
    });
});
