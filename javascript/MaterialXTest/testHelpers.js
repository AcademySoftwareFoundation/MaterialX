import fs from 'fs';
import path from 'path';

export function getMtlxStrings(fileNames, subPath)
{
    return fileNames.map(name => fs.readFileSync(path.resolve(subPath, name), 'utf8'));
}
