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

const nRFjprog = require('../index.js');

let device;
jasmine.DEFAULT_TIMEOUT_INTERVAL = 100000;

describe('Single device - non-destructive', () => {
    beforeAll(done => {
        const callback = (err, connectedDevices) => {
            expect(err).toBeUndefined();
            expect(connectedDevices.length).toBeGreaterThanOrEqual(1);
            device = connectedDevices[0];

            done();
        };

        nRFjprog.getConnectedDevices(callback);
    });

    it('finds correct device info', done => {
        const callback = (err, deviceInfo) => {
            expect(err).toBeUndefined();
            expect(deviceInfo).toMatchObject(device.deviceInfo);
            done();
        };

        nRFjprog.getDeviceInfo(device.serialNumber, callback);
    });

    it('finds correct probe info', done => {
        const callback = (err, probeInfo) => {
            expect(err).toBeUndefined();
            expect(probeInfo).toMatchObject(device.probeInfo);
            expect(probeInfo).toHaveProperty('serialNumber');
            expect(probeInfo).toHaveProperty('clockSpeedkHz');
            expect(probeInfo).toHaveProperty('firmwareString');
            done();
        };

        nRFjprog.getProbeInfo(device.serialNumber, callback);
    });

    it('finds correct library info', done => {
        const callback = (err, libraryInfo) => {
            expect(err).toBeUndefined();
            expect(libraryInfo).toMatchObject(device.libraryInfo);
            expect(libraryInfo).toHaveProperty('version');
            expect(libraryInfo.version).toHaveProperty('major');
            expect(libraryInfo.version).toHaveProperty('minor');
            expect(libraryInfo.version).toHaveProperty('revision');
            expect(libraryInfo).toHaveProperty('path');
            done();
        };

        nRFjprog.getLibraryInfo(device.serialNumber, callback);
    });

    it('throws an error when device do not exist', done => {
        const callback = (err, deviceInfo) => {
            expect(err).toMatchSnapshot();
            expect(deviceInfo).toBeUndefined();
            done();
        };

        nRFjprog.getDeviceInfo(1, callback);
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

    it('keeps connection open when using open/close', done => {
        const readLength = 10;

        const callback = (err, contents) => {
            expect(err).toBeUndefined();
            nRFjprog.close(device.serialNumber, (err) => {
                expect(err).toBeUndefined();
                expect(contents.length).toBe(readLength);
                done();
            });
        };

        nRFjprog.open(device.serialNumber, (err) => {
            expect(err).toBeUndefined();
            nRFjprog.read(device.serialNumber, 0x0, readLength, callback);
        });
    });

    it('calling open twice returns an error', done => {
        nRFjprog.open(device.serialNumber, (err) => {
            expect(err).toBeUndefined();
            nRFjprog.open(device.serialNumber, (err) => {
                expect(err).toBeDefined();
                done();
            });
        });
    });

    it('should be able to reopen after close', done => {
        nRFjprog.open(device.serialNumber, (err) => {
            expect(err).toBeUndefined();
            nRFjprog.close(device.serialNumber, (err) => {
                expect(err).toBeUndefined();
                nRFjprog.open(device.serialNumber, (err) => {
                    expect(err).toBeUndefined();
                    nRFjprog.close(device.serialNumber, (err) => {
                        expect(err).toBeUndefined();
                        done();
                    });
                });
            });
        });
    });

    it('calling close twice has no effect', done => {
        nRFjprog.open(device.serialNumber, (err) => {
            expect(err).toBeUndefined();
            nRFjprog.close(device.serialNumber, (err) => {
                expect(err).toBeUndefined();
                nRFjprog.close(device.serialNumber, (err) => {
                    expect(err).toBeUndefined();
                    done();
                });
            });
        });
    });

    it('reads more than 0x10000 bytes', done => {
        const readLength = 0x10004;

        const callback = (err, contents) => {
            expect(err).toBeUndefined();
            expect(contents).toBeDefined();
            expect(contents.length).toBe(readLength);
            done();
        };

        nRFjprog.read(device.serialNumber, 0x0, readLength, callback);
    });
});
