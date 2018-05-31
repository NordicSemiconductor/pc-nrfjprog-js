/* Copyright (c) 2010 - 2017, Nordic Semiconductor ASA
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
 * Small script run *after* the node binary module is either fetched or compiled.
 *
 * Both nrfjprog and J-link libraries are required for pc-nrfjprog-js to function.
 * This script warns the user if these libs are missing and points them to the
 * right place to look for a solution.
 */

'use strict';

const chalk = require('chalk');

// SKIP_NRFJPROG_CHECK environment variable allows to skip checking
// for the purpose of publishing job on build server
const skipNrfjprogCheck = (process.env.SKIP_NRFJPROG_CHECK || false).toString().toUpperCase();
if (['1', 'ON', 'TRUE', 'Y', 'YES'].includes(skipNrfjprogCheck)) {
    console.log('Skipping nrfjprog library version check.');
    process.exit(0);
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

getLibraryVersion()
    .then(version => {
        console.log('nrfjprog libraries are at version', version);
        console.log('J-Link libraries seem to be working as expected.');
    })
    .catch(err => {
        console.error();
        console.error(chalk.red.bold(`Found error: ${err.errcode}`));

        const isNrfjprogError = err.errno === 2 && err.errcode === 'CouldNotFindJprogDLL';
        const isJlinkError = err.errno === 7 && err.errcode === 'CouldNotOpenDLL'
            && err.lowlevelError === 'JLINKARM_DLL_NOT_FOUND';

        if (isNrfjprogError) {
            console.error('WARNING: The automated setup of nrfjprog libraries failed. The ' +
                'pc-nrfjprog-js library cannot function without them.');
            console.error(chalk.bold('Please install nRF5x Command Line Tools manually from ' +
                'the Nordic Semiconductor website.'));
        } else if (isJlinkError) {
            console.error('WARNING: The J-Link libraries were not found on your system. The ' +
                'pc-nrfjprog-js library cannot function without them.');
            console.error(chalk.bold('Please visit https://www.segger.com/downloads/jlink/ and ' +
                'install the "J-Link Software and Documentation Pack".'));
        } else {
            console.error('The pc-nrfjprog-js post-install check failed unexpectedly:', err.message);
        }

        console.error();
        process.exit(1);
    });
