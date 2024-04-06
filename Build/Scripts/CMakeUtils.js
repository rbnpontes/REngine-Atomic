const config = require('./BuildConfig');

/**
 * get CMake flags for generator.
 * @param {boolean} isDevBuild 
 * @returns 
 */
function getCMakeFlags(isDevBuild) {
    const flags = [];
    const appendFlag = (flagName, flagProp) => flags.push(`-D${flagName}=${Boolean(config[flagProp]) ? 'ON' : 'OFF'}`);
    
    // local cmake builds are always dev builds 
    flags.push(`-DATOMIC_DEV_BUILD=${isDevBuild ? 1 : 0}`);

    [
        ['ATOMIC_OPENGL', 'opengl'],
        ['ATOMIC_D3D11', 'd3d9'],
        ['RENGINE_DILIGENT', 'diligent']
    ].forEach(x => {
        const [flagName, flagProp] = x;
        appendFlag(flagName, flagProp);
    });

    const res = flags.join(' ');
    return res;
}

module.exports = {
    getCMakeFlags,
};