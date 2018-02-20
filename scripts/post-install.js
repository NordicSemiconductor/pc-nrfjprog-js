// Small script run *after* the node binary module is either fetched or compiled.

/*
 * Both nrfjprog and J-link libraries are required for pc-nrfjprog-js to function.
 * This script warns the user if these libs are missing and points them to the
 * right place to look for a solution.
 */

'use strict';

const chalk = require('chalk');

function getLibraryVersion() {
    return new Promise((resolve, reject) => {
        try {
            // eslint-disable-next-line global-require
            const nrfjprog = require('..');
            nrfjprog.getLibraryVersion((err, version) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(version);
                }
            });
        } catch (ex) {
            reject(ex);
        }
    });
}

getLibraryVersion()
    .then(version => {
        console.log('nrfjprog libraries are at version', version);
        console.log('J-Link libraries seem to be working as expected.');
    })
    .catch(err => {
        console.log();
        console.log(chalk.red.bold(`Found error: ${err.errcode}`));

        const isNrfjprogError = err.errno === 2 && err.errcode === 'CouldNotFindJprogDLL';
        const isJlinkError = err.errno === 7 && err.errcode === 'CouldNotOpenDLL'
            && err.lowlevelError === 'JLINKARM_DLL_NOT_FOUND';

        if (isNrfjprogError) {
            console.log('WARNING: The automated setup of nrfjprog libraries failed. The ' +
                'pc-nrfjprog-js library cannot function without them.');
            console.log(chalk.bold('Please install nRF5x Command Line Tools manually from ' +
                'the Nordic Semiconductor website.'));
        } else if (isJlinkError) {
            console.log('WARNING: The J-Link libraries were not found on your system. The ' +
                'pc-nrfjprog-js library cannot function without them.');
            console.log(chalk.bold('Please visit https://www.segger.com/downloads/jlink/ and ' +
                'install the "J-Link Software and Documentation Pack".'));
        } else {
            console.log('The pc-nrfjprog-js post-install check failed unexpectedly:', err.message);
        }

        console.log();
        process.exit(1);
    });
