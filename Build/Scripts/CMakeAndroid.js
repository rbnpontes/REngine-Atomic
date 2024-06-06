const { Task } = require('jake');
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
function buildResourcePackages(package_path) {
    if(!fs.existsSync(package_path))
        fs.mkdirSync(package_path, { recursive: true });

    {
        // Clear directory files
        const tmp_files = fs.readdirSync(package_path);
        if(tmp_files.length != 0)
            console.log(`- Resource Directory isn\'t empty. Clearing (${tmp_files.length}) resources.`);
        tmp_files.forEach(x => fs.unlinkSync(path.join(package_path, x)));
    }

    const { PackageTool } = require('./PackageTool');
    const pkgs = [
        path.join(g_engine_root, 'Resources/CoreData'),
        path.join(g_engine_root, 'Resources/PlayerData'),
        path.join(g_engine_root, 'Submodules/EngineExamples/FeatureExamples/CPlusPlus/Data')
    ]
        .filter(x => fs.existsSync(x))
        .map(x => new PackageTool(x, path.join(package_path, path.basename(x))+'.pak', false));

    pkgs.forEach(pkg => {
        const pkg_name = path.basename(pkg.directory) + '.pak';
        console.log(`- Building package ${pkg_name}`);
        pkg.collect();
        pkg.calculateOffsets();
        pkg.calculateChecksum();
        const output_pkg = pkg.build();
        console.log(`- Validating ${pkg_name}`);
        pkg.validate(output_pkg);
        console.log('- Package was validate with success');
    });
}

namespace('android', ()=> {
    const project_dir = path.resolve(g_engine_root, '../REngine-Android');
    const lib_dir = path.resolve(project_dir, 'lib');

    async function genTask() {
        if(!fs.existsSync(project_dir))
            fs.mkdirSync(project_dir);
        
        for(let i = 0; i < g_supported_abis.length; ++i) {
            const abi = g_supported_abis[i];
            console.log(`- Generating Android ${abi} Project`);
            await genProject(abi, path.join(project_dir, abi));
            console.log('- Finished. Project generated with success 🎉');
        }
    }
    async function buildTask() {
        for(let i = 0; i < g_supported_abis.length; ++i) {
            const project_gen_path = path.join(project_dir, g_supported_abis[i]);
            console.log(`- Building Android ${g_supported_abis[i]} Project`);
            await buildProject(g_supported_abis[i], project_gen_path);
            console.log('- Finished. Project built with success 🎉');
        }
    }
    async function buildResourcesTask() {
        console.log('- Build resources');
        buildResourcePackages(path.join(project_dir, 'resources'));
        console.log('- Finished. Resources has been build with success 🎉');
    }
    async function clearLibsTask() {
        console.log('- Clearing built libraries');
        g_supported_abis.forEach(abi => {
            console.log(`- Clearing libraries ${abi}`);
            const target_lib_path = path.join(lib_dir, abi);
            if(!fs.existsSync(target_lib_path))
                return;
    
            const files = fs.readdirSync(target_lib_path);
            files.forEach(file => {
                file = path.join(target_lib_path, file);
                fs.unlinkSync(file);
            });
            console.log(`- Finished. Total of (${files.length}) ${abi} files cleared.`);
        });
    }
    async function copyLibsTask() {
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
    }
    async function genBoilerplateTask(projectNamespace, appName) {
        if(!projectNamespace)
            projectNamespace = process.env.project_namespace;
        if(!appName)
            appName = process.env.app_name;

        const project_android_out_dir = path.resolve(project_dir, appName);
        const android_template_dir = path.resolve(g_engine_root, 'Build/Android');
        const text_extensions = [
            '.kt',
            '.kts',
            '.xml',
            '.java',
            '.gitignore',
        ];

        const directories = {};
        const dirs_2_create = [];
        const dirs_2_process = [];
        const files_2_process = [];
        const files_2_create = [];

        const mustache_variables = {
            app_name : appName,
            project_namespace: projectNamespace
        };
        const hasMustache = (input)=> /\{{2}[A-Za-z_]+\}{2}/g.test(input);
        const resolveMustache = (input)=> {
            Object.entries(mustache_variables).forEach(x => {
                const [key, value] = x;
                const expected_keys = ['{{'+key+'}}', '{{ '+ key + ' }}'];
                const containsKey = ()=> {
                    return expected_keys.find(expected_key => input.indexOf(expected_key) != -1);
                };

                // Replace all mustache patterns until no patterns exists
                while(containsKey())
                    expected_keys.forEach(expected_key => input = input.replace(expected_key, value));
            });
            return input;
        }; 
        fs.readdirSync(android_template_dir, { recursive : true })
            .map(x => path.join(android_template_dir, x))
            .filter(x => x.endsWith('.template'))
            .forEach(x => {
                const stat = fs.statSync(x);
                directories[path.dirname(x)] = 1;
                if(stat.isDirectory()) {
                    (hasMustache(x) ? dirs_2_process : dirs_2_create).push(x);
                    return;
                }

                const file_path = x.replace('.template', '');
                const file_ext = path.extname(file_path);

                if(text_extensions.includes(file_ext))
                    files_2_process.push(x);
                else
                    files_2_create.push(x);
            });
        Object.keys(directories).forEach(dir => {
            (hasMustache(dir) ? dirs_2_process : dirs_2_create).push(dir);
        });

        // Process directories with mustache pattern
        dirs_2_process.forEach(dir => {
            dir = resolveMustache(dir).replace(/\./g,  path.sep);
            dirs_2_create.push(dir);
        });

        // Create directories on output project dir
        dirs_2_create.forEach(dir => {
            const output_dir = path.join(project_android_out_dir, dir.replace(android_template_dir, ''));

            if(fs.existsSync(output_dir))
                return;
            fs.mkdirSync(output_dir, { recursive: true });
        });

        const getFinalPath = (file_path)=> {
            let file_output_path = path.join(project_android_out_dir, file_path.replace(android_template_dir, '')).replace('.template', '');
            file_output_path = resolveMustache(file_output_path);

             // fix replaced mustache directories
             const dir = path.dirname(file_output_path);
             file_output_path = file_output_path.replace(dir, dir.replace(/\./g, path.sep));

             return file_output_path;
        };
        // Copy files that can't be processed.
        files_2_create.forEach(file_path => {
            const file_output_path = getFinalPath(file_path);

            // then, copy file into destination
            if(fs.existsSync(file_output_path))
                fs.unlinkSync(file_output_path);

            const buffer = fs.readFileSync(file_path);
            fs.writeFileSync(file_output_path, buffer);
        });

        // Process files and copy to destination
        files_2_process.forEach(file_path => {
            const file_output_path = getFinalPath(file_path);

            if(fs.existsSync(file_output_path))
                fs.unlinkSync(file_output_path);

            const data = resolveMustache(fs.readFileSync(file_path).toString());
            fs.writeFileSync(file_output_path, data);
        });
    }

    task('gen', genTask);
    task('build', buildTask);
    task('buildres', buildResourcesTask);
    task('clrlibs', clearLibsTask);
    task('cpylibs', copyLibsTask);
    task('genproj', genBoilerplateTask);
    task('full', async ()=> {
        const tasks = [
            genTask,
            buildTask,
            buildResourcesTask,
            clearLibsTask,
            copyLibsTask
        ];
        
        for(let i = 0; i < tasks.length; ++i) {
            const task = tasks[i];
            await task();
        }
        console.log('- Finished 🎉');
    });
});