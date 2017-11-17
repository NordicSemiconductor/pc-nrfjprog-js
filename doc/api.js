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
 * Progress information.<br />
 * Long running operations can indicate progress. If the optional progress callback is used, this object will be sent when progress is made.
 * @typedef Progress
 * @property {string} process An indication of what subprocess is performed.
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
 * Represents information of an individual device.
 *
 * The fields in this data structure about non-volatile memory, RAM, UICR and QSPI can also
 * be found in the product specifications available
 * at http://infocenter.nordicsemi.com, under the "Memory" section of each product model.
 *
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
 * @property {integer} codeAddress  Memory address for the start of the non-volatile (flash) memory block.
 *   Typically <tt>0x0000 0000</tt>.
 * @property {integer} codePageSize Size of each page of non-volatile (flash) memory.
 * @property {integer} codeSize     Total size of the non-volatile (flash) memory
 *
 * @property {integer} uicrAddress  Memory address for the start of the UICR
 *   (User Information Configuration Registers). Typically <tt>0x1000 1000</tt>.
 * @property {integer} infoPageSize Size of the FICR/UICR. Typically 4KiB.
 *
 * @property {integer} dataRamAddress Memory address for the start of the volatile RAM.
 *   Typically <tt>0x2000 0000</tt>, in the SRAM memory region.
 * @property {integer} ramSize        Size of the volatile RAM, in bytes.
 * @property {boolean} codeRamPresent Whether the volatile RAM is also mapped to a executable memory region or not.
 * @property {integer} codeRamAddress Memory address for the volatile RAM, in the code memory region.
 *   When <tt>codeRamPresent</tt> is true, both <tt>codeRamAddress</tt> and
 *   <tt>dataRamAddress</tt> point to the same volatile RAM, but the hardware
 *   uses a different data bus in each case.
 *
 * @property {boolean} qspiPresent  Whether QSPI (Quad Serial Peripheral Interface) is present or not.
 * @property {integer} xipAddress   When <tt>qspiPresent</tt> is true, the memory address for the
 *   XIP (eXecute In Place) feature. This memory address maps to the external flash
 *   memory connected through QSPI.
 * @property {integer} xipSize      Size of the XIP memory region.
 *
 * @property {integer} pinResetPin  Which pin acts as the reset pin. e.g. a value of <tt>21</tt>
 *   means that the pin marked as "P0.21" acts as the reset pin.
 */

/**
 * Represents the device information of the debug probe
 *
 * @typedef ProbeInformation
 *
 * @property {integer} serialNumber The serialnumber of the probe
 * @property {integer} clockSpeedkHz The clock speed of the probe interface
 * @property {string} firmwareString The version infomation about the J-Link firmware
 */

/**
 * Represents the information about the J-link ARM interface library
 *
 * @typedef LibraryInformation
 *
 * @property {object} version The version of the interface library
 * @property {integer} version.major The major version of the interface library
 * @property {integer} version.minor The minor version of the interface library
 * @property {string} version.revision The revision version of the interface library
 * @property {string} path The path to the interface library
 */

/**
 * Represents the serial number and information of an individual device
 * @typedef SerialNumberAndDeviceInformation
 *
 * @property {integer} serialNumber
 * @property {module:pc-nrfjprog-js~DeviceInformation} deviceInfo
 * @property {module:pc-nrfjprog-js~ProbeInformation} probeInfo
 * @property {module:pc-nrfjprog-js~LibraryInformation} libraryInfo
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
 * @property {integer} qspi_erase_mode=nrfjprogjs.ERASE_NONE
 *    How much of the QSPI memory should be erased. Value must be one of:<br/>
 *    <tt>nrfjprogjs.ERASE_NONE</tt><br/>
 *    <tt>nrfjprogjs.ERASE_ALL</tt><br/>
 *    <tt>nrfjprogjs.ERASE_PAGES</tt><br/>
 *    <tt>nrfjprogjs.ERASE_PAGES_INCLUDING_UICR</tt><br/>
 * @property {boolean} reset=true Whether the device should be reset after programming.
 */

