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

// This is a mock module, exporting the function fingerprints that are defined somewhere
// else in the .h / .cpp files.
//
// The goal is to be able to run JSdoc against this file to generate fancy API docs.
//
// TODO: Move these docstrings to the corresponding .h/.cpp file
// TODO: Become able to run JSdoc in .h/.cpp files, with a JSdoc plugin a la https://www.npmjs.com/package/jsdoc-jsx

/**
 *
 * <p>The <tt>pc-nrfjprog-js.RTT</tt> module exposes the functionality to the RTT functionality.
 * RTT is a high-speed protocol specified by SEGGER, available on all Nordic Semiconductor
 * development kits with a SEGGER chip. More details about the protocol may be found at
 * SEGGERS product pages:
 * {@link https://www.segger.com/products/debug-probes/j-link/technology/real-time-transfer/about-real-time-transfer/|Real Time Transfer}.</p>
 *
 * <p>RTT works on the principle that the firmware sets up a buffer in RAM which it populates
 * with UP and DOWN circular buffers. A device may have multiple up and down buffers, called channels.</p>
 *
 * <p>When you want to write to a device, you choose a down channel and write data to it. The RTT
 * protocol will then copy the data into the chosen buffer of the chosen channel and update
 * the end pointer for that buffer. The firmware will subsequently detect that there is new data
 * in the buffer and can then read it.</p>
 *
 * <p>When you want to read from a device, the firmware chooses an up channel and writes data to it.
 * The RTT protocol will write data to the end of the buffer and update the end pointer for the
 * buffer of the chosen channel. When you subsequently read on that up channel, you will get
 * the data in the buffer, up to the specified amount of data you specified, and the start
 * pointer for the buffer will be updated.</p>
 *
 * <p>Due to <tt>nRFjprog</tt> resetting the device for all device specific function calls,
 * they may NOT be performed simultaneously.</p>
 *
 * <p>You may at any time have at most <strong>one</strong> RTT connection open.</p>
 *
 * @example
 * const nRFjprogjs = require('pc-nrfjprog-js');
 * const RTT = nrfjprogjs.RTT;
 *
 * RTT.start(12345678, {}, function(err, down, up) {
 *      RTT.write(0, 'Some data to write', function(err, length, timeSinceRTTStartInUs) {
 *          RTT.read(0, 100, function(err, stringData, rawData, timeSinceRTTStartInUs) {
 *              RTT.stop(function(err) {
 *                  console.log('Stopped');
 *              });
 *          });
 *      });
 * });
 *
 * @module pc-nrfjprog-js.RTT
 */

/**
 * Possible error.<br/>
 * If an operation completed sucessfully, the error passed to the callback
 * function will be <tt>undefined</tt> (and thus, falsy).<br/>
 * This will be an instance of the built-in {@link https://nodejs.org/dist/latest/docs/api/errors.html#errors_class_error|Error} class, with some extra properties:
 * @typedef {Error} Error
 * @property {integer} errno
 *    The error number. Value will be one of the following predefined constants:<br/>
 *    <tt>nrfjprogjs.RTTCouldNotLoadHighlevelLibrary</tt><br/>
 *    <tt>nrfjprogjs.RTTCouldNotOpenHighlevelLibrary</tt><br/>
 *    <tt>nrfjprogjs.RTTCouldNotGetDeviceInformation</tt><br/>
 *    <tt>nrfjprogjs.RTTCouldNotLoadnRFjprogLibrary</tt><br/>
 *    <tt>nrfjprogjs.RTTCouldNotOpennRFjprogLibrary</tt><br/>
 *    <tt>nrfjprogjs.RTTCouldNotConnectToDevice</tt><br/>
 *    <tt>nrfjprogjs.RTTCouldNotStartRTT</tt><br/>
 *    <tt>nrfjprogjs.RTTCouldNotFindControlBlock</tt><br/>
 *    <tt>nrfjprogjs.RTTCouldNotGetChannelInformation</tt><br/>
 *    <tt>nrfjprogjs.RTTCouldNotCallFunction</tt><br/>
 *    <tt>nrfjprogjs.RTTNotInitialized</tt><br/>
 *    <tt>nrfjprogjs.RTTCouldNotExecuteDueToLoad</tt><br/>
 *
 * @property {String} errcode A human-readable version of the error code.
 * @property {String} erroroperation The internal function that caused the error.
 * @property {String} errmsg Error string. The value will be equal to that of the built-in <tt>message</tt> property.
 * @property {integer} lowlevelErrorNo The low-level error code, if applicable.
 * @property {String} lowlevelError A human-readable version of the low-level error code.
 * @property {String} log The complete log from the internal functions.
 *
 * @example
 * RTT.start(12345678, {}, function(err, down, up){
 *     if (err) {
 *         throw err;
 *     } else {
 *         // success
 *     }
 * });
 *
 * @example
 * RTT.start(12345678, {}, function(err, down, up){
 *     if (err && err.errno === nrfjprogjs.RTTCouldNotFindControlBlock) {
 *          console.error('The firmware is not RTT capable');
 *     }
 * });
 */

