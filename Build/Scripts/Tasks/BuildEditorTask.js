const path      = require('path');
const config    = require('../BuildConfig');
const fs        = require('fs-extra');
const host      = require('../Host');
const os        = require('os');
const archiver  = require('archiver');
const constants = require('../Constants');

const { 
    visualStudioGetCmakeGenerator, 
    visualStudioDefineVsToolsEnv 
} = require('../Utils/VisualStudioUtils');
const { 
    cmakeGenerate 
} = require('../Utils/CmakeUtils');
const { 
    engineGetRoot, 
    engineGetArtifactsRoot
} = require('../Utils/EngineUtils');
const { 
    execAsync 
} = require('../Utils/ProcessUtils');
const { getUnsupportedEnvironmentError } = require('../Exceptions');


const engine_root = engineGetRoot();
const artifacts_root = engineGetArtifactsRoot();
const editor_app_folder = config.editorAppFolder;
const resources_dest = (() => {
    if (os.platform() == 'win32')
        return editor_app_folder;
    else if (os.platform() == 'darwin')
        return path.resolve(editor_app_folder, 'Contents');
    return path.join(artifacts_root, constants.engine_editor_name);
})();


function editorCleanArtifacts() {
    console.log('- Clearing artifacts directory');
    fs.readdirSync(artifacts_root)
        .filter(x => !x.endsWith('.gitkeep'))
        .forEach(x => {
            x = path.join(artifacts_root, x);
            fs.removeSync(x);
        });
    console.log('- Finished. artifacts directory is clean!');
}

