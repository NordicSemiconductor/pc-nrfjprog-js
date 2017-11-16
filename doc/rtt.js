
// This is a mock module, exporting the function fingerprints that are defined somewhere
// else in the .h / .cpp files.
//
// The goal is to be able to run JSdoc against this file to generate fancy API docs.
//
// TODO: Move these docstrings to the corresponding .h/.cpp file
// TODO: Become able to run JSdoc in .h/.cpp files, with a JSdoc plugin a la https://www.npmjs.com/package/jsdoc-jsx

/**
 *
 * The <tt>pc-nrfjprog-js.RTT</tt> module exposes the functionality to the RTT functionality.
 *
 * @example
 * const nrfjprogjs = require('pc-nrfjprog-js');
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
 * RTT0.start(12345678, {}, function(err, down, up){
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
 * Information about the different up and down channels available on the device
 * @typedef ChannelInfo
 * @property {integer} channelIndex The index used to address this channel
 * @property {String} name The name of the channel
 * @property {integer} size The size of the channel
 */

 /**
 * Option flags to be used when starting RTT. This may speed up the process of locating the control block, and the RTT Start.
 * @typedef StartOptions
 * @property {integer} controlBlockLocation The location of the control block. If this location is not the start of the RTT control block, start will fail.
 */


/**
 * Async function to start RTT
 *
 * @example
 * RTT.start(12345678, {}, function(err, down, up){
 *      if (err) console.error('The firmware is not RTT capable');
 *
 *      console.log('Down channels');
 *      down.map(function(channel) {
 *             console.log('Index:', channel.channelIndex);
 *             console.log('Name:', channel.name);
 *             console.log('Size:', channel.size);
 *      });
 *
 *      console.log('Up channels');
 *      up.map(function(channel) {
 *             console.log('Index:', channel.channelIndex);
 *             console.log('Name:', channel.name);
 *             console.log('Size:', channel.size);
 *      });
 * });
 *
 * @param {integer} serialNumber The serial number of the device to start RTT on
 * @param {integer} startOptions A plain object containing options about how to start RTT
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect three parameters: ({@link module:pc-nrfjprog-js~Error|Error}, Array of {@link module:pc-nrfjprog-js~Version|ChannelInfo},
 *   Array of {@link module:pc-nrfjprog-js~Version|ChannelInfo})
 */
export function start(serialnumber, startoptions, callback) {}

/**
 * Async function to stop RTT
 *
 * @example
 * RTT.stop(function(err){
 *     if (err) console.error('Stopping RTT failed');
 * });
 *
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error})
 */
export function stop(callback) {}

/**
 * Async function to read RTT contents on a device.
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
 * @param {integer} channelIndex The RTT channel to read from
 * @param {integer} length The max amout of data to read
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect four parameters: ({@link module:pc-nrfjprog-js~Error|Error}, String, Array of Integers, integer)
 */
export function read(channelIndex, length, callback) {}



/**
 * Async function to write data to the device
 *
 * @example
 * RTT.write(0, 'Start command', function(err, length, timeSinceRTTStartInUs) {
 *      if (err) throw err;
 *      console.log('Amount of data written', length);
 *      console.log('Time since start of RTT in micro seconds:', timeSinceRTTStartInUs)
 * } );
 *
 * @example
 * RTT.write(0, [0, 1, 2, 3, 4], function(err, length, timeSinceRTTStartInUs) {
 *      if (err) throw err;
 *      console.log('Amount of data written', length);
 *      console.log('Time since start of RTT in micro seconds:', timeSinceRTTStartInUs)
 * } );
 *
 * @param {integer} channelIndex The RTT channel to write to
 * @param {string|integer[]} data The data to send
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect three parameters: ({@link module:pc-nrfjprog-js~Error|Error}, integer, integer)
 */
export function write(channelIndex, data, callback) {}

