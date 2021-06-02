import { expect } from 'chai';;
import Module from './_build/JsMaterialX.js';

describe('Basics', () => {
    let mx;
    before(async () => {
        mx = await Module();
    });

    it('DataTypes', () => {
        const testValues = {
            integer: '1',
            boolean: 'true',
            float: '1.1',
            color3: '0.1, 0.2, 0.3',
            color4: '0.1, 0.2, 0.3, 0.4',
            vector2: '1.1, 2.1',
            vector3: '1.1, 2.1, 3.1',
            vector4: '1.1, 2.1, 3.1, 4.1',
            matrix33: '0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1',
            matrix44: '1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1',
            string: 'value',
            integerarray: '1, 2, 3',
            booleanarray: 'false, true, false',
            floatarray: '1.1, 2.1, 3.1',
            stringarray: "'one', 'two', 'three'",
        };

        for (let type in testValues) {
            const value = testValues[String(type)];
            const newValue = mx.Value.createValueFromStrings(value, type);
            const typeString = newValue.getTypeString();
            const valueString = newValue.getValueString();
            expect(typeString).to.equal(type);
            expect(valueString).to.equal(value);
        }
    });

    it('Vectors', () => {
        const v1 = new mx.Vector3(1, 2, 3);
        let v2 = new mx.Vector3(2, 4, 6);

        // Indexing operators
        expect(v1.getItem(2)).to.equal(3);

        v1.setItem(2, 4);
        expect(v1.getItem(2)).to.equal(4);
        v1.setItem(2, 3);
        // Component-wise operators
        let res = v2.add(v1);
        expect(res.equals(new mx.Vector3(3, 6, 9))).to.be.true;

        res = v2.sub(v1);
        expect(res.equals(new mx.Vector3(1, 2, 3))).to.be.true;

        res = v2.multiply(v1);
        expect(res.equals(new mx.Vector3(2, 8, 18))).to.be.true;

        res = v2.divide(v1);
        expect(res.equals(new mx.Vector3(2, 2, 2))).to.be.true;

        v2 = v2.add(v1);
        expect(v2.equals(new mx.Vector3(3, 6, 9))).to.be.true;

        v2 = v2.sub(v1);
        expect(v2.equals(new mx.Vector3(2, 4, 6))).to.be.true;

        v2 = v2.multiply(v1);
        expect(v2.equals(new mx.Vector3(2, 8, 18))).to.be.true;

        v2 = v2.divide(v1);
        expect(v2.equals(new mx.Vector3(2, 4, 6))).to.be.true;

        expect(v1.multiply(new mx.Vector3(2, 2, 2)).equals(v2)).to.be.true;
        expect(v2.divide(new mx.Vector3(2, 2, 2)).equals(v1)).to.be.true;

        // Geometric methods
        let v3 = new mx.Vector4(4, 4, 4, 4);
        expect(v3.getMagnitude()).to.equal(8);
        expect(v3.getNormalized().getMagnitude()).to.equal(1);
        expect(v1.dot(v2)).to.equal(28);
        expect(v1.cross(v2).equals(new mx.Vector3())).to.be.true;

        // Vector copy
        const v4 = v2.copy();
        expect(v4.equals(v2)).to.be.true;
        v4.setItem(0, v4.getItem(0) + 1);
        expect(v4.notEquals(v2)).to.be.true;
    });

    function multiplyMatrix(matrix, val) {
        const clonedMatrix = matrix.copy();
        for (let i = 0; i < clonedMatrix.numRows(); ++i) {
            for (let k = 0; k < clonedMatrix.numColumns(); ++k) {
                const v = clonedMatrix.getItem(i, k);
                clonedMatrix.setItem(i, k, v * val);
            }
        }
        return clonedMatrix;
    }

    function divideMatrix(matrix, val) {
        const clonedMatrix = matrix.copy();
        for (let i = 0; i < clonedMatrix.numRows(); ++i) {
            for (let k = 0; k < clonedMatrix.numColumns(); ++k) {
                const v = clonedMatrix.getItem(i, k);
                clonedMatrix.setItem(i, k, v / val);
            }
        }
        return clonedMatrix;
    }

    it('Matrices', () => {
        // Translation and scale
        const trans = mx.Matrix44.createTranslation(new mx.Vector3(1, 2, 3));
        const scale = mx.Matrix44.createScale(new mx.Vector3(2, 2, 2));
        expect(trans.equals(new mx.Matrix44(1, 0, 0, 0,
                                            0, 1, 0, 0,
                                            0, 0, 1, 0,
                                            1, 2, 3, 1)));
        expect(scale.equals(new mx.Matrix44(2, 0, 0, 0,
                                            0, 2, 0, 0,
                                            0, 0, 2, 0,
                                            0, 0, 0, 1)));

        // Indexing operators
        expect(trans.getItem(3, 2)).to.equal(3);
        trans.setItem(3, 2, 4);
        expect(trans.getItem(3, 2)).to.equal(4);
        trans.setItem(3, 2, 3);

        // Matrix methods
        expect(trans.getTranspose().equals(
            new mx.Matrix44(1, 0, 0, 1,
                            0, 1, 0, 2,
                            0, 0, 1, 3,
                            0, 0, 0, 1)
        )).to.be.true;
        expect(scale.getTranspose().equals(scale)).to.be.true;
        expect(trans.getDeterminant()).to.equal(1);
        expect(scale.getDeterminant()).to.equal(8);
        expect(trans.getInverse().equals(
            mx.Matrix44.createTranslation(new mx.Vector3(-1, -2, -3)))).to.be.true;

        // Matrix product
        const prod1 = trans.multiply(scale);
        const prod2 = scale.multiply(trans);
        const prod3 = multiplyMatrix(trans, 2);
        let prod4 = trans;
        prod4 = prod4.multiply(scale);
        expect(prod1.equals(new mx.Matrix44(2, 0, 0, 0,
                                            0, 2, 0, 0,
                                            0, 0, 2, 0,
                                            2, 4, 6, 1)));
        expect(prod2.equals(new mx.Matrix44(2, 0, 0, 0,
                                            0, 2, 0, 0,
                                            0, 0, 2, 0,
                                            1, 2, 3, 1)));
        expect(prod3.equals(new mx.Matrix44(2, 0, 0, 0,
                                            0, 2, 0, 0,
                                            0, 0, 2, 0,
                                            2, 4, 6, 2)));
        expect(prod4.equals(prod1));

        // Matrix division
        const quot1 = prod1.divide(scale);
        const quot2 = prod2.divide(trans);
        const quot3 = divideMatrix(prod3, 2);
        let quot4 = quot1;
        quot4 = quot4.divide(trans);
        expect(quot1.equals(trans)).to.be.true;
        expect(quot2.equals(scale)).to.be.true;
        expect(quot3.equals(trans)).to.be.true;

        // 2D rotation
        const _epsilon = 1e-4;
        const rot1 = mx.Matrix33.createRotation(Math.PI / 2);
        const rot2 = mx.Matrix33.createRotation(Math.PI);
        expect(rot1.multiply(rot1).isEquivalent(rot2, _epsilon));
        expect(rot2.isEquivalent(mx.Matrix33.createScale(new mx.Vector2(-1, -1)), _epsilon));
        expect(rot2.multiply(rot2).isEquivalent(mx.Matrix33.IDENTITY, _epsilon));

        // 3D rotation
        const rotX = mx.Matrix44.createRotationX(Math.PI);
        const rotY = mx.Matrix44.createRotationY(Math.PI);
        const rotZ = mx.Matrix44.createRotationZ(Math.PI);
        expect(rotX.multiply(rotY).isEquivalent(mx.Matrix44.createScale(new mx.Vector3(-1, -1, 1)), _epsilon));
        expect(rotX.multiply(rotZ).isEquivalent(mx.Matrix44.createScale(new mx.Vector3(-1, 1, -1)), _epsilon));
        expect(rotY.multiply(rotZ).isEquivalent(mx.Matrix44.createScale(new mx.Vector3(1, -1, -1)), _epsilon));

        // Matrix copy
        const trans2 = trans.copy();
        expect(trans2.equals(trans)).to.be.true;
        trans2.setItem(0, 0, trans2.getItem(0, 0) + 1);
        expect(trans2.notEquals(trans)).to.be.true;
    });

    it('Array conversion', () => {
        // Functions that return vectors in C++ should return an array in JS
        const doc = mx.createDocument();
        const nodeGraph = doc.addNodeGraph();
        doc.addNodeGraph();
        const nodeGraphs = doc.getNodeGraphs();
        expect(nodeGraphs).to.be.an.instanceof(Array);
        expect(nodeGraphs.length).to.equal(2);

        // Elements fetched through the vector -> array conversion should be editable and changes should be reflected
        // in the original objects.
        // Note: We cannot simply compare these objects for equality, since they're separately constructed pointers
        // to the same object.
        const backdrop = nodeGraph.addBackdrop();
        const backDrops = nodeGraphs[0].getBackdrops();
        expect(backDrops.length).to.equal(1);
        nodeGraphs[0].addBackdrop();
        expect(nodeGraph.getBackdrops().length).to.equal(2);

        // Functions that expect vectors as parameters in C++ should accept arrays in JS
        // Built-in types
        const pathSegments = ['path', 'to', 'something'];
        const namePath = mx.createNamePath(pathSegments);
        expect(namePath).to.equal(pathSegments.join(mx.NAME_PATH_SEPARATOR));

        // Complex (smart pointer) types
        const node1 = nodeGraph.addNode('node');
        const node2 = nodeGraph.addNode('node');
        const node3 = nodeGraph.addNode('node', 'anotherNode');
        backdrop.setContainsElements([node1, node2, node3]);
        const nodes = backdrop.getContainsElements();
        expect(nodes.length).to.equal(3);
        expect(nodes[0].getName()).to.equal('node1'); // Name auto-constructed from category
        expect(nodes[1].getName()).to.equal('node2'); // Name auto-constructed from category
        expect(nodes[2].getName()).to.equal('anotherNode'); // Name set explicitly at creation time
    });

    it('C++ exception handling', () => {
      // Exceptions that are thrown and caught in C++ shouldn't bubble up to JS
      const doc = mx.createDocument();
      const nodeGraph1 = doc.addNodeGraph();
      const nodeGraph2 = doc.addNodeGraph();
      nodeGraph1.setInheritsFrom(nodeGraph2);
      nodeGraph2.setInheritsFrom(nodeGraph1);
      expect(nodeGraph1.hasInheritanceCycle()).to.be.true;
    });
});
