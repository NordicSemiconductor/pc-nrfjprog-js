# pc-nrfjprog-js

Node.js library that exposes the functionality of the [nRF Command Line Tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools), used for development, programming, and debugging of Nordic Semiconductor's SoCs (System on Chip).

## Installing

The library can be installed from npm, using:

    npm install pc-nrfjprog-js

This will pull down precompiled binaries for your platform/runtime environment. If precompiled binaries do not exist, then npm will try to compile them, which requires:

* Node.js (>=4)
* npm (>=3.7.0)
* CMake (>=2.8.12)
* A C/C++ toolchain

As part of the installation procedure, pc-nrfjprog-js will check if it can access the nrfjprog libraries, and verify that they are up to date. If not, it will try to install/upgrade these libraries.

## Building from source

The library can be built from source, using:

    npm run build

The library can be build from source while installing, using:

    npm install --build-from-source

Building from source requires the tools from Installing to be installed.

## Required setup

[SEGGER J-Link Software](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) must be downloaded and installed.

If you are seeing errors like `Errorcode: CouldNotLoadDLL (0x3)` then please check that the J-Link library is properly installed.

## API documentation

https://nordicsemiconductor.github.io/pc-nrfjprog-js/

## Example

```
const nrfjprogjs = require('pc-nrfjprog-js');

nrfjprogjs.getConnectedDevices(function(err, devices) {
    console.log('There are ' + devices.length + ' nRF devices connected.');
});
```

## Tests

The project has integration tests that run against a devkit/dongle. Note that these tests will erase the contents on the connected devkit/dongle. To run the tests:

    npm test
