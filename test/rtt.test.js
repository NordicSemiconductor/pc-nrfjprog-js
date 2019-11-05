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

let device;

jasmine.DEFAULT_TIMEOUT_INTERVAL = 100000;

describe('RTT', () => {
    beforeAll(done => {
        const programCallback = err => {
            expect(err).toBeUndefined();

            done();
        };

        const callback = (err, connectedDevices) => {
            expect(err).toBeUndefined();
            expect(connectedDevices.length).toBeGreaterThanOrEqual(1);

            // We only have firmware for the nRF52 at the moment. RTT also works with nRF51, but with another firmware
            let nrf52devices = connectedDevices.filter(device => device.deviceInfo.family === nRFjprog.NRF52_FAMILY);

            device = nrf52devices[0];

            expect(device).toBeDefined();

            nRFjprog.program(device.serialNumber, "./test/hex/rtt.hex", { }, programCallback);
        };

        nRFjprog.getConnectedDevices(callback);
    });

    describe('opens and close as expected', () => {
        it('starts a rtt session', (done) => {
            const stopCallback = (err) => {
                expect(err).toBeUndefined();
                done();
            };

            const startCallback = (err, down, up) => {
                expect(err).toBeUndefined();
                expect(down).toBeDefined();
                expect(up).toBeDefined();

                nRFjprog.rttStop(device.serialNumber, stopCallback);
            };

            nRFjprog.rttStart(device.serialNumber, {}, startCallback);
        });

        it('starts a rtt session when using correct control block location', (done) => {
            const stopCallback = (err) => {
                expect(err).toBeUndefined();
                done();
            };

            const startCallback = (err, down, up) => {
                expect(err).toBeUndefined();
                expect(down).toBeDefined();
                expect(up).toBeDefined();

                nRFjprog.rttStop(device.serialNumber, stopCallback);
            };

            nRFjprog.rttStart(device.serialNumber, { controlBlockLocation: 0x200006E0 }, startCallback);
        });

        it('returns an error when wrong serialnumber', done => {
            const startCallback = (err, down, up) => {
                expect(err).toBeDefined();
                expect(err).toMatchSnapshot();

                done();
            };

            nRFjprog.rttStart(0, {}, startCallback);
        });

        it('returns an error when using wrong control block location', done => {
            const startCallback = (err, down, up) => {
                expect(err).toBeDefined();
                expect(err).toMatchSnapshot();

                done();
            };

            nRFjprog.rttStart(device.serialNumber, { controlBlockLocation: 15 }, startCallback);
        });
    });

    describe('fails cleanly when calling other functions before start', () => {
        it('fails cleanly when calling read without start', (done) => {
            const readCallback = (err, data, raw, time) => {
                expect(err).toBeDefined();
                expect(err).toMatchSnapshot();

                done();
            };

            nRFjprog.rttRead(device.serialNumber, 0, 100, readCallback);
        });

        it('fails cleanly when calling write without start', (done) => {
            const writeCallback = (err, length, time) => {
                expect(err).toBeDefined();
                expect(err).toMatchSnapshot();

                done();
            };

            nRFjprog.rttWrite(device.serialNumber, 0, "test", writeCallback);
        });

        it('fails cleanly when calling stop without start', (done) => {
            const stopCallback = (err) => {
                expect(err).toBeDefined();
                expect(err).toMatchSnapshot();

                done();
            };

            nRFjprog.rttStop(device.serialNumber, stopCallback);
        });
    });

    describe('reads from device', () => {
        beforeEach(done => {
            const startCallback = (err, down, up) => {
                expect(err).toBeUndefined();
                expect(down).toBeDefined();
                expect(up).toBeDefined();

                done();
            };
            nRFjprog.rttStart(device.serialNumber, {}, startCallback);
        });

        afterEach(done => {
            const stopCallback = (err) => {
                expect(err).toBeUndefined();
                done();
            };

            nRFjprog.rttStop(device.serialNumber, stopCallback);
        });

        it('reads startmessage', done => {
            const readCallback = (err, data, raw, time) => {
                expect(err).toBeUndefined();
                expect(data).toMatchSnapshot();
                expect(raw).toMatchSnapshot();
                expect(time).toBeDefined();

                done();
            };

            nRFjprog.rttRead(device.serialNumber, 0, 100, readCallback);
        });

        it('reads the set number of bytes', done => {
            const readLength = 5;
            const readCallback = (err, data, raw) => {
                expect(err).toBeUndefined();
                expect(data.length).toBe(readLength);
                expect(raw.length).toBe(readLength);

                done();
            };

            nRFjprog.rttRead(device.serialNumber, 0, readLength, readCallback);
        });
    });

    describe('writes to device', () => {
        beforeEach(done => {
            const readCallback = (err, data, raw, time) => {
                expect(err).toBeUndefined();

                done();
            }

            const startCallback = (err, down, up) => {
                expect(err).toBeUndefined();
                expect(down).toBeDefined();
                expect(up).toBeDefined();

                // Clear the read buffers
                nRFjprog.rttRead(device.serialNumber, 0, 100, readCallback);
            };

            nRFjprog.rttStart(device.serialNumber, {}, startCallback);
        });

        afterEach(done => {
            const stopCallback = (err) => {
                expect(err).toBeUndefined();
                done();
            };

            nRFjprog.rttStop(device.serialNumber, stopCallback);
        });

        it('writes a text to loopback and reads it back', done => {
            const writetext = "this is a test";
            let readStartTime = Date.now();

            const readCallback = (err, data, raw) => {
                expect(err).toBeUndefined();

                if (data.length === 0
                    && Date.now() - readStartTime < 2000) {
                    nRFjprog.rttRead(device.serialNumber, 0, 100, readCallback);
                    return;
                }

                expect(data).toBe(writetext);

                done();
            };

            const writeCallback = (err, length, time) => {
                expect(err).toBeUndefined();
                expect(length).toBe(writetext.length);
                expect(time).toBeDefined();

                readStartTime = Date.now();
                nRFjprog.rttRead(device.serialNumber, 0, 100, readCallback);
            };

            nRFjprog.rttWrite(device.serialNumber, 0, writetext, writeCallback);
        });

        it('writes a arry of integers to loopback and reads it back', done => {
            const writearray = [0, 1, 2, 3];
            let readStartTime = Date.now();

            const readCallback = (err, data, raw) => {
                expect(err).toBeUndefined();

                if (raw.length === 0
                    && Date.now() - readStartTime < 2000) {
                    nRFjprog.rttRead(device.serialNumber, 0, 100, readCallback);
                    return;
                }

                expect(raw).toEqual(writearray);

                done();
            };

            const writeCallback = (err, length, time) => {
                expect(err).toBeUndefined();
                expect(length).toBe(writearray.length);
                expect(time).toBeDefined();

                readStartTime = Date.now();
                nRFjprog.rttRead(device.serialNumber, 0, 100, readCallback);
            };

            nRFjprog.rttWrite(device.serialNumber, 0, writearray, writeCallback);
        });
    });

    describe.skip('race condition', () => {
        afterEach(done => {
            const stopCallback = (err) => {
                expect(err).toBeUndefined();
                done();
            };

            nRFjprog.rttStop(device.serialNumber, stopCallback);
        });

        it('handle race conditions by waiting', done => {
            const readOrder = [];
            const startCallback = (err, down, up) => {
                expect(err).toBeUndefined();
                expect(down).toBeDefined();
                expect(up).toBeDefined();
            };

            const readCallback = (number, err, data, raw) => {
                readOrder.push(number);
                expect(err).toBeUndefined();

                if (readOrder.length === 3) {
                    expect(readOrder).toEqual(expect.arrayContaining([1, 2, 3]));
                    done();
                }
            };

            nRFjprog.rttStart(device.serialNumber, {}, startCallback);
            nRFjprog.rttRead(device.serialNumber, 0, 100, readCallback.bind(null, 1));
            nRFjprog.rttRead(device.serialNumber, 0, 100, readCallback.bind(null, 2));
            nRFjprog.rttRead(device.serialNumber, 0, 100, readCallback.bind(null, 3));
        });

        // A timeout of 10 seconds is to long to trip this up
        it.skip('returns an error when the call time exceeds 10 seconds', done => {
            const errCount = 0;
            const writeElementCount = 10000;
            const writeResults = [];

            const startCallback = (err, down, up) => {
                expect(err).toBeUndefined();
                expect(down).toBeDefined();
                expect(up).toBeDefined();
            };

            const writeCallback = (err, writeLength, time) => {
                if (err) {
                    errCount++;
                }

                writeResults.push(err);

                if (writeResults.length % 1000 === 0) {
                    console.log(writeResults.length);
                }

                if (writeResults.length === writeElementCount) {
                    expect(errCount).toBeGreaterThan(0);
                    done();
                }
            };

            nRFjprog.rttStart(device.serialNumber, {}, startCallback);

            for (let i = 0; i < readElementCount; i++) {
                nRFjprog.rttWrite(device.serialNumber, 0, "test", writeCallback);
            }
        });
    });
});


