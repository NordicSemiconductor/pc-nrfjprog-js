/* Copyright (c) 2015 - 2017, Nordic Semiconductor ASA
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
 * This script extracts the nrfjprog libraries for the given platform under
 * the nrfjprog/unpacked directory (if not already done), and runs the tests
 * by spawning a jest process with LD_LIBRARY_PATH set to nrfjprog/unpacked.
 */

'use strict';

const path = require('path');
const fs = require('fs');
const tar = require('tar');
const os = require('os');
const spawn = require('child_process').spawn;

const jest = path.join(__dirname, 'node_modules', '.bin', 'jest');

const NRFJPROG_ARCHIVES_DIR = path.join(__dirname, 'nrfjprog');
const NRFJPROG_HOME = path.join(NRFJPROG_ARCHIVES_DIR, 'unpacked');
const PLATFORM_ARCHIVE_EXT = {
    linux: 'Linux-x86_64.tar',
    darwin: 'OSX.tar',
};
const PLATFORM = os.platform();

function dirExists(dir) {
    return new Promise(resolve => {
        fs.stat(dir, err => {
            if (err) {
                resolve(false);
            } else {
                resolve(true);
            }
        });
    });
}

function getTarFilePath() {
    return new Promise((resolve, reject) => {
        fs.readdir(NRFJPROG_ARCHIVES_DIR, (err, files) => {
            const fileExt = PLATFORM_ARCHIVE_EXT[PLATFORM];
            const tarFile = files.find(file => file.endsWith(fileExt));
            if (tarFile) {
                resolve(path.join(NRFJPROG_ARCHIVES_DIR, tarFile));
            } else {
                reject(new Error(`Unable to find tar file for ${PLATFORM}`));
            }
        });
    });
}

function extractTarFile(filePath, outputDir) {
    return new Promise((resolve, reject) => {
        const extractor = tar.Extract({ path: outputDir })
            .on('error', err => reject(err))
            .on('end', () => resolve());
        fs.createReadStream(filePath)
            .on('error', err => reject(err))
            .pipe(extractor);
    });
}

function setupNrfjprog() {
    if (PLATFORM === 'win32') {
        console.log('NOTE: Automatic setup of nrfjprog is not possible on Windows. ' +
            'If you see DLL errors, then run the installer from the nrfjprog directory.');
        return Promise.resolve();
    }
    return dirExists(NRFJPROG_HOME)
        .then(exists => {
            if (exists) {
                console.log(`The nrfjprog libraries already exist in ${NRFJPROG_HOME}. ` +
                    'No setup required.');
                return Promise.resolve();
            }
            return getTarFilePath()
                .then(filePath => extractTarFile(filePath, NRFJPROG_HOME))
                .then(() => console.log('The nrfjprog libraries have been set up in ' +
                    `${NRFJPROG_HOME}.`));
        });
}

function runTests() {
    // Spawning a new process when running jest. By doing this, we can specify
    // a custom LD_LIBRARY_PATH env variable so that the tests are able to load
    // the nrfjprog libs.
    const options = {
        env: Object.assign({}, process.env, {
            LD_LIBRARY_PATH: path.join(NRFJPROG_HOME, 'nrfjprog'),
        }),
        shell: true,
        stdio: 'inherit',
    };

    const argv = process.argv.slice(2);
    spawn(jest, argv, options).on('exit', code => {
        if (code !== 0) {
            return Promise.reject(new Error(`The jest process exited with status ${code}`));
        }
        return Promise.resolve();
    });
}

setupNrfjprog()
    .then(() => runTests())
    .catch(err => {
        console.error(`Error running tests: ${err.message}`);
        process.exit(1);
    });
