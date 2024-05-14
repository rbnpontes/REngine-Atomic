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
    if(!scanDir.memo_files)
        scanDir.memo_files = {};
    
    const files = scanDir.memo_files[p] ?? fs.readdirSync(p).map(x => path.join(p, x));
    scanDir.memo_files[p] = files;

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

/**
 * @param {string[]} args 
 * @param {string?} workingDir
 * @returns {Promise<void>}
 */
function executeCmake(args, workingDir) {
    const proc = nodeSpawn(getCmakeCommand(), args, { cwd: workingDir });
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

function genProject(abi, output) {
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
        '-DATOMIC_DOTNET=OFF',
        '-DRENGINE_SHARED=ON',
        '-DBUILD_SHARED_LIBS=ON'
    ];

    return executeCmake(args);
}

function buildProject(abi, path) {
    if(!fs.existsSync(path)) {
        console.log(`- Android project ${abi} does not exists. Skipping!`);
        return Promise.resolve(void(0));
    }

    // limit cores to 2 ~ 8
    const available_cores = Math.min(Math.max(os.cpus().length, 2), 8);
    console.log(`- Build with ${available_cores} cores`);
    return executeCmake([
        '--build', '.', 
        '-j', available_cores.toString()
    ], path);
}

function copyBuildLibs(project_src, dest) {
    const files = scanDir(project_src);
    const libs = files.filter(x => {
        x = path.basename(x);
        return x.startsWith('lib') && x.endsWith('.so') /*|| x.endsWith('.a')*/;
    });
    libs.forEach(lib => {
        const lib_file_name = path.basename(lib);
        const dest_path = path.join(dest, lib_file_name);
        if(fs.existsSync(dest_path))
            fs.unlinkSync(dest_path);

        console.log(`- Copying ${path.basename(dest_path)}`);
        const data = fs.readFileSync(lib)
        fs.writeFileSync(dest_path, data);
    });
}

namespace('android', ()=> {
    const project_dir = path.resolve(g_engine_root, '../REngine-Android');
    const lib_dir = path.resolve(project_dir, 'lib');

    task('gen', async ()=> {
        if(!fs.existsSync(project_dir))
            fs.mkdirSync(project_dir);
        
        for(let i = 0; i < g_supported_abis.length; ++i) {
            const abi = g_supported_abis[i];
            console.log(`- Generating Android ${abi} Project`);
            await genProject(abi, path.join(project_dir, abi));
            console.log('- Finished. Project generated with success 🎉');
        }
    });
    
    task('build', async ()=> {
        for(let i = 0; i < g_supported_abis.length; ++i) {
            const project_gen_path = path.join(project_dir, g_supported_abis[i]);
            console.log(`- Building Android ${g_supported_abis[i]} Project`);
            await buildProject(g_supported_abis[i], project_gen_path);
            console.log('- Finished. Project built with success 🎉');
        }
    });

    task('cpylibs', ()=> {
        // create lib artifact dir
        if(!fs.existsSync(lib_dir))
            fs.mkdirSync(lib_dir);
        
        console.log('- Copying build libraries');
        g_supported_abis.forEach(abi => {
            console.log(`- Copying build libraries ${abi}`);
            const target_lib_path = path.join(lib_dir, abi);
            const project_gen_path = path.join(project_dir, abi);
            if(!fs.existsSync(target_lib_path))
                fs.mkdirSync(target_lib_path);
            copyBuildLibs(project_gen_path, target_lib_path);
            console.log(`- Finished. ${abi} libraries has been copied with success 🎉`);
        });
    });
});