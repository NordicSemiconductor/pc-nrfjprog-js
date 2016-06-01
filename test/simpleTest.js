'use strict';

const nrfjprog = require('../index.js');
let probe = new nrfjprog.DebugProbe();
probe.getSerialNumbers((err, serialNumbers) => {
    console.log(JSON.stringify(serialNumbers));
    console.log('I was called back!');

    console.log(nrfjprog.UP_DIRECTION);
    console.log(nrfjprog.DOWN_DIRECTION);
});
