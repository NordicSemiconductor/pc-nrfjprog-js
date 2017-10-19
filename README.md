# pc-nrfjprog-js

Node.js library that exposes the functionality of the [nRF5x Command-Line-Tools](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.tools/dita/tools/nrf5x_command_line_tools/nrf5x_command_line_tools_lpage.html?cp=5_1), used for development, programming, and debugging of Nordic Semiconductor's nRF5x SoCs (System on Chip).

## Installing

The following tools required for building the project:

* Node.js (>=4)
* npm (>=3.7.0)
* CMake (>=2.8.12)
* A C/C++ toolchain

To install dependencies and build the project:

    npm install

### nRF5x-Command-Line-Tools

Before using the library, nRF5x-Command-Line-Tools must to be installed. The project includes nRF5x-Command-Line-Tools installers/archives for all supported platforms in the `nrfjprog` directory.

#### Windows

Run the nRF-Command-Line-Tools installer (exe) from the `nrfjprog` directory. This will install the required nrfjprog libraries and SEGGER J-Link.

#### Linux/macOS

Download and install [SEGGER J-Link](https://www.segger.com/downloads/jlink/).

Extract the nRF-Command-Line-Tools tar file from the `nrfjprog` directory, f.ex.:

    sudo tar -xf nRF5x-Command-Line-Tools_<version>_<platform>.tar -C /opt

This will create a directory `/opt/nrfjprog` containing the nrfjprog libraries. These libraries must be available for the process that uses pc-nrfjprog-js, f.ex. by adding the nrfjprog directory to `LD_LIBRARY_PATH`:

    export LD_LIBRARY_PATH=/opt/nrfjprog

## API documentation

http://nordicsemiconductor.github.io/pc-nrfjprog-js/

## Example

```
let nrfjprogjs = require('pc-nrfjprog-js');

nrfjprogjs.getConnectedDevices(function(err, devices) {
    console.log('There are ' + devices.length + ' nRF devices connected.');
});
```

## Tests

The project has integration tests that runs against a devkit/dongle. Note that these tests will erase the contents on the connected devkit/dongle. To run the tests:

    npm test
