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

'use strict';

const nrfjprog = require('../index.js');

let nRFjprog;
let device;

describe('Single device - non-destructive', () => {
    beforeAll(done => {
        nRFjprog = new nrfjprog.nRFjprog();

        const callback = (err, connectedDevices) => {
            device = connectedDevices[0];
            done();
        };

        nRFjprog.getConnectedDevices(callback);
    });

    it('finds correct family', done => {
        const callback = (err, family) => {
            expect(err).toBeUndefined();
            expect(family).toBe(device.family);
            done();
        };

        nRFjprog.getFamily(device.serialNumber, callback);
    });

    it('finds device version', done => {
        const callback = (err, deviceVersion) => {
            expect(err).toBeUndefined();
            expect(deviceVersion).toBe(5);
            done();
        };

        nRFjprog.getDeviceVersion(device.serialNumber, callback);
    });

    it('throws an error when device do not exist', done => {
        const callback = (err, family) => {
            expect(err).toMatchSnapshot();
            expect(family).toBeUndefined();
            done();
        };

        nRFjprog.getFamily(1, callback);
    });

    it('reads from specified address', done => {
        const callback = (err, contents) => {
            expect(err).toBeUndefined();
            expect(contents).toBeDefined();
            done();
        };

        nRFjprog.read(device.serialNumber, 0x0, 1, callback);
    });

    it('reads 5 bytes from specified address', done => {
        const readLength = 5;

        const callback = (err, contents) => {
            expect(err).toBeUndefined();
            expect(contents).toBeDefined();
            expect(contents.length).toBe(readLength);
            done();
        };

        nRFjprog.read(device.serialNumber, 0x0, readLength, callback);
    });

    it('reads unsigned 32 from specified address', done => {
        const callback = (err, contents) => {
            expect(err).toBeUndefined();
            expect(contents).toBeDefined();
            done();
        };

        nRFjprog.readU32(device.serialNumber, 0x0, callback);
    });
});
