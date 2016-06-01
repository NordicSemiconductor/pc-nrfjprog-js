'use strict';

const nrfjprog = require('../index.js');
let probe = new nrfjprog.DebugProbe();
probe.getSerialNumbers((err, serialNumbers) => {
    console.log(JSON.stringify(serialNumbers));
    console.log('I was called back!');

    console.log(nrfjprog.UP_DIRECTION);
    console.log(nrfjprog.DOWN_DIRECTION);

    probe.program(682447605, 1, 'connectivity_115k2_with_s132_2.0.1.hex', err => {
        console.log(err);
        console.log('Done programming');
    });
});