/**
 * Information about the different up and down channels available on the device. A down channel is
 * a channel from the computer to the device. An up channel is a channel from the device to the
 * computer.
 * @typedef ChannelInfo
 * @property {integer} channelIndex The index used to address this channel
 * @property {integer} direction The direction of the channel. Value will be one of <tt>nrfjprogjs.UP_DIRECTION</tt> or <tt>nrfjprogjs.DOWN_DIRECTION</tt>
 * @property {String} name The name of the channel
 * @property {integer} size The size of the channel
 */

 /**
 * Option flags to be used when starting RTT. This may speed up the process of locating the control block, and the RTT Start. If <tt>controlBlockLocation</tt>
 * specified, only that location will be searched for the RTT control block, and an error will be returned if no control block where found. If no value
 * is specified for <tt>controlBlockLocation</tt>, the RAM will be searched for the location of the RTT control block.
 * @typedef StartOptions
 * @property {integer} [controlBlockLocation] The location of the control block. If this location is not the start of the RTT control block, start will fail.
 */

/**
 * <p>Async function to start RTT.</p>
 *
 * <p>This function will attempt to open an RTT connection to the device with serialnumber
 * <tt>serialNumber</tt>. It will return an {@link pc-nrfjprog-js.module:RTT~Error|Error}
 * if the device does not exist or if its firmware doesn't support RTT.</p>
 *
 * <p>The RTT protocol uses down channels to write to the device and up channels to read from the device.</p>
 *
 * <p>You can only open RTT on one device at any given time.</p>
 *
 * <p>When you have an open RTT session, you should not call any functions in <tt>pc-nrfjprog-js</tt>,
 * as these will reset the device.<p>
 *
 * <p>When you are done with the RTT session, you should call <tt>stop()</tt>.</p>
 *
 * @example
 * RTT.start(12345678, {}, function(err, down, up){
 *      if (err) console.error('The firmware is not RTT capable');
 *
 *      console.log('Down channels');
 *      down.map(function(channel) {
 *             console.log('Index:', channel.channelIndex);
 *             console.log('Direction:', channel.direction);
 *             console.log('Name:', channel.name);
 *             console.log('Size:', channel.size);
 *      });
 *
 *      console.log('Up channels');
 *      up.map(function(channel) {
 *             console.log('Index:', channel.channelIndex);
 *             console.log('Direction:', channel.direction);
 *             console.log('Name:', channel.name);
 *             console.log('Size:', channel.size);
 *      });
 * });
 *
 * @param {integer} serialNumber The serial number of the device to start RTT on
 * @param {StartOptions} startOptions A plain object containing options about how to start RTT
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect three parameters: ({@link pc-nrfjprog-js.module:RTT~Error|Error}, Array of {@link pc-nrfjprog-js.module:RTT~ChannelInfo|ChannelInfo},
 *   Array of {@link pc-nrfjprog-js.module:RTT~ChannelInfo|ChannelInfo})
 */
export function start(serialnumber, startoptions, callback) {}

/**
 * Async function to stop RTT.
 *
 * @example
 * RTT.stop(function(err){
 *     if (err) console.error('Stopping RTT failed');
 * });
 *
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link pc-nrfjprog-js.module:RTT~Error|Error})
 */
export function stop(callback) {}

/**
 * Async function to read RTT contents from an up channel on the device. You read on the up channel specified by
 * the <tt>channelIndex</tt>. The data returned are attempted converted to a string (<tt>stringData</tt> in the
 * example), and as a raw, data array containing the bytes read from the channel (<tt>rawData</tt> in the example).
 * For the string callback value, the data will be encoded as UTF8.
 *
 * @example
 * RTT.read(0, 100, function(err, stringData, rawData, timeSinceRTTStartInUs) {
 *      if (err) throw err;
 *      console.log(stringData);
 *      console.log(rawData);
 *      console.log('Time since start of RTT in micro seconds:', timeSinceRTTStartInUs)
 * } );
 *
 *
 * @param {integer} channelIndex The RTT up channel index to read from
 * @param {integer} length The max amout of bytes to read
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect four parameters: ({@link pc-nrfjprog-js.module:RTT~Error|Error}, String, Array of Integers, integer)
 */
export function read(channelIndex, length, callback) {}



/**
 * Async function to write data to a down channel on the device. You write on the down channel specified by
 * the <tt>channelIndex</tt>. The <tt>data</tt> written may either be a string or an array of integers. String
 * data will be UTF8 encoded.
 *
 * @example
 * RTT.write(0, 'Start command', function(err, length, timeSinceRTTStartInUs) {
 *      if (err) throw err;
 *      console.log('Amount of bytes written', length);
 *      console.log('Time since start of RTT in micro seconds:', timeSinceRTTStartInUs)
 * } );
 *
 * @example
 * RTT.write(0, [0, 1, 2, 3, 4], function(err, length, timeSinceRTTStartInUs) {
 *      if (err) throw err;
 *      console.log('Amount of bytes written', length);
 *      console.log('Time since start of RTT in micro seconds:', timeSinceRTTStartInUs)
 * } );
 *
 * @param {integer} channelIndex The RTT down channel index to write to
 * @param {string|integer[]} data The data to send
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect three parameters: ({@link pc-nrfjprog-js.module:RTT~Error|Error}, integer, integer)
 */
export function write(channelIndex, data, callback) {}

