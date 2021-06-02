import * as path from 'path';
import Module from './_build/JsMaterialX.js';
import { expect } from 'chai';

describe('XmlIo', () => {
  let mx;
  before(async () => {
      mx = await Module();
  });

  it('should convert FilePath to string', () => {
    const exportOptions = new mx.XmlExportOptions();
    expect(typeof exportOptions.userTexturePath).to.equal("string");
  });

  it('should prepend include tag', () => {
    const doc = mx.createDocument();
    const includePath = "SomePath";
    const writeOptions = new mx.XmlWriteOptions();
    mx.prependXInclude(doc, includePath);
    const xmlString = mx.writeToXmlString(doc, writeOptions);
    expect(xmlString).to.include(includePath);
  });
});

