const path = require('path');
const { argv } = require('process');
const { PackageTool } = require('./PackageTool');

function print_usage() {
    console.log([
        '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@',
        '@@@ REngine - PackageTool                                     @@@',
        '@@@ Usage: yarn pkg [directory] [package-output-path].pak     @@@',
        '@@@        [0=little endian|1=big endian]                     @@@',
        '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@'
    ].join('\n'));
}
function error(err_desc) {
    console.error(`Error: ${err_desc}`);
    print_usage();
    process.exit(1);
}

function getArgs() {
    const args = [...argv];
    const selfCmdIdx = args.findIndex(x => x.endsWith('PackageTool.js'));
    const result = args.splice(selfCmdIdx + 1, args.length);
    if(result.length < 3)
        error('Invalid arguments');
    return result;
}

const [directory, pkgPath, isBigEndian] = getArgs();
{
    if(!directory)
        error('Invalid arguments. Directory is empty');
    
    if(!pkgPath)
        error('Invalid arguments. Package output path is empty');
    
    if(!pkgPath.endsWith('.pak'))
        error('Invalid arguments. Package output path must ends with extension .pak');

    if(isBigEndian != '0' && isBigEndian != '1')
        error('Invalid arguments. Endianess must be specified');
}

const pkg = new PackageTool(
    path.resolve(directory), 
    pkgPath, 
    Boolean(parseInt(isBigEndian) | 0)
);

console.log('- Collecting files');
pkg.collect();
console.log('- Calculating offsets');
pkg.calculateOffsets();
console.log('- Calculating checksum');
pkg.calculateChecksum();
console.log('- Building Package');
const packageOutputPath = pkg.build();
console.log('- Validating Package');
try {
    pkg.validate(packageOutputPath);
} catch(e) {
    console.error('- Invalid Package. Exiting!');
    console.error(e.message);
    process.exit(0);
}
console.log('- 🎉 Finished');