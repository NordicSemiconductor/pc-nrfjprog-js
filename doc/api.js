
// This is a mock module, exporting the function fingerprints that are defined somewhere
// else in the .h / .cpp files.
//
// The goal is to be able to run JSdoc against this file to generate fancy API docs.
//
// TODO: Move these docstrings to the corresponding .h/.cpp file
// TODO: Become able to run JSdoc in .h/.cpp files, with a JSdoc plugin a la https://www.npmjs.com/package/jsdoc-jsx

/**
 *
 * The <tt>pc-nrfjprog-js</tt> module exposes the functionality to the
 * nRF5x Command-line tools
 * to your nodeJS programs.
 *
 * @example
 * let nrfjprogjs = require('pc-nrfjprog-js');
 *
 * nrfjprogjs.getConnectedDevices(function(err, devices) {
 *     console.log('There are ' + devices.length + ' nRF devices connected.');
 * });
 *
 * @module pc-nrfjprog-js
 */

/**
 * Possible error.<br/>
 * If an operation completed sucessfully, the error passed to the callback
 * function will be <tt>undefined</tt> (and thus, falsy).<br/>
 * This will be an instance of the built-in {@link https://nodejs.org/dist/latest/docs/api/errors.html#errors_class_error|Error} class, with some extra properties:
 * @typedef {Error} Error
 * @property {integer} errno
 *    The error number. Value will be one of the following predefined constants:<br/>
 *    <tt>nrfjprogjs.CouldNotFindJlinkDLL</tt><br/>
 *    <tt>nrfjprogjs.CouldNotFindJProgDLL</tt><br/>
 *    <tt>nrfjprogjs.CouldNotOpenDevice</tt><br/>
 *    <tt>nrfjprogjs.CouldNotOpenDLL</tt><br/>
 *    <tt>nrfjprogjs.CouldNotConnectToDevice</tt><br/>
 *    <tt>nrfjprogjs.CouldNotCallFunction</tt><br/>
 *    <tt>nrfjprogjs.CouldNotErase</tt><br/>
 *    <tt>nrfjprogjs.CouldNotProgram</tt><br/>
 *    <tt>nrfjprogjs.CouldNotRead</tt><br/>
 *    <tt>nrfjprogjs.CouldNotOpenHexFile</tt><br/>
 *
 * @property {String} errcode A human-readable version of the error code.
 * @property {String} erroroperation The internal function that caused the error.
 * @property {String} errmsg Error string. The value will be equal to that of the built-in <tt>message</tt> property.
 * @property {integer} lowlevelErrorNo The low-level error code, if applicable.
 * @property {String} lowlevelError A human-readable version of the low-level error code.
 * @property {String} log The complete log from the internal functions.
 *
 * @example
 * nrfprogjs.getDllVersion(function(err, version){
 *     if (err) {
 *         throw err;
 *     } else {
 *         // success
 *     }
 * });
 *
 * @example
 * nrfprogjs.program(serialNumber, file, programmingOptions, function(err){
 *     if (err && err.errno === nrfprogjs.CouldNotOpenHexFile) {
 *          console.error('.hex file not found');
 *     }
 * });
 */


/**
 * Represents a semver-like version number, e.g. 9.6.0 as an object of the form
 *    <tt>{ major: 9, minor: 6, revision: 0 }</tt>
 * @typedef Version
 * @property {integer} major The major version number
 * @property {integer} minor The minor version number
 * @property {integer} revision The revision number
 */

