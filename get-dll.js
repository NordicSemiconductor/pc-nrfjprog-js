'use strict';

const https = require('https');
const tar = require('tar');
const fs = require('fs');
const sander = require('sander');
const os = require('os');
const path = require('path');
const exec = require('child_process').exec ;

/*
 * nRF5x-Command-Line-Tools (nrfjprog) is required for programming. This script
 * downloads nrfjprog for the current platform.
 *
 * On Linux/macOS, the nrfjprog libraries will be placed in pc-nrfjprog-js's directory,
 * so that nodejs can find them. The script extracts
 * the nrfjprog tar file, and copies the libraries to the correct directory.
 *
 * In win32 platforms, this just downloads the the installer and logs a message
 * to tell the user they should run it.
 */

const DOWNLOAD_DIR = path.join(__dirname, 'nrfjprog');

const PLATFORM_CONFIG = {
    linux: {
        // See https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux64/51386
        url: 'https://www.nordicsemi.com/eng/nordic/download_resource/51386/27/17243451/94917',
        destinationFile: path.join(DOWNLOAD_DIR, 'nrfjprog-linux64.tar'),
        extractTo: path.join(DOWNLOAD_DIR, 'unpacked'),
        copyFiles: {
            source: path.join(DOWNLOAD_DIR, 'unpacked', 'nrfjprog'),
            destination: `${__dirname}/`,
            pattern: /\.so/,
        },
    },
    darwin: {
        // See https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-OSX/53402
        url: 'https://www.nordicsemi.com/eng/nordic/download_resource/53402/19/93375824/99977',
        destinationFile: path.join(DOWNLOAD_DIR, 'nrfjprog-darwin.tar'),
        extractTo: path.join(DOWNLOAD_DIR, 'unpacked'),
        copyFiles: {
            source: path.join(DOWNLOAD_DIR, 'unpacked', 'nrfjprog'),
            destination: `${__dirname}/`,
            pattern: /\.dylib/,
        },
    },
    win32: {
        // See https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Win32/33444
        // When changing this, remember to also update the nrfjprog version in installer.nsh
        url: 'https://www.nordicsemi.com/eng/nordic/download_resource/33444/47/97153666/53210',
        destinationFile: path.join(DOWNLOAD_DIR, 'nrfjprog-win32.exe'),
        instructions: "WARNING: You must manually install the latest nRF5x command line tools on this platform. Please check the " + DOWNLOAD_DIR + " directory and run the \"nrfjprog-win32.exe\" installer that you will find there.",
    },
};

function downloadFile(url, destinationFile) {
    return new Promise((resolve, reject) => {
        const file = fs.createWriteStream(destinationFile);
        https.get(url, response => {
            const statusCode = response.statusCode;
            if (statusCode !== 200) {
                reject(new Error(`Unable to download ${url}. Got status code ${statusCode}`));
            } else {
                response.pipe(file);
                response.on('end', () => resolve());
            }
        });
    });
}

function extractTarFile(filePath, outputDir) {
    console.log(`Extracting nrfjprog from ${filePath} to ${outputDir}`);

    return new Promise((resolve, reject) => {
        const extractor = tar.extract({ cwd: outputDir })
            .on('error', err => reject(err))
            .on('end', () => resolve());
        fs.createReadStream(filePath)
            .on('error', err => reject(err))
            .pipe(extractor);
    });
}

const platform = os.platform();
const platformConfig = PLATFORM_CONFIG[platform];

if (!platformConfig) {
    throw new Error(`Unsupported platform: '${platform}'. Cannot get nrfjprog command-line tools.`);
}

const dllVersion = new Promise((res, rej)=>{
    try {
        const nrfjprog = require('.');
        nrfjprog.getDllVersion((err, version)=>{
            if (err) { rej(err) } else {res(version)}
        })
    } catch(ex) {
        rej(ex);
    }
})


// Check if binary libraries are really needed
dllVersion
.then((version)=>{
    console.log('nrfjprog libraries at version ', version, ', no need to fetch them');
})
.catch((err)=>{
    console.log(err);
    console.log(`Downloading nrfjprog from ${platformConfig.url} to ${platformConfig.destinationFile}`);

    Promise.resolve()
        .then(sander.mkdir(DOWNLOAD_DIR))
        .then(() => downloadFile(platformConfig.url, platformConfig.destinationFile))
        .then(() => {
            if (platformConfig.extractTo) {
                return sander.mkdir(platformConfig.extractTo).then(()=>
                    extractTarFile(platformConfig.destinationFile, platformConfig.extractTo)
                );
            }
            return Promise.resolve();
        })
        .then(() => {
            if (platformConfig.copyFiles) {
                const copyConfig = platformConfig.copyFiles;
                console.log(`Copying nrfjprog libs from ${copyConfig.source} to ${copyConfig.destination}`);
                const files = sander.lsr(copyConfig.source);

                return Promise.all([
                    // copy .so / .dylib files
                    files
                        .then(filenames=>
                            filenames.filter(filename=>filename.match(copyConfig.pattern))
                        )
                        .then(filenames=>{
                            return Promise.all(filenames.map(filename=>{
                                return sander.symlinkOrCopy(copyConfig.source, filename).to(copyConfig.destination, filename);
                            })
                        );
                    }),

                    // copy .h files
                    files
                        .then(filenames=>
                            filenames.filter(filename=>filename.match(/\.h$/))
                        )
                        .then(filenames=>{
                            return Promise.all(filenames.map(filename=>{
                                return sander.symlinkOrCopy(copyConfig.source, filename).to(DOWNLOAD_DIR, 'include', filename);
                            })
                        );
                    }),
                ]);
            }
            return Promise.resolve();
        })
        .then(()=>{
            if (platformConfig.instructions) {
                console.warn(platformConfig.instructions);
            }
        })
        .catch(error => console.log(`Error when getting nrfjprog: ${error.message}`));
});