function editorGetBuildDirectory() {
    switch (os.platform()) {
        case 'win32':
            return path.join(artifacts_root, 'Build/Windows');
        case 'linux':
            return path.join(artifacts_root, 'Build/Linux');
        case 'darwin':
            return path.join(artifacts_root, 'Build/MacOS');
        default:
            throw new Error('Not supported this environment');
    }
}
async function editorGetCmakeGenerator() {
    switch (os.platform()) {
        case 'win32':
            return await visualStudioGetCmakeGenerator();
        case 'linux':
            return 'Unix Makefiles';
        case 'darwin':
            return 'Xcode';
        default:
            throw new Error('Not found suitable cmake generator for this environment.');
    }
}
async function editorBuildFirstPhase() {
    console.log(`- Starting ${constants.engine_name} editor first phase`);

    const build_dir = editorGetBuildDirectory();
    const build_func_tbl = {
        win32: async () => {
            // Execute Build Windows
            const vs_tools_path = await visualStudioDefineVsToolsEnv();
            const compile_script = path.resolve(__dirname, '../Windows/CompileAtomicEditorPhase1.bat');
            return await execAsync(compile_script, [config.config, vs_tools_path], { cwd: build_dir });
        },
        linux: async () => {
            // Execute Build Linux
            return await execAsync(
                'make',
                [constants.engine_native_lib, '-j8'],
                { cwd: build_dir }
            );
        },
        darwin: async () => {
            // Execute Build MacOS
            return await execAsync(
                'xcodebuild',
                [
                    '-target', 'GenerateScriptBindings',
                    '-target', constants.engine_native_lib,
                    '-configuration', config.config,
                    '-parallelizeTargets',
                    '-jobs', '4'
                ],
                { cwd : build_dir }
            );
        }
    }

    const build_func = build_func_tbl[os.platform()];
    if (!build_func)
        return;

    const exit_code = await build_func();
    if(exit_code != 0)
        throw new Error('Build has failed with exit code '+exit_code);
    console.log('- First phase has been completed with success!!!');
}
async function editorBuildSecondPhase() {
    console.log(`- Starting ${constants.engine_name} editor second phase`);
    const build_dir = editorGetBuildDirectory();
    const build_func_tbl = {
        win32: async () => {
            const vs_tools_path = await visualStudioDefineVsToolsEnv();
            const compile_script = path.resolve(__dirname, '../Windows/CompileAtomicEditorPhase2.bat');
            return await execAsync(compile_script, [config.config, vs_tools_path], { cwd: build_dir });
        },
        linux: async () => {
            return await execAsync(
                'make',
                [constants.engine_editor_name, constants.engine_player_name, '-j8'],
                { cwd: build_dir }
            );
        },
        darwin: async () => {
            return await execAsync(
                'xcodebuild',
                [
                    '-target', constants.engine_editor_name,
                    '-target', constants.engine_player_name,
                    '-configuration', config.config,
                    '-parallelizeTargets',
                    '-jobs', '4'
                ],
                { cwd: build_dir }
            );
        }
    };

    const build_func = build_func_tbl[os.platform()];
    if (!build_func)
        return;

    const exit_code = await build_func();
    if(exit_code != 0)
        throw new Error('Build has failed with exit code '+exit_code);
    console.log('- Second phase has been completed with success!!!');
}
async function editorGenerate() {
    console.log(`- Generating ${constants.engine_name} Editor`);
    // Remove editor directory always
    if (fs.existsSync(editor_app_folder))
        fs.rmSync(editor_app_folder, { recursive: true, force: true });
    fs.mkdirSync(editor_app_folder);

    const build_dir = editorGetBuildDirectory();
    const dirs_2_create = [
        path.resolve(artifacts_root, constants.engine_net_name),
        build_dir,
        host.getGenScriptRootDir()
    ];
    const dirs_2_remove = [
        path.resolve(artifacts_root, 'Build/Android'),
        path.resolve(artifacts_root, 'Build/IOS')
    ];

    // if noclean argument is present. skip cleaning operation
    if (!config.noclean) {
        console.log('- Cleaning files');
        dirs_2_create.forEach(x => {
            if(!fs.existsSync(x))
                fs.mkdirSync(x, { recursive: true });
        });
        dirs_2_remove.forEach(x => {
            if(fs.existsSync(x))
                fs.rmSync(x, { recursive: true, force: true });
        });
    }

    process.chdir(build_dir);

    console.log('- Generating Editor Project');

    // execute cmake to generate project
    const generator = await editorGetCmakeGenerator();
    await cmakeGenerate({
        generator,
        source: engine_root,
        build: build_dir,
        additionalOptions: [
            `-D${constants.cmake_option_prefix}_PROFILING=OFF`
        ]
    });

    console.log('- Editor Project was generated with success.');
}
function editorCopyNETBinaries() {
    // TODO: refactor this condition
    if (!config['with-atomic-net'])
        return;

    const engine_lib_path = path.resolve(engine_root, 'Artifacts', constants.engine_net_name, config.config);
    const engine_lib_out_path = path.resolve(resources_dest, 'Resources/ToolData', constants.engine_net_name, config.config);
    const engine_proj_path = path.resolve(engine_root, constants.engine_net_name, constants.engine_project_json);
    const engine_proj_out_path = path.resolve(
        resources_dest,
        'Resources/ToolData',
        constants.engine_net_name,
        'Build/Projects',
        constants.engine_project_json
    );

    [
        [engine_lib_path, engine_lib_out_path],
        [engine_proj_path, engine_proj_out_path]
    ].forEach(x => {
        const [src, dst] = x;
        fs.copySync(src, dst);
    });
}
function editorCopyCEFBinaries() {
    console.log('- Copying CEF binaries')
    const bin_path = path.join(engine_root, 'Submodules/CEF/Linux/Release');
    const res_path = path.join(engine_root, 'Submodules/CEF/Linux/Resources');

    const bin_files = fs.readdirSync(
        bin_path
    ).map(x => [path.join(bin_path, x), x]);
    const res_files = fs.readdirSync(
        res_path
    ).map(x => [path.join(res_path, x), x]);


    [
        ...bin_files,
        ...res_files
    ].forEach(x => {
        const [filepath, filename] = x;
        fs.copySync(
            filepath,
            path.join(resources_dest, filename)
        );
    });
}
async function editorCopyBinaries() {
    console.log(`- Copying ${constants.engine_name} Editor Binaries`);

    const build_dir = await editorGetBuildDirectory();

    //const editor_build_dir = path.resolve(build_dir, 'Source', constants.engine_editor_name, config.config);
    const editor_build_dir = (()=> {
        switch(os.platform()) {
            case 'win32':
            case 'linux':
                return path.resolve(build_dir, 'Source', constants.engine_editor_name, config.config);
            case 'darwin':
                return path.resolve(build_dir, 'Source', constants.engine_editor_name, constants.engine_editor_name);
            default:
                throw getUnsupportedEnvironmentError();
        }
    })();
    const editor_output_dir = (()=> {
        switch(os.platform()){
            case 'win32':
            case 'linux':
                return path.resolve(artifacts_root, constants.engine_editor_name);
            case 'darwin':
                return path.resolve(artifacts_root, constants.engine_editor_name, constants.engine_editor_name);
            default:
                throw getUnsupportedEnvironmentError();
        }
    })();
    const core_data_dir = path.resolve(engine_root, 'Resources/CoreData');
    const core_data_output_dir = path.resolve(resources_dest, 'Resources/CoreData');
    const player_data_dir = path.resolve(engine_root, 'Resources/PlayerData');
    const player_data_output_dir = path.resolve(resources_dest, 'Resources/PlayerData');
    const engine_editor_data_dir = path.resolve(engine_root, 'Data', constants.engine_editor_name);
    const tool_data_output_dir = path.resolve(resources_dest, 'Resources/ToolData');
    const editor_data_dir = path.resolve(engine_root, 'Resources/EditorData');
    const editor_data_output_dir = path.resolve(resources_dest, 'Resources/EditorData');
    const editor_scripts_dir = path.resolve(engine_root, `Artifacts/Build/Resources/EditorData/${constants.engine_editor_name}/EditorScripts`);
    const editor_scripts_output_dir = path.resolve(resources_dest, 'Resources/EditorData/AtomicEditor/EditorScripts');
    const app_file_dir = (() => {
        if (os.platform() == 'win32') {
            return path.resolve(
                build_dir,
                'Source',
                constants.engine_player_name,
                'Application',
                config.config,
                constants.engine_player_name + '.exe'
            );
        }
        else if (os.platform() == 'darwin') {
            return path.resolve(
                build_dir,
                'Source',
                constants.engine_player_name,
                'Application',
                config.config,
                constants.engine_player_name + '.app',
                'Contents/MacOS',
                constants.engine_player_name
            );
        }

        return path.resolve(
            build_dir,
            'Source',
            constants.engine_player_name,
            'Application',
            constants.engine_player_name
        );
    })();
    const app_file_output_dir = (() => {
        if (os.platform() == 'win32') {
            return path.resolve(
                resources_dest,
                'Resources/ToolData/Deployment/Windows/x64/',
                constants.engine_player_name + '.exe'
            );
        }
        else if (os.platform() == 'darwin') {
            return path.resolve(
                resources_dest,
                'Resources/ToolData/Deployment/MacOS',
                constants.engine_player_name + '.app',
                'Contents/MacOS',
                constants.engine_player_name
            );
        }

        return path.resolve(
            resources_dest,
            'Resources/ToolData/Deployment/Linux',
            constants.engine_player_name
        );
    })();

    [
        [editor_build_dir, editor_output_dir],
        [core_data_dir, core_data_output_dir],
        [player_data_dir, player_data_output_dir],
        [engine_editor_data_dir, tool_data_output_dir],
        [editor_data_dir, editor_data_output_dir],
        [editor_scripts_dir, editor_scripts_output_dir],
        [app_file_dir, app_file_output_dir]
    ].forEach(x => {
        const [src, dst] = x;
        console.log(`- Copying. From: ${src} - To: ${dst}`);
        fs.copySync(src, dst);
    });

    // if (os.platform() == 'linux')
    //     editorCopyCEFBinaries();
    editorCopyNETBinaries();

    console.log('- Editor binaries has been copied with success!!!');
}
async function editorPackage() {
    console.log('- Packing Editor to Shipping');
    const pkg_output_path = path.join(editor_app_folder, constants.engine_name+'.zip');

    if(fs.existsSync(pkg_output_path) && !config.noclean)
        fs.unlinkSync(pkg_output_path);
    
    const archive_task = new Promise((resolve, reject)=> {
        const final_name = (()=> {
            switch(os.platform()){
                case 'win32':
                    return constants.engine_name + '-Windows.zip';
                case 'linux':
                    return constants.engine_name + '-Linux.zip';
                case 'darwin':
                    return constants.engine_name + '-MacOS.zip';
                default:
                    throw new Error('Not supported on this platform');
            }
        })();
        const write_stream  = fs.createWriteStream(path.join(artifacts_root, final_name));
        const archive = archiver('zip', {
            zlib: { level: 9 },
        });
        archive.on('warning', e=> {
            if(e.code == 'ENOENT')
                console.log(e);
            else
                reject(e);
        });
        archive.on('error', e => reject(e));
        archive.on('end', ()=> resolve());
        archive.pipe(write_stream);

        archive.directory(editor_app_folder, false);
        archive.finalize();
    });
    await archive_task;
    console.log('- Editor was packed with success at '+pkg_output_path);
}

async function editorBuild() {
    console.log(`- Building ${constants.engine_name} Editor`);

    if(!config.noclean)
        editorCleanArtifacts();

    try {
        await editorGenerate();
    } catch {
        console.log('- Project Generation has failed. generally this occurs because binding files is not generated correctly');
        console.log('- Build script will run same step again');
        await editorGenerate();
    }
    await editorBuildFirstPhase();
    await editorBuildSecondPhase();
    await editorCopyBinaries();
    if(config.package)
        editorPackage();
    console.log(`- Finished. ${constants.engine_name} Editor built to ${editor_app_folder}`);
}

module.exports = {
    editorCleanArtifacts,
    editorBuild,
    editorGenerate,
    editorBuildFirstPhase,
    editorBuildSecondPhase,
    editorCopyBinaries,
    editorPackage
};