/**
 * Represents information of an individual device
 * @typedef DeviceInformation
 *
 * @property {integer} family
 *    Device family. Value will be equal to one of the following predefined constants:<br/>
 *    <tt>nrfjprogjs.NRF51_FAMILY</tt><br/>
 *    <tt>nrfjprogjs.NRF52_FAMILY</tt><br/>
 *    <tt>nrfjprogjs.UNKNOWN_FAMILY</tt><br/>
 *
 * @property {integer} deviceType
 *    Type of device. Value will be equal to one of the following predefined constants:<br/>
 *    <tt>nrfjprogjs.NRF51xxx_xxAA_REV1</tt><br/>
 *    <tt>nrfjprogjs.NRF51xxx_xxAA_REV2</tt><br/>
 *    <tt>nrfjprogjs.NRF51xxx_xxAA_REV3</tt><br/>
 *    <tt>nrfjprogjs.NRF51xxx_xxAB_REV3</tt><br/>
 *    <tt>nrfjprogjs.NRF51xxx_xxAC_REV3</tt><br/>
 *    <tt>nrfjprogjs.NRF51802_xxAA_REV3</tt><br/>
 *    <tt>nrfjprogjs.NRF52832_xxAA_ENGA</tt><br/>
 *    <tt>nrfjprogjs.NRF52832_xxAA_ENGB</tt><br/>
 *    <tt>nrfjprogjs.NRF52832_xxAA_REV1</tt><br/>
 *    <tt>nrfjprogjs.NRF52840_xxAA_ENGA</tt><br/>
 *    <tt>nrfjprogjs.NRF52832_xxAA_FUTURE</tt><br/>
 *    <tt>nrfjprogjs.NRF52840_xxAA_FUTURE</tt><br/>
 *    <tt>nrfjprogjs.NRF52810_xxAA_REV1</tt><br/>
 *    <tt>nrfjprogjs.NRF52810_xxAA_FUTURE</tt><br/>
 *    <tt>nrfjprogjs.NRF52832_xxAB_REV1</tt><br/>
 *    <tt>nrfjprogjs.NRF52832_xxAB_FUTURE</tt><br/>
 *    <tt>nrfjprogjs.NRF51801_xxAB_REV3</tt><br/>
 *
 * @property {integer} codeAddress
 * @property {integer} codePageSize
 * @property {integer} codeSize
 *
 * @property {integer} uicrAddress
 * @property {integer} infoPageSize
 *
 * @property {boolean} codeRamPresent
 * @property {integer} codeRamAddress
 * @property {integer} dataRamAddress
 * @property {integer} ramSize
 *
 * @property {boolean} qspiPresent
 * @property {integer} xipAddress
 * @property {integer} xipSize
 *
 * @property {integer} pinResetPin
 */

/**
 * Represents the serial number and information of an individual device
 * @typedef SerialNumberAndDeviceInformation
 *
 * @property {string} serialNumber
 * @property {module:pc-nrfjprog-js~DeviceInformation} deviceInfo
 */

/**
 * Option flags to be used when sending a program to the device.
 * @typedef ProgramOptions
 * @property {integer} inputFormat=nrfjprogjs.INPUT_FORMAT_HEX_FILE
 *    How the <tt>filename</tt> string passed to <tt>program()</tt> shall be interpreted.
 *    Value must be one of:<br/>
 *    <tt>nrfjprogjs.INPUT_FORMAT_HEX_FILE</tt>: The string represents a filename for a .hex file<br/>
 *    <tt>nrfjprogjs.INPUT_FORMAT_HEX_STRING</tt>: The string represents the contents of a .hex file<br/>
 * @property {boolean} verify=true
 *    Whether verification should be performed as part of the programming.
 *    Akin to <tt>nrfjprog --program --verify</tt> in the command-line tools
 * @property {integer} chip_erase_mode=nrfjprogjs.ERASE_ALL
 *    How much of the flash memory should be erased. Value must be one of:<br/>
 *    <tt>nrfjprogjs.ERASE_NONE</tt><br/>
 *    <tt>nrfjprogjs.ERASE_ALL</tt><br/>
 *    <tt>nrfjprogjs.ERASE_PAGES</tt><br/>
 *    <tt>nrfjprogjs.ERASE_PAGES_INCLUDING_UICR</tt><br/>
 * @property {boolean} qspi_erase_mose=nrfjprogjs.ERASE_NONE
 *    How much of the QSPI memory should be erased. Value must be one of:<br/>
 *    <tt>nrfjprogjs.ERASE_NONE</tt><br/>
 *    <tt>nrfjprogjs.ERASE_ALL</tt><br/>
 *    <tt>nrfjprogjs.ERASE_PAGES</tt><br/>
 *    <tt>nrfjprogjs.ERASE_PAGES_INCLUDING_UICR</tt><br/>
 * @property {boolean} reset Whether the device should be reset after programming
 */

