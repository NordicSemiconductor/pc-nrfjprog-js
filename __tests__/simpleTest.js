/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in or with a processor manufactured by Nordic
 *   Semiconductor ASA, or in or with a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *   5. Any software provided in binary or object form under this license must not be
 *   reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

'use strict';

const nrfjprog = require('../index.js');

let debugProbe;
let device;

describe('Test nrfjprog integration', () => {
    beforeEach(() => {
        debugProbe = new nrfjprog.DebugProbe();
    });

    describe('Generic functionality', () => {
        it('gets dll version', done => {
            const callback = (err, version) => {
                expect(err).toBeUndefined();
                expect(version).toHaveProperty('major');
                expect(version).toHaveProperty('minor');
                expect(version).toHaveProperty('revision');
                done();
            };

            debugProbe.getDllVersion(callback);
        });

        it('finds all connected devices', done => {
            const callback = (err, connectedDevices) => {
                expect(err).toBeUndefined();
                expect(connectedDevices.length).toBeGreaterThanOrEqual(1);
                expect(connectedDevices[0]).toHaveProperty('serialNumber');
                done();
            };

            debugProbe.getConnectedDevices(callback);
        });

        it('throws when wrong parameters are sent in', () => {
            expect(() => { debugProbe.getDllVersion(); }).toThrowErrorMatchingSnapshot();
        });
    });

    describe('Single-device', () =>{
        beforeAll(done => {
            const callback = (err, connectedDevices) => {
                device = connectedDevices[0];
                done();
            };

            debugProbe.getConnectedDevices(callback);
        });

        describe('Non-destructive functionality', () => {
            it('finds correct family', done => {
                const callback = (err, family) => {
                    expect(err).toBeUndefined();
                    expect(family).toBe(device.family);
                    done();
                }

                debugProbe.getFamily(device.serialNumber, callback);
            });

            it('throws an error when device do not exist', done => {
                const callback = (err, family) => {
                    expect(err).toMatchSnapshot();
                    expect(family).toBeUndefined();
                    done();
                }

                debugProbe.getFamily(1, callback);
            });

            it('reads from specified address', done => {
                const callback = (err, contents) => {
                    expect(err).toBeUndefined();
                    expect(contents).toBeDefined();
                    done();
                }

                debugProbe.read(device.serialNumber, 0x0, 1, callback);
            });
        });
    });
});


/*
let probe = new nrfjprog.DebugProbe();
let probe2 = new nrfjprog.DebugProbe();

probe.getDllVersion((err, dllVersion) => {
    if (err) {
        console.log(err, dllVersion);
        return;
    }

    console.log(dllVersion);

    probe.GetConnectedDevices((err, connectedDevices) => {
        if (err) {
            console.log(err);
            return;
        }

        console.log(connectedDevices);
    });
});
probe2.getDllVersion((err, dllVersion) => {
    if (err) {
        console.log(2, err, dllVersion);
        return;
    }

    console.log(2, dllVersion);

    probe2.GetConnectedDevices((err, connectedDevices) => {
        if (err) {
            console.log(2, err);
            return;
        }

        console.log(2, connectedDevices);
    });
});
*/