/**
 * Option flags to be used when reading the content of the device.
 * @typedef ReadToFileOptions
 * @property {boolean} readram=false Read the contents of the ram
 * @property {boolean} readcode=true Read the contents of the flash
 * @property {boolean} readuicr=false Read the contents of the uicr
 * @property {boolean} readqspi=false Read the contents of the qspi
  */

/**
 * Flags to be used when erasing a device.
 * @typedef EraseOptions
 * @property {integer} erase_mode=nrfjporgjs.ERASE_ALL
 *    How much of the memory should be erased. Value must be one of:<br/>
 *    <tt>nrfjprogjs.ERASE_NONE</tt><br/>
 *    <tt>nrfjprogjs.ERASE_ALL</tt><br/>
 *    <tt>nrfjprogjs.ERASE_PAGES</tt><br/>
 *    <tt>nrfjprogjs.ERASE_PAGES_INCLUDING_UICR</tt><br/>
 * @property {integer} start_address=0
 *    Start erasing from this address. Only relevant when using <tt>ERASE_PAGES</tt> or <tt>ERASE_PAGES_INCLUDING_UICR</tt> modes.
 * @property {integer} end_address=0
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
export function getDllVersion(callback) {}

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
 * Async function to get information of a single device, given its serial number.
 *
 * @example
 * nrfjprogjs.getProbeInfo(123456789, function(err, info) {
 *      if (err) throw err;
 *      console.log('Selected device has the following clockspeed ' + info.clockSpeedkHz + 'kHz');
 * } );
 *
 * @param {integer} serialNumber The serial number of the device to query
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect two parameters: ({@link module:pc-nrfjprog-js~Error|Error}, {@link module:pc-nrfjprog-js~ProbeInformation|ProbeInformation}).
 */
export function getProbeInfo(serialNumber, callback) {}



/**
 * Async function to get information about the low level library used by the device, given its serial number.
 *
 * @example
 * nrfjprogjs.getLibrayInfo(123456789, function(err, info) {
 *      if (err) throw err;
 *      console.log('Selected device use ' + info.path + ' to connect');
 * } );
 *
 * @param {integer} serialNumber The serial number of the device to query
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect two parameters: ({@link module:pc-nrfjprog-js~Error|Error}, {@link module:pc-nrfjprog-js~LibraryInformation|LibraryInformation}).
 */
export function getLibraryInfo(serialNumber, callback) {}