/**
 * Erase flags to be used when sending a program to the device.
 * @typedef EraseOptions
 * @property {boolean} erase_mode
 *    How much of the memory should be erased. Value must be one of:<br/>
 *    <tt>nrfjprogjs.ERASE_NONE</tt><br/>
 *    <tt>nrfjprogjs.ERASE_ALL</tt><br/>
 *    <tt>nrfjprogjs.ERASE_PAGES</tt><br/>
 *    <tt>nrfjprogjs.ERASE_PAGES_INCLUDING_UICR</tt><br/>
 * @property {boolean} start_address
 *    Start erasing from this address. Only relevant when using <tt>ERASE_PAGES</tt> or <tt>ERASE_PAGES_INCLUDING_UICR</tt> modes.
 * @property {boolean} end_address
 *    Erasing up to this address. Only relevant when using <tt>ERASE_PAGES</tt> or <tt>ERASE_PAGES_INCLUDING_UICR</tt> modes.
 */

/**
 * Async function to get the version of the nrfjprog DLL in use.
 *
 * @example
 * nrfjprogjs.getDllVersion( function(err, version) {
 *      if (err) throw err;
 *      console.log( version.major + '.' + version.minor + '.' + version.revision ) // e.g. 9.6.0
 * } );
 *
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect two parameters: ({@link module:pc-nrfjprog-js~Error|Error}, {@link module:pc-nrfjprog-js~Version|Version}).
 */
export function getDllVersion() {}

/**
 * Async function to get a list of all connected devices.
 *
 * @example
 * nrfjprogjs.getConnectedDevices( function(err, devices) {
 *      if (err) throw err;
 *      for (let i = 0; i < devices.length; i++) {
 *          console.log(
 *              devices[i].serialNumber +
 *              ' has ' +
 *              devices[i].deviceInfo.ramSize +
 *              ' bytes of RAM'
 *          );
 *      }
 * } );
 *
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect two parameters: ({@link module:pc-nrfjprog-js~Error|Error}, Array of {@link module:pc-nrfjprog-js~SerialNumberAndDeviceInformation|SerialNumberAndDeviceInformation}).
 */
export function getConnectedDevices(callback) {}



/**
 * Async function to get information of a single device, given its serial number.
 *
 * @example
 * nrfjprogjs.getDeviceInfo(123456789, function(err, info) {
 *      if (err) throw err;
 *      console.log('Selected device has' + info.ramSize + ' bytes of RAM');
 * } );
 *
 * @param {integer} serialNumber The serial number of the device to query
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect two parameters: ({@link module:pc-nrfjprog-js~Error|Error}, {@link module:pc-nrfjprog-js~DeviceInformation|DeviceInformation}).
 */
export function getDeviceInfo(serialNumber, callback) {}



/**
 * Async function to read a chunk of memory. The data received by the callback
 * is an array of integers, each of them representing a single byte (with values
 * from 0 to 255).
 * <br/>
 *
 * Please note that the data is an array of numbers - it is NOT a {@link https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint8Array|UInt8Array},
 * and it is NOT a {@link https://nodejs.org/api/buffer.html|Buffer}.
 * <br/>
 *
 * This is the same functionality as running "<tt>nrfjprog --memrd</tt>" in the command-line tools.
 *
 * @example
 * nrfjprogjs.read(123456789, 0, 16, function(err, data) {
 *      if (err) throw err;
 *      console.log('The first 16 bytes of memory look like: ' + data.join(','));
 * } );
 *
 * @param {integer} serialNumber The serial number of the device to read memory from
 * @param {integer} address The start address of the block of memory to be read
 * @param {integer} length The amount of bytes to be read
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect two parameters: ({@link module:pc-nrfjprog-js~Error|Error}, Array of integers).
 */
export function read(serialNumber, address, length, callback) {}



