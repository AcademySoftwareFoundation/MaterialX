import * as THREE from 'three';

const debugFileHandling = true;
let loadingCallback;

export function setLoadingCallback(cb) {
    loadingCallback = cb;
}

export function dropHandler(ev) {
    if (debugFileHandling) console.log('File(s) dropped', ev.dataTransfer.items, ev.dataTransfer.files);

    // Prevent default behavior (Prevent file from being opened)
    ev.preventDefault();

    if (ev.dataTransfer.items)
    {
        const allEntries = [];

        let haveGetAsEntry = false;
        if (ev.dataTransfer.items.length > 0)
        haveGetAsEntry = ("getAsEntry" in ev.dataTransfer.items[0]) || ("webkitGetAsEntry" in ev.dataTransfer.items[0]);

        if (haveGetAsEntry) {
        for (var i = 0; i < ev.dataTransfer.items.length; i++)
        {
            let item = ev.dataTransfer.items[i];
            let entry = ("getAsEntry" in item) ? item.getAsEntry() : item.webkitGetAsEntry();
            allEntries.push(entry);
        }
        handleFilesystemEntries(allEntries);
        return;
        }

        for (var i = 0; i < ev.dataTransfer.items.length; i++)
        {
        let item = ev.dataTransfer.items[i];
        
        // API when there's no "getAsEntry" support
        console.log(item.kind, item, entry);
        if (item.kind === 'file')
        {
            var file = item.getAsFile();
            testAndLoadFile(file);
        }
        // could also be a directory
        else if (item.kind === 'directory')
        {
            var dirReader = item.createReader();
            dirReader.readEntries(function(entries) {
            for (var i = 0; i < entries.length; i++) {
                console.log(entries[i].name);
                var entry = entries[i];
                if (entry.isFile) {
                entry.file(function(file) {
                    testAndLoadFile(file);
                });
                }
            }
            });
        }
        }
    } else {
        for (var i = 0; i < ev.dataTransfer.files.length; i++) {
            let file = ev.dataTransfer.files[i];
            testAndLoadFile(file);
        }
    }
}

export function dragOverHandler(ev) {
    ev.preventDefault();
}

async function handleFilesystemEntries(entries) {
    const allFiles = [];
    const fileIgnoreList = [
      '.gitignore',
      'README.md',
      '.DS_Store',
    ]
    const dirIgnoreList = [
      '.git',
      'node_modules',
    ]

    for (let entry of entries) {
      if (debugFileHandling) console.log("file entry", entry)
      if (entry.isFile) {
        if (debugFileHandling) console.log("single file", entry);
        if (fileIgnoreList.includes(entry.name)) {
          continue;
        }
        allFiles.push(entry);
      }
      else if (entry.isDirectory) {
        if (dirIgnoreList.includes(entry.name)) {
          continue;
        }
        const files = await readDirectory(entry);
        if (debugFileHandling) console.log("all files", files);
        for (const file of files) {
          if (fileIgnoreList.includes(file.name)) {
            continue;
          }
          allFiles.push(file);
        }
      }
    }

    // sort so mtlx files come first
    allFiles.sort((a, b) => {
        if (a.name.endsWith('.mtlx') && !b.name.endsWith('.mtlx')) {
            return -1;
        }
        if (!a.name.endsWith('.mtlx') && b.name.endsWith('.mtlx')) {
            return 1;
        }
        return 0;
    });

    if (debugFileHandling) console.log("all files", allFiles);

    const imageLoader = new THREE.ImageLoader();

    // put all files in three' Cache
    for (const fileEntry of allFiles) {
        const allowedFileTypes = [
            'png', 'jpg', 'jpeg'
        ];

        const ext = fileEntry.fullPath.split('.').pop();
        if (!allowedFileTypes.includes(ext)) {
            console.log("skipping file", fileEntry.fullPath);
            continue;
        }


        const buffer = await new Promise((resolve, reject) => {
            fileEntry.file(function(file) {
                var reader = new FileReader();
                reader.onloadend = function(e) {
                    resolve(this.result);
                };
                reader.readAsArrayBuffer(file);
            }, (e) => {
                console.error("Error reading file ", e);
            });
        });

        const img = await imageLoader.loadAsync(URL.createObjectURL(new Blob([buffer])));
        console.log("caching file", fileEntry.fullPath, img);
        THREE.Cache.add(fileEntry.fullPath, img);
    }

    loadingCallback(allFiles[0]);
}

async function readDirectory(directory) {
    let entries = [];
    let getEntries = async (directory) => {
        let dirReader = directory.createReader();
        await new Promise((resolve, reject) => {
        dirReader.readEntries(
            async (results) => {
            if (results.length) {
                // entries = entries.concat(results);
                for (let entry of results) {
                if (entry.isDirectory) {
                    await getEntries(entry);
                }
                else {
                    entries.push(entry);
                }
                }
            }
            resolve();
            },
            (error) => {
            /* handle error — error is a FileError object */
            },
        )}
    )};

    await getEntries(directory);
    return entries;
}

function testAndLoadFile(file) {
    let ext = file.name.split('.').pop();
    if (debugFileHandling) console.log(file.name + ", " + file.size + ", " + ext);
    if(ext == 'usd' || ext == 'usdz' || ext == 'usda' || ext == 'usdc') {
        clearStage();
        // loadFile(file);
        loadingCallback(file);
    }
}