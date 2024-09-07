console.log('Executing Module');
console.log('__dirname : ' + __dirname);
console.log('__filename: ' + __filename);

if(typeof module === 'undefined')
    throw new Error('"module" property is not defined.');
if(typeof module.exports === 'undefined')
    throw new Error('"module.exports" property is not defined.');

module.exports = { result : 'test-module-export' };