console.log('.:: Running Engine Sandbox Javascript Tests ::.');

console.log('Running Log Tests');
console.log('test log info');
console.debug('test debug log');
console.warn('test warning log');
console.error('test error log');

const reqFn = require;
if(typeof reqFn == 'undefined')
    throw new Error('Require function is not available.');
if(typeof reqFn.cache == 'undefined')
    throw new Error('Require function doesn\'t contains cache object.');

console.log('Loading test module');
const moduleExport = require('./test-module.js');
const moduleResult = moduleExport.result;
console.log('Module Export: '+moduleResult);

const moduleExport2 = require('./test-module.js');
if(moduleExport2 != moduleExport)
    throw new Error('Invalid require implementation. Require must return same reference of previous loaded module!');