/**
 * Async function to read a chunk of memory. The data received by the callback
 * is an array of integers, each of them representing a single byte (with values
 * from 0 to 255).
 * <br/>
 *
 * The read operation happens without verifying that the addresses are accessible or
 * even exist. Note that if the target address is in unpowered RAM, the operation will fail.
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
 * The read operation happens without verifying that the addresses are accessible or
 * even exist. The address parameter needs to be 32-bit aligned (must be a multiple of 4).
 * Note that if the target address is in unpowered RAM, the operation will fail.
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
 * This is the same functionality as running "<tt>nrfjprog --program</tt>" in the command-line tools.<br />
 *
 * If the {@link module:pc-nrfjprog-js~ProgramOptions|ProgramOption} chip_erase_mode is ERASE_ALL, this function will recover the device if it initially is not allowed to program the device due to protection.
 *
 * @example
 * nrfjprogjs.program(123456789, "/some/path/nrf52832_abcd.hex", {}, function(err) {
 *      if (err) throw err;
 * } );
 *
 * @param {integer} serialNumber The serial number of the device to program
 * @param {string} filename Either the filename of the <tt>.hex</tt> file containing the program, or the contents of such a file.
 * @param {module:pc-nrfjprog-js~ProgramOptions} options A plain object containing options about how to push the program.
 * @param {Function} [progressCallback] Optional parameter for getting progress callbacks. It shall expect one parameter: ({@link module:pc-nrfjprog-js~Progress|Progress}).
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function program(serialNumber, filename, options, progressCallback, callback) {}


/**
 * Async function to read memory from the device and write the results into a file.
 * <br />
 *
 * The read operation happens without verifying that the addresses are accessible or
 * even exist. Note that if the target address is in unpowered RAM, the operation will fail.
 * <br/>
 *
 * This is the same functionality as running "<tt>nrfjprog --readcode</tt>" in the command-line tools.
 * @example
 * nrfjprogjs.readToFile(123456789, "/some/path/to/store/file.hex", {}, function(err) {
 *      if (err) throw err;
 * } );
 *
 * @param {integer} serialNumber The serial number of the device to read
 * @param {string} filename The filename of the <tt>.hex</tt> file where the content of the device should be stored.
 * @param {module:pc-nrfjprog-js~ReadToFileOptions} options A plain object containing options about what to read.
 * @param {Function} [progressCallback] Optional parameter for getting progress callbacks. It shall expect one parameter: ({@link module:pc-nrfjprog-js~Progress|Progress}).
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function readToFile(serialNumber, filename, options, progressCallback, callback) {}


/**
 * Async function to verify the program in the device
 * <br/>
 *
 * Compares the contents of the provided .hex file against the contents of the memory of the device connected.<br/>
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
 * @param {Function} [progressCallback] Optional parameter for getting progress callbacks. It shall expect one parameter: ({@link module:pc-nrfjprog-js~Progress|Progress}).
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function verify(serialNumber, filename, options, progressCallback, callback) {}



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
 * @param {Function} [progressCallback] Optional parameter for getting progress callbacks. It shall expect one parameter: ({@link module:pc-nrfjprog-js~Progress|Progress}).
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function erase(serialNumber, options, progressCallback, callback) {}


/**
 * Async function to recover a device
 * <br/>
 *
 * This operation attempts to recover the device and leave it as it was when it left Nordic factory. It will attempt to
 * connect, erase all user available flash, halt and eliminate any protection. Note that this operation may take up to 30 s
 * if the device was readback protected. Note as well that this function only affects internal flash and CPU, but does not
 * erase, reset or stop any peripheral, oscillator source nor extarnally QSPI-connected flash. The operation will therefore
 * leave the watchdog still operational if it was running.<br/>
 *
 * This is the same functionality as running "<tt>nrfjprog --recover</tt>" in the command-line tools.
 *
 * @param {integer} serialNumber The serial number of the device to recover
 * @param {Function} [progressCallback] Optional parameter for getting progress callbacks. It shall expect one parameter: ({@link module:pc-nrfjprog-js~Progress|Progress}).
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function recover(serialNumber, progressCallback, callback) {}


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

/**
 * Async function to open (and keep open) a connection to a device.
 *
 * By default, all other function calls implicitly open a connection, perform
 * an operation, reset the device and close the connection to the device.<br/>
 *
 * This can impact performance negatively. In order to prevent the extra steps
 * (open, reset, close), one can explicitly <tt>open()</tt> and <tt>close()</tt>
 * a connection to a device. This will keep a connection open, allowing all other
 * function calls to execute faster, and resetting the device only once (when
 * the connection is closed).<br />
 *
 * Open connections shall be closed by calling {@link module:pc-nrfjprog-js~close|close}
 *
 * If a connection to a device is opened, then all subsequent calls will use the
 * already-opened connection. Opening a connection twice has no effect. Closing
 * a connection twice will close it on the first `close()` call: the second
 * one will have no effect.<br />
 *
 * @example
 * nrfjprogjs.read(123456789, 0, function(err, data) {
 *
 *   nrfjprogjs.read(123456789, 0x0000, 0x1000, function(err1, data1) {
 *     if (err1) throw err;
 *
 *     nrfjprogjs.read(123456789, 0x1000, 0x1000, function(err2, data2) {
 *       if (err2) throw err;
 *
 *       nrfjprogjs.close(123456789, function() {});
 *     } );
 *   } );
 * } );
 *
 * @param {integer} serialNumber The serial number of the device to open
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function open(serialNumber, callback) {}

/**
 * Async function to close a connection to a device opened by
 * {@link module:pc-nrfjprog-js~open|open}.<br />
 *
 * @param {integer} serialNumber The serial number of the device to close
 * @param {Function} callback A callback function to handle the async response.
 *   It shall expect one parameter: ({@link module:pc-nrfjprog-js~Error|Error}).
 */
export function close(serialNumber, callback) {}
