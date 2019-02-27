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

const cmakeJS = require('cmake-js');
const os = require('os');

function build(debug, target)
{
    const { npm_config_runtime, npm_config_target, npm_config_arch } = process.env;

    const runtime = npm_config_runtime || 'node';
    const runtimeVersion = npm_config_target || process.version.substr(1);
    const arch = npm_config_arch || (process.plaform == 'win32' ? os.arch() : undefined);

    const options = {
        runtime,
        runtimeVersion,
        arch,
        debug,
        preferGnu: true,
        target,
    };

    if (process.platform === 'win32') {
        if (process.arch === 'ia32') {
            options.generator = 'Visual Studio 15 2017';
        } else if (process.arch === 'x64') {
            options.generator = 'Visual Studio 15 2017 Win64';
        } else {
            console.log(`${process.arch} is not supported on Windows`);
        }
    }

    const buildSystem = new cmakeJS.BuildSystem(options);
    buildSystem.rebuild();
}

let times = 0;

function main(args = []) {
    const debug = args.includes('--debug');
    const target = args.includes('--target') ? args[args.indexOf('--target') + 1] : undefined;

    try {
        build(debug, target);
    } catch(e) {
        if (e.code != 'MODULE_NOT_FOUND') {
            throw e;
        }
        times += 1;
        if (times == 5) {
            throw e;
        }
        setTimeout(() => main(args), 2000);
    }
}

main(process.argv);
