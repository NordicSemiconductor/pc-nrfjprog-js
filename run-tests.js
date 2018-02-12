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

function runTests() {
    // Spawning a new process when running jest. This is legacy: before,
    // a custom LD_LIBRARY_PATH env variable was set so that the tests are able to load
    // the nrfjprog libs.
    const options = {
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

runTests()
.catch(err => {
    console.error(`Error running tests: ${err.message}`);
    process.exit(1);
});
