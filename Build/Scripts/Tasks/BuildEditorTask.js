const path = require('path');
const config = require('../BuildConfig');
const fs = require('fs-extra');
const constants = require('../Constants');
const os = require('os');
const { visualStudioGetCmakeGenerator, visualStudioDefineVsToolsEnv, visualStudioExecMsBuild } = require('../Utils/VisualStudioUtils');
const { cmakeGenerate } = require('../Utils/CmakeUtils');
const host = require('../Host');
const { engineGetRoot, engineGetArtifactsRoot } = require('../Utils/EngineUtils');
const { execAsync } = require('../Utils/ProcessUtils');

const engine_root = engineGetRoot();
const artifacts_root = engineGetArtifactsRoot();
const editor_app_folder = config.editorAppFolder;
const resources_dest = (()=> {
    if(os.platform() == 'win32')
        return editor_app_folder;
    else if(os.platform() == 'darwin')
        return path.resolve(editor_app_folder, 'Contents');
    return path.join(artifacts_root, constants.engine_editor_name);
})();


function editorGetBuildDirectory() {
    switch(os.platform()) {
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
    switch(os.platform()){
        case 'win32':
            return await visualStudioGetCmakeGenerator();
        case 'linux':
            return 'Unix Makefile';
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
        win32 : async ()=> {
            await visualStudioDefineVsToolsEnv();
            const compile_script = path.resolve(__dirname, '../Windows/CompileAtomicEditorPhase1.bat');
            await execAsync(compile_script, [config.config], { cwd: build_dir });
        },
        linux: async ()=> {
            throw new Error('Not implemented Linux Build');
        },
        darwin: async ()=> {
            throw new Error('Not implemented MacOS Build');
        }
    }

    const build_func = build_func_tbl[os.platform()];
    if(!build_func)
        return;

    await build_func();
    console.log('- First phase has been completed with success!!!');
}

async function editorBuildSecondPhase() {
    console.log(`- Starting ${constants.engine_name} editor second phase`);
    const build_dir = editorGetBuildDirectory();
    const build_func_tbl = {
        win32 : async ()=> {
            await visualStudioDefineVsToolsEnv();
            const compile_script = path.resolve(__dirname, '../Windows/CompileAtomicEditorPhase2.bat');
            await execAsync(compile_script, [config.config], { cwd: build_dir });
        },
        linux: async ()=> {
            throw new Error('Not implemented Linux Build');
        },
        darwin: async ()=> {
            throw new Error('Not implemented MacOS Build');
        }
    };

    const build_func = build_func_tbl[os.platform()];
    if(!build_func)
        return;

    await build_func();
    console.log('- Second phase has been completed with success!!!');
}
async function editorGenerate() {
    console.log(`- Generating ${constants.engine_name} Editor`);
    // Remove editor directory always
    if(fs.existsSync(editor_app_folder))
        fs.rmSync(editor_app_folder, { recursive: true, force: true });
    fs.mkdirSync(editor_app_folder);
    
    const build_dir = editorGetBuildDirectory();
    const dirs_2_create = [
        path.resolve(artifacts_root, constants.engine_net_name),
        build_dir,
        host.getGenScriptRootDir()
    ];
    const dirs_2_remove = [
        path.resolve(artifacts_root, 'Build/Android')
    ];
    
    // if noclean argument is present. skip cleaning operation
    if(!config.noclean) 
    {
        console.log('- Cleaning files');
        dirs_2_create.forEach(x => fs.mkdirSync(x, { recursive : true }));
        dirs_2_remove.forEach(x => fs.rmSync(x, { recursive: true, force: true }));
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
    if(!config['with-atomic-net'])
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
async function editorCopyBinaries() {
    console.log(`- Copying ${constants.engine_name} Editor Binaries`);

    const build_dir = await editorGetBuildDirectory();
    
    const editor_build_dir          = path.resolve(build_dir, 'Source', constants.engine_editor_name, config.config);
    const editor_output_dir         = path.resolve(artifacts_root, constants.engine_editor_name);
    const core_data_dir             = path.resolve(engine_root, 'Resources/CoreData');
    const core_data_output_dir      = path.resolve(resources_dest, 'Resources/CoreData');
    const player_data_dir           = path.resolve(engine_root, 'Resources/PlayerData');
    const player_data_output_dir    = path.resolve(resources_dest, 'Resources/PlayerData');
    const engine_editor_data_dir    = path.resolve(engine_root, 'Data', constants.engine_editor_name);
    const tool_data_output_dir      = path.resolve(resources_dest, 'Resources/ToolData');
    const editor_data_dir           = path.resolve(engine_root, 'Resources/EditorData');
    const editor_data_output_dir    = path.resolve(resources_dest, 'Resources/EditorData');
    const editor_scripts_dir        = path.resolve(engine_root, `Artifacts/Build/Resources/EditorData/${constants.engine_editor_name}/EditorScripts`);
    const editor_scripts_output_dir = path.resolve(resources_dest, 'Resources/EditorData/AtomicEditor/EditorScripts');
    const get_app_file_dir = ()=> {
        if(os.platform() == 'win32'){
            return path.resolve(
                build_dir, 
                'Source', 
                constants.engine_player_name,
                'Application', 
                config.config, 
                constants.engine_player_name+'.exe'
            );
        }
        else if(os.platform() == 'darwin') {
            return path.resolve(
                build_dir, 
                'Source',
                constants.engine_player_name,
                'Application', 
                config.config, 
                constants.engine_player_name+'.app', 
                'Contents/MacOS', 
                constants.engine_player_name
            );
        }

        return path.resolve(
            builder_dir, 
            'Source',
            constants.engine_player_name,
            'Application', 
            constants.engine_player_name
        );
    };
    const get_app_file_output_dir = ()=> {
        if(os.platform() == 'win32') {
            return path.resolve(
                resources_dest,
                'Resources/ToolData/Deployment/Windows/x64/',
                constants.engine_player_name + '.exe'
            );
        }
        else if(os.platform() == 'darwin') {
            return path.resolve(
                resources_dest,
                'Resources/ToolData/Deployment/MacOS',
                constants.engine_player_name+'.app',
                'Contents/MacOS',
                constants.engine_player_name
            );
        }

        return path.resolve(
            resources_dest,
            'Resources/ToolData/Deployment/Linux',
            constants.engine_player_name
        );
    };

    [
        [editor_build_dir, editor_output_dir],
        [core_data_dir, core_data_output_dir],
        [player_data_dir, player_data_output_dir],
        [engine_editor_data_dir, tool_data_output_dir],
        [editor_data_dir, editor_data_output_dir],
        [editor_scripts_dir, editor_scripts_output_dir],
        [get_app_file_dir(), get_app_file_output_dir()]
    ].forEach(x => {
        const [src, dst] = x;
        fs.copySync(src, dst);
    });

    editorCopyNETBinaries();

    console.log('- Editor binaries has been copied with success!!!');
}

async function editorBuild() {
    console.log(`- Building ${constants.engine_name} Editor`);

    await editorGenerate();
    await editorBuildFirstPhase();
    await editorBuildSecondPhase();
    await editorCopyBinaries();
    console.log(`- Finished. ${constants.engine_name} Editor built to ${editor_app_folder}`);
}

module.exports = {
    editorBuild,
    editorGenerate,
    editorBuildFirstPhase,
    editorBuildSecondPhase,
    editorCopyBinaries
};