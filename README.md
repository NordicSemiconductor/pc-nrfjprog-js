# pc-nrfjprog-js

Node.js library that exposes the functionality of the [nRF5x Command-Line-Tools](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.tools/dita/tools/nrf5x_command_line_tools/nrf5x_command_line_tools_lpage.html?cp=5_1), used for development, programming, and debugging of Nordic Semiconductor's nRF5x SoCs (System on Chip).

## Installing

The library can be installed from npm, using:

    npm install pc-nrfjprog-js

This will pull down precompiled binaries for your platform/runtime environment. If precompiled binaries do not exist, then npm will try to compile them, which requires:

* Node.js (>=4)
* npm (>=3.7.0)
* CMake (>=2.8.12)
* A C/C++ toolchain

## Required setup

Before using the library, some platform specific setup is required. If you are seeing errors like `Errorcode: CouldNotFindJprogDLL (0x2)` or `Errorcode: CouldNotLoadDLL (0x3)` then please check that these tools are properly installed.

### Windows

Download [nRF-Command-Line-Tools](http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.tools%2Fdita%2Ftools%2Fnrf5x_command_line_tools%2Fnrf5x_installation.html) and run the installer (exe). This will install the required nrfjprog libraries and SEGGER J-Link.

Note that the nRF-Command-Line-Tools for Windows is only available in 32-bit at the moment. This means that 32-bit Node.js is required in order to use pc-nrfjprog-js on Windows.

### Linux/macOS

[SEGGER J-Link](https://www.segger.com/downloads/jlink/) must be downloaded and installed. The required nRF-Command-Line-Tools libraries are bundled together with pc-nrfjprog-js on Linux/macOS, so they do not have to be installed.

## API documentation

http://nordicsemiconductor.github.io/pc-nrfjprog-js/

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
