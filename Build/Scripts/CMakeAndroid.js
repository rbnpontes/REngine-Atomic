const fs = require('fs');
const path = require('path');
const os = require('os');
const nodeSpawn = require('child_process').spawn;

const config = require('./BuildConfig');

const g_engine_root = config.atomicRoot;
const g_supported_abis = [
    'armeabi-v7a', 
    'arm64-v8a'
];

/**
 * @returns {string}
 */
function requireNdkEnv() {
    if(!process.env.NDK_ROOT)
        throw new Error('NDK_ROOT environment variable is required.');
    if(!fs.existsSync(process.env.NDK_ROOT))
        throw new Error('NDK_ROOT environment variable path is not valid. This is not a valid directory');    
    return process.env.NDK_ROOT;
}

/**
 * 
 * @param {string} p
 * @returns {Array<string>} 
 */
function scanDir(p) {
    // cache files for a better performance
    if(!this.memo_files)
        this.memo_files = {};
    
    const files = this.memo_files[p] ?? fs.readdirSync(p).map(x => path.join(p, x));
    this.memo_files[p] = files;

    let results = files;
    files.forEach(file => {
        if(fs.statSync(file).isDirectory())
            results = [...results, ...scanDir(file)];
    });
    
    return results;
}

/**
 * @returns {string}
 */
function getToolchainPath() {
    if(getToolchainPath.result)
        return getToolchainPath.result;
    const toolchain = scanDir(requireNdkEnv()).find(x => x.endsWith('android.toolchain.cmake'));
    if(!toolchain)
        throw new Error('Not found NDK Android CMake Toolchain. Are you sure that NDK path is correct ?');
    return getToolchainPath.result = toolchain;
}

/**
 * @returns {string}
 */
function getMakePath() {
    if(getMakePath.result)
        return getMakePath.result;
    const make_file = os.platform() == 'win32' ? 'make.exe' : 'make';
    const make = scanDir(requireNdkEnv()).find(x => x.endsWith(make_file));
    if(!make)
        throw new Error('Not found NDK Android Make. Are you sure that NDK path is correct ?');
    return getMakePath.result = make;
}

/**
 * @returns {string}
 */
function getCmakeCommand() {
    return os.platform() == 'win32' ? 'cmake.exe' : 'cmake';
}

function executeCmake(abi, output) {
    if(!fs.existsSync(output))
        fs.mkdirSync(output, { recursive: true });
    
    const args = [
        '-S', g_engine_root,
        '-B', output, 
        '-G', 'Unix Makefiles',
        `-DCMAKE_TOOLCHAIN_FILE=${getToolchainPath()}`,
        `-DCMAKE_MAKE_PROGRAM=${getMakePath()}`,
        `-DANDROID_ABI=${abi}`,
        '-DANDROID_PLATFORM=android-26',
        '-DANDROID_STL=c++_static',
        '-DATOMIC_EDITOR=OFF', // Editor doesn't works on Android
        '-DATOMIC_PROFILING=OFF',
        // TODO: fix this issues
        '-DATOMIC_JAVASCRIPT=OFF',
        '-DATOMIC_DOTNET=OFF'
    ];

    const proc = nodeSpawn(getCmakeCommand(), args);
    proc.stdout.on('data', data => {
        console.log(data.toString());
    });
    proc.stderr.on('data', (data)=> {
        console.error(data.toString());
    });

    return new Promise((resolve, reject)=> {
        proc.on('exit', code => {
            if(code != 0)
                reject(new Error('CMake process exited with code '+code));
            resolve();
        });
    });
}

namespace('build', function() {
    task('genandroid', {
        async: true
    }, async function() {
        const output_dir = path.resolve(g_engine_root, '../REngine-Android');
        if(!fs.existsSync(output_dir))
            fs.mkdirSync(output_dir);

        for(let i = 0; i < g_supported_abis.length; ++i) {
            const abi = g_supported_abis[i];
            console.log(`- Generating Android ${abi} Project`);
            await executeCmake(abi, path.join(output_dir, abi));
            console.log('- Finish. Project generated with success 🎉');
        }
    });
});