'use strict';

const nrfjprog = require('../index.js');
let probe = new nrfjprog.DebugProbe();
probe.connect(() => {
    console.log('I was called back!');

    console.log(nrfjprog.UP_DIRECTION);
    console.log(nrfjprog.DOWN_DIRECTION);
});