// TODO: What is the endianness of this???
/**
 * Async function to read a single 4-byte word from memory.
 * <br/>
 *
 * Please note that the data is a number - it is NOT a {@link https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint32Array|UInt32Array},
 * and it is NOT a {@link https://nodejs.org/api/buffer.html|Buffer}.
 * <br/>
 *
 * This is the same functionality as running "<tt>nrfjprog --memrd -w</tt>" in the command-line tools.
 *
 * @example
 * nrfjprogjs.read(123456789, 0, function(err, data) {
 *      if (err) throw err;
 *      console.log('The first word of memory looks like: ' + data);
 * } );
 *
 * @param {integer} serialNumber The serial number of the device to read memory from
 * @param {integer} address The address of the word to be read
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect two parameters: ({@link module:pc-nrfjprog-js~Error|Error}, integer).
 */
export function readU32(serialNumber, address, callback) {}



/**
 * Async function to push a program to the device.
 * <br/>
 *
 * This is the same functionality as running "<tt>nrfjprog --program</tt>" in the command-line tools.
 *
 * @example
 * nrfjprogjs.program(123456789, "/some/path/nrf52832_abcd.hex", {}, function(err) {
 *      if (err) throw err;
 * } );
 *
 * @param {integer} serialNumber The serial number of the device to program
 * @param {string} filename Either the filename of the <tt>.hex</tt> file containing the program, or the contents of such a file.
 * @param {module:pc-nrfjprog-js~ProgramOptions} options A plain object containing options about how to push the program.
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function program(serialNumber, filename, options, callback) {}


// TODO
//     static NAN_METHOD(ReadToFile); // Params: serialNumber, filename, options {readram, readcode, readuicr, readqspi}, callback(error)
export function readToFile(serialNumber, filename, options) {}


/**
 * Async function to verify the program in the device
 * <br/>
 *
 * This is the same functionality as running "<tt>nrfjprog --verify</tt>" in the command-line tools.
 *
 * @example
 * nrfjprogjs.verify(123456789, "/some/path/nrf52832_abcd.hex", function(err) {
 *      if (err) throw err;
 * } );
 *
 * @param {integer} serialNumber The serial number of the device
 * @param {string} filename The filename of the <tt>.hex</tt> file containing the program.
 * @param {Object} options={} Reserved for future use.
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function verify(serialNumber, filename, callback) {}



/**
 * Async function to erase a chunk of memory.
 * <br/>
 *
 * This is the same functionality as running "<tt>nrfjprog --erasepage</tt>" in the command-line tools.<br/>
 *
 * Will not erase a locked device. To erase a locked device, use {@link module:pc-nrfjprog-js~recover|recover}
 *
 * @param {integer} serialNumber The serial number of the device
 * @param {module:pc-nrfjprog-js~EraseOptions} options Options on how to erase the device memory
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function erase(serialNumber, options, callback) {}



/**
 * Async function to recover a device
 * <br/>
 *
 * Recover the whole chip by removing all user accessible content.
 * <br/>
 *
 * This is the same functionality as running "<tt>nrfjprog --recover</tt>" in the command-line tools.
 *
 * @param {integer} serialNumber The serial number of the device to recover
 * @param {module:pc-nrfjprog-js~EraseOptions} options Options on how to perform the memory recovery
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function recover(serialNumber, options, callback) {}


// TODO: Check that this equates to "--memwr" and not to "--ramwr"
/**
 * Async function to write data to a device's memory, given an array of byte values.
 * <br/>
 *
 * Please use an array of numbers -  a {@link https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint8Array|UInt8Array}
 * might work due to type casting, but a {@link https://nodejs.org/api/buffer.html|Buffer} will
 * most likely fail.
 * <br/>
 *
 * @param {integer} serialNumber The serial number of the device to write memory to
 * @param {integer} address The start address of the block of memory to be written
 * @param {Array.integer} data Array of byte values to be written
 * @param {Function} callback A callback function to handle the async response
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function write(serialNumber, address, data, callback) {}


// TODO: Check that this equates to "--memwr" and not to "--ramwr"
/**
 * Async function to write data to a device's memory, given the value for a single 4-byte word.
 * <br/>
 *
 * Please use a single number as the parameter - do NOT use a
 * {@link https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint32Array|UInt32Array}
 * or a {@link https://nodejs.org/api/buffer.html|Buffer}.
 * <br/>
 *
 * @param {integer} serialNumber The serial number of the device to write memory to
 * @param {integer} address Address of the memory word to be written
 * @param {integer} data Value to be written
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function writeU32(serialNumber, address, data, callback) {}







