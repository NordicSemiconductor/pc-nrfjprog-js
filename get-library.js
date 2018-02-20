'use strict';

const https = require('https');
const tar = require('tar');
const fs = require('fs');
const sander = require('sander');
const os = require('os');
const path = require('path');
const opn = require('opn');


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
const LIB_DIR = path.join(DOWNLOAD_DIR, 'lib');

const PLATFORM_CONFIG = {
    linux: {
        // See https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux64/51386
        url: 'https://www.nordicsemi.com/eng/nordic/download_resource/51386/27/17243451/94917',
        destinationFile: path.join(DOWNLOAD_DIR, 'nrfjprog-linux64.tar'),
    },
    darwin: {
        // See https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-OSX/53402
        url: 'https://www.nordicsemi.com/eng/nordic/download_resource/53402/19/93375824/99977',
        destinationFile: path.join(DOWNLOAD_DIR, 'nrfjprog-darwin.tar'),
    },
    win32: {
        // See https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Win32/33444
        url: 'https://www.nordicsemi.com/eng/nordic/download_resource/33444/47/97153666/53210',
        destinationFile: path.join(DOWNLOAD_DIR, 'nrfjprog-win32.exe'),
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
                response.on('end', () => {
                    file.end();
                    resolve();
                });
            }
        });
    });
}

function extractTarFile(filePath, outputDir) {
    console.log(`Extracting nrfjprog from ${filePath} to ${outputDir}`);
    return tar.extract({
        file: filePath,
        filter: pathToFile => pathToFile.match(/\.so|\.dylib$|\.h$/),
        strip: 2,
        cwd: outputDir,
    });
}

const platform = os.platform();
const platformConfig = PLATFORM_CONFIG[platform];

if (!platformConfig) {
    throw new Error(`Unsupported platform: '${platform}'. Cannot get nRF5x command-line tools.`);
}

function getLibraryVersion() {
    return new Promise((resolve, reject) => {
        try {
            // eslint-disable-next-line global-require
            const nrfjprog = require('.');
            nrfjprog.getLibraryVersion((err, version) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(version);
                }
            });
        } catch (ex) {
            reject(ex);
        }
    });
}

// Check if nrfjprog binary libraries are working or not
getLibraryVersion()
    .then(version => {
        console.log('nrfjprog libraries at version ', version, ', no need to fetch them');
    })
    .catch(err => {
        // Windows-specific: check if the header files for nrfjprog are there. If they are,
        // assume the nrfjprog files are available in the system, do not download, do not
        // prompt information.
        if (platform === 'win32') {
            try {
                fs.accessSync(path.join(process.env['ProgramFiles(x86)'], 'Nordic Semiconductor', 'nrf5x', 'bin', 'headers', 'nrfjprog.h'));
            } catch (ex) {
                return Promise.reject(err);
            }
            return Promise.resolve();
        }
        return Promise.reject(err);
    })
    .catch(err => {
        let tryAgainAfterwards;
        if (err.errno === 2 && err.errcode === 'CouldNotFindJprogDLL') {
            // This will happen if the binary bindings have already been built
            // (with node-gyp) but the *.so files are not found.
            tryAgainAfterwards = true;
        } else if (err.message.match(/^Could not locate the bindings file/)) {
            // This will happen on the first install, where the bindings are not
            // yet built, and they need the jprog header files.
            tryAgainAfterwards = false;
        } else {
            // Nothing to do here - this block only handles problems with the nrf-jprog libs
            // (from Nordic), not with the Jlink-ARM libs (from Segger).
            console.log(err.message);
            return Promise.resolve();
        }

        console.log('nrfjprog libraries seem to be missing.');
        console.log(`Downloading nrfjprog from ${platformConfig.url} to ${platformConfig.destinationFile}`);

        return Promise.resolve()
            .then(() => sander.mkdir(DOWNLOAD_DIR))
            .then(() => downloadFile(platformConfig.url, platformConfig.destinationFile))
            .then(() => {
                if (platformConfig.destinationFile.endsWith('.tar')) {
                    return sander.mkdir(LIB_DIR).then(() =>
                        extractTarFile(platformConfig.destinationFile, LIB_DIR),
                    );
                }
                return Promise.resolve();
            })
            .then(() => {
                if (platformConfig.spawnChild) {
                    console.log('Installation of pc-nrfjprog-js requires running ', platformConfig.spawnChild);
                    return opn(platformConfig.spawnChild);
                }
                return Promise.resolve();
            })
            .then(() => {
                if (platformConfig.instructions) {
                    return Promise.resolve(console.warn(platformConfig.instructions));
                } else if (tryAgainAfterwards) {
                    // Try and see if it works now.
                    return getLibraryVersion().then(version => {
                        console.log('Automated fetch of nrfjprog seems to have worked, now at ', version);
                    });
                }
                return Promise.resolve();
            })
            .catch(error => console.log(`Error when getting nrfjprog: ${error.message}`))
            .then(() => {
                if (sander.existsSync(platformConfig.destinationFile)) {
                    return sander.rm(platformConfig.destinationFile)
                        .catch(error => console.log(`Unable to remove ${platformConfig.destinationFile}: ${error.message}`));
                }
                return Promise.resolve();
            });
    });
