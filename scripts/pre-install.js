/* Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 3. This software, with or without modification, must only be used with a Nordic
 *    Semiconductor ASA integrated circuit.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * nRF5x Command Line Tools (nrfjprog) is required for pc-nrfjprog-js to function.
 * This script checks if nrfjprog libraries are found, and installs them if required.
 *
 * On Linux/macOS, the nrfjprog artifact (tar) is extracted into nrfjprog/ directory. This
 * directory will then contain both headers (required when building) and libraries
 * (required at runtime).
 *
 * On Windows, the nrfjprog installer (exe) is run, which installs the libraries and
 * headers in Program Files. The pc-nrfjprog-js library will then find the nrfjprog
 * libraries by doing a registry lookup.
 */

'use strict';

const axios = require('axios');
const tar = require('tar');
const fs = require('fs');
const sander = require('sander');
const path = require('path');
const semver = require('semver');
const crypto = require('crypto');

const { nrfjprog } = require('../package.json');

const DOWNLOAD_DIR = path.join(__dirname, '..', 'nrfjprog');
const BUILD_DIR = path.join(__dirname, '..', 'build', 'Release');
const DOWNLOAD_URL = 'https://github.com/NordicSemiconductor/pc-nrfjprog-js/releases/download/nrfjprog';

const requiredVersion = nrfjprog.version;
const platform = `${process.platform}_${process.arch}`;
const filename = `nrfjprog-${requiredVersion}-${platform}.tar.gz`;
const fileUrl = `${DOWNLOAD_URL}/${filename}`;
const destinationFile = path.join(DOWNLOAD_DIR, filename);
const skipDownload = process.env.RELEASE;

async function downloadChecksum() {
    console.log('Downloading', `${fileUrl}.md5`);
    const { status, data } = await axios.get(`${fileUrl}.md5`);
    if (status !== 200) {
        throw new Error(`Unable to download ${fileUrl}.md5. ` +
            `Got status code ${status}`);
    }
    return data.split(' ').shift();
}

async function downloadFile() {
    if (skipDownload === 'true') {
        console.log('RELEASE is set to true. Skip downloading nrfjprog.');
        return Promise.resolve();
    }

    const hash = crypto.createHash('md5');
    const expectedChecksum = await downloadChecksum();

    console.log(`Downloading nrfjprog from ${fileUrl}...`);
    await removeDirIfExists(DOWNLOAD_DIR)
    await sander.mkdir(DOWNLOAD_DIR);

    const { status, data: stream } = await axios.get(
        `${fileUrl}`,
        {responseType: 'stream'}
    );
    if (status !== 200) {
        throw new Error(`Unable to download ${fileUrl}. ` +
            `Got status code ${status}`);
    }

    return new Promise((resolve, reject) => {
        const file = fs.createWriteStream(destinationFile);
        stream.pipe(file);
        stream.on('data', data => hash.update(data));
        stream.on('error', reject);
        stream.on('end', () => {
            file.end();
            const calculatedChecksum = hash.digest('hex');
            if (calculatedChecksum !== expectedChecksum) {
                fs.unlinkSync(destinationFile);
                console.log('Calculated checksum:', calculatedChecksum);
                console.log('Expected checksum:  ', expectedChecksum);
                reject(new Error('Checksum verification failed.'));
            }
            resolve();
        });
    });
}

function extractTarFile(filePath, outputDir) {
    return sander.mkdir(outputDir)
        .then(() => tar.extract({
            file: filePath,
            strip: 1,
            keep: true,
            cwd: outputDir,
        }));
}

function getLibraryVersion() {
    return new Promise((resolve, reject) => {
        try {
            // eslint-disable-next-line global-require
            const nrfjprog = require('..');
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

function removeFileIfExists(filePath) {
    if (sander.existsSync(filePath)) {
        return sander.unlink(filePath);
    }
    return Promise.resolve();
}

function removeDirIfExists(dirPath) {
    if (sander.existsSync(dirPath)) {
        return sander.rimraf(dirPath);
    }
    return Promise.resolve();
}

function installNrfjprog(pathToArtifact) {
    if (pathToArtifact.endsWith('.tar') || pathToArtifact.endsWith('.tar.gz')) {
        console.log(`Extracting ${pathToArtifact} to ${DOWNLOAD_DIR}...`);
        return extractTarFile(pathToArtifact, DOWNLOAD_DIR)
            .then(() => sander.readdir(DOWNLOAD_DIR))
            .then(files => files.filter(fn => fn.match(/\.(so|dll|dylib)$/)))
            .then(files => files.forEach(
                fn => sander.symlinkOrCopySync(path.join(DOWNLOAD_DIR, fn)).to(path.join(BUILD_DIR, fn))
            ));
    }
    return Promise.reject(new Error(`Unsupported nrfjprog artifact: ${pathToArtifact}`));
}

let isInstallationRequired = false;
getLibraryVersion()
    .then(version => {
        const currentVersion = `${version.major}.${version.minor}.${version.revision}`;
        if (semver.lt(currentVersion, requiredVersion)) {
            console.log(`Found nrfjprog version ${currentVersion}, but ` +
                `${requiredVersion} is required`);
            isInstallationRequired = true;
        } else {
            console.log('Found nrfjprog libraries at required version', version);
        }
    })
    .catch(error => {
        console.log(`Validation of nrfjprog libraries failed: ${error.message}`);
        isInstallationRequired = true;
    })
    .then(() => {
        if (isInstallationRequired) {
            console.log('Trying to install nrfjprog');

            let exitCode = 0;
            return downloadFile()
                // without this sleep windows install fails with EBUSY
                .then(() => new Promise(resolve => setTimeout(resolve, 1000)))
                .then(() => installNrfjprog(destinationFile))
                .catch(error => {
                    exitCode = 1;
                    console.error(`Error when installing nrfjprog libraries: ${error.message}`);
                })
                .then(() => removeFileIfExists(destinationFile))
                .catch(error => {
                    exitCode = 1;
                    console.error(`Unable to remove downloaded nrfjprog artifact: ${error.message}`);
                })
                .then(() => process.exit(exitCode));
        }
        return Promise.resolve();
    });
