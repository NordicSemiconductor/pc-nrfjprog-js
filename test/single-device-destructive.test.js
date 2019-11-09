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

'use strict';

const nRFjprog = require('../index.js');
const fs = require('fs');

let device;
jasmine.DEFAULT_TIMEOUT_INTERVAL = 100000;

let testfile = './test/hex/connectivity_1.1.0_1m_with_s132_3.0.hex';

describe('Single device - destructive', () => {
    beforeAll(done => {
        const callback = (err, connectedDevices) => {
            expect(err).toBeUndefined();
            expect(connectedDevices.length).toBeGreaterThanOrEqual(1);
            device = connectedDevices[0];
            if (device.deviceInfo.family === nRFjprog.NRF53_FAMILY) {
                testfile = './test/hex/while_true_nrf53_application.hex';
            }
            done();
        };

        nRFjprog.getConnectedDevices(callback);
    });

    it('erases the whole device', done => {
        const callback = (err) => {
            expect(err).toBeUndefined();
            done();
        };

        nRFjprog.erase(device.serialNumber, {}, callback);
    });

    it('programs a hex file', done => {

        const callback = (err) => {
            expect(err).toBeUndefined();

            nRFjprog.verify(device.serialNumber, testfile, { }, verifyCallback);
        };

        const verifyCallback = (err) => {
            expect(err).toBeUndefined();
            done();
        };

        nRFjprog.program(device.serialNumber, testfile, { }, callback);
    });

    it('reads device content', done => {
        const callback = (err) => {
            expect(err).toBeUndefined();
            done();
        };

        nRFjprog.readToFile(device.serialNumber, "./after_readToFile.hex", { readcode: true, readuicr: true }, callback);
    });

    it('verifies a hex file', done => {
        const callback = (err) => {
            expect(err).toBeUndefined();
            done();
        };

        nRFjprog.verify(device.serialNumber, testfile, { }, callback);
    });

    // There's an issue with the mocked progress callback under jest,
    // but it works fine otherwise, hence it's skipped here
    it.skip('verifies a hex file with progress callback', done => {
        const mockProgressCallback = jest.fn();

        const callback = (err) => {
            expect(err).toBeUndefined();
            // expect(mockProgressCallback).toHaveBeenCalled();
            done();
        };

        nRFjprog.verify(device.serialNumber, testfile, { }, mockProgressCallback, callback);
    });

    it('programs a hex string', done => {
        const callback = (err) => {
            expect(err).toBeUndefined();

            nRFjprog.verify(device.serialNumber, testfile, { }, verifyCallback);
        };

        const verifyCallback = (err) => {
            expect(err).toBeUndefined();
            done();
        };

        const filecontent = fs.readFileSync(testfile).toString('utf-8');

        nRFjprog.program(device.serialNumber, filecontent, { inputFormat: nRFjprog.INPUT_FORMAT_HEX_STRING }, callback);
    });

    it('recovers a device', done => {
        const callback = (err) => {
            expect(err).toBeUndefined();

            nRFjprog.readToFile(device.serialNumber, "./after_recover.hex", { readcode: true, readuicr: true }, readToFileCallback);
        };

        const readToFileCallback = (err) => {
            expect(err).toBeUndefined();
            done();
        };

        nRFjprog.recover(device.serialNumber, callback);
    });

    it('writes an array to a device', done => {
        const address = 0x1028;
        const data = [0, 1, 2, 3, 4, 5, 6];

        const callback = (err) => {
            expect(err).toBeUndefined();

            nRFjprog.read(device.serialNumber, address, data.length, readCallback);
        };

        const readCallback = (err, contents) => {
            expect(err).toBeUndefined();
            expect(contents).toBeDefined();
            expect(contents).toEqual(expect.arrayContaining(data));
            done();
        };

        nRFjprog.write(device.serialNumber, address, data, callback);
    });

    it('writes a 32 bit value to a device', done => {
        const address = 0x1024;
        const data = 0x12345678;

        const callback = (err) => {
            expect(err).toBeUndefined();

            nRFjprog.readU32(device.serialNumber, address, readCallback);
        };

        const readCallback = (err, contents) => {
            expect(err).toBeUndefined();
            expect(contents).toBeDefined();
            expect(contents).toEqual(data);
            done();
        };

        nRFjprog.writeU32(device.serialNumber, address, data, callback);
    });
});
