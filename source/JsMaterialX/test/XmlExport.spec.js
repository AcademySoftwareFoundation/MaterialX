import * as path from 'path';
import Module from './_build/JsMaterialX.js';
import { expect } from 'chai';

describe('XmlExport', () => {
  let mx;
  before(async () => {
      mx = await Module();
  });

  it('should convert FilePath to string', () => {
    const exportOptions = new mx.XmlExportOptions();
    expect(typeof exportOptions.resolvedTexturePath).to.equal("string");
  });
});

