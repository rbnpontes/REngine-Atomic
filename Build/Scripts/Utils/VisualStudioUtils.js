const path = require('path');
const fs = require('fs');
const { spawnAsync, execAsync } = require('./ProcessUtils');

const generators_table = {
    17: 'Visual Studio 17 2022',
    16: 'Visual Studio 16 2019',
    15: 'Visual Studio 15 2017 Win64'
};
const supported_versions = Object.keys(generators_table).map(x => x.toString());

/**
 * Return current Visual Studio Generator.
 * If visual studio isn't installed. then this method will fail.
 * @returns {Promise<string>}
 */
async function visualStudioGetCmakeGenerator() {
    const versions = visualStudioSupportedVersions();
    const curr_installed_version = await visualStudioExecVsWhere(['-latest', '-property', 'installationVersion'])

    if (!curr_installed_version)
        throw new Error('No Visual Studio Installation available.');

    const target_version = versions.find(x => curr_installed_version.startsWith(x));
    if (!target_version)
        throw new Error('This Visual Studio is not compatible. Version=' + target_version);

    const generator = generators_table[target_version];
    if(!generator)
        throw new Error(`Not found suitable generator for visual studio (${target_version}) version`);
    return generator;
}

function visualStudioSupportedVersions() {
    return supported_versions;
}
async function visualStudioExecVsWhere(args) {
    const vs_where_path = path.resolve(__dirname, '../../Windows/vswhere.exe');
    return await spawnAsync(vs_where_path, args).then(x => x.trim());
}

/**
 * Search for Visual Studio Tools directory from Installation Path
 * @param {string} installation_path 
 * @returns {string} return tools path. if is not found, null will return
 */
function __findToolsDirectory(installation_path) {
     // find VsMSBuildCmd, this is required for Engine Tool
     const build_cmd = fs.readdirSync(installation_path, { recursive: true}).find(x => x.endsWith('VsMSBuildCmd.bat') || x.endsWith('VsDevCmd.bat'));
     if(!build_cmd)
       return null;
     return path.basename(path.join(installation_path, build_cmd));
}
/**
 * Search at default path, that it's <VS_INSTALL_PATH>/Common7/Tools
 * @param {*} installation_path 
 */
function __findByDefaultLocation(installation_path) {
    const target_path = path.resolve(installation_path, 'Common7/Tools');
    if(!fs.existsSync(target_path))
        return null;
    return fs.readdirSync(target_path)
        .find(x =>  x.endsWith('VsMSBuildCmd.bat') || x.endsWith('VsDevCmd.bat')) 
        ? target_path : null;
}

const memo_tools_root_symbol = Symbol('tools_root');
/**
 * Get Visual Studio Tools Directory
 * @returns {Promise<string>}
 */
async function visualStudioGetToolsRoot() {
    // Getting vs tools path can be expensive, for this reason, results will be
    // memoized.
    if(visualStudioGetToolsRoot[memo_tools_root_symbol])
        return visualStudioGetToolsRoot[memo_tools_root_symbol];

    const installation_path = await visualStudioExecVsWhere([
        '-latest',
        '-property', 'installationPath'
    ]);

    if(!installation_path)
        throw new Error('Not found Visual studio installation path.');

    const tools_dir = __findByDefaultLocation(installation_path) ?? __findToolsDirectory(installation_path);
    if(!tools_dir)
        throw new Error('Not found Visual Studio Tools path.');

    visualStudioGetToolsRoot[memo_tools_root_symbol] = tools_dir;
    return tools_dir;
}

async function visualStudioDefineVsToolsEnv() {
    const tools_root = await visualStudioGetToolsRoot();
    await execAsync('setx', ['VS_TOOLS', tools_root], { noLogs: true });
    console.log('- Set VS_TOOLS: '+tools_root);
    process.env.TOOLS_ROOT = tools_root;
}

module.exports = {
    visualStudioGetCmakeGenerator,
    visualStudioSupportedVersions,
    visualStudioExecVsWhere,
    visualStudioGetToolsRoot,
    visualStudioDefineVsToolsEnv,
};