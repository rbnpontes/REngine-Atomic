const path = require('path');
const fs = require('fs-extra');
const os = require('os');
const { glob, globSync } = require('glob');
const TsLint = require('tslint');

const { engineGetRoot, engineGetArtifactsRoot } = require("../Utils/EngineUtils");
const { execAsync } = require('../Utils/ProcessUtils');
const { getUnsupportedEnvironmentError } = require('../Exceptions');

const constants = require('../Constants');
const engine_root = engineGetRoot();
const engine_tool = (()=> {
    const artifacts = engineGetArtifactsRoot();
    switch(os.platform()) {
        case 'win32':
            return path.resolve(artifacts, 'Build', constants.engine_tool_name, constants.engine_tool_name+'.exe');
        case 'darwin':
        case 'linux':
            return path.resolve(artifacts, 'Build', constants.engine_tool_name, constants.engine_tool_name);
    }
    getUnsupportedEnvironmentError();
})();
const script_dir = path.resolve(engine_root, 'Script');
const typescript_generated_types_path = path.resolve(engineGetArtifactsRoot(), 'Build/TypeScript');

/**
 * @typedef {Object} PackageModuleExcludeDesc
 * @property {string[]?} WEB
 * @property {string[]?} ANDROID
 * @property {string[]?} IOS
 * @property {string[]?} WINDOWS
 * @property {string[]?} MACOSX
 * @property {string[]?} LINUX
 */
/**
 * @typedef {Object} PackageDesc
 * @property {string} name
 * @property {string?} namespace
 * @property {string[]} modules
 * @property {PackageModuleExcludeDesc?} moduleExclude
 * @property {string[]?} dependencies
 * @property {string[]?} platforms
 * @property {string[]?} bindings
 */
/**
 * @typedef {Object} ScriptModule
 * @property {string[]} module_names
 * @property {string[]} bindings
 */
/**
 * Get Script Packages used that will be generate bindings
 * @returns {Array<string>}
 */
function _getScriptPackages() {
    const target = path.resolve(engine_root, 'Script/Packages');
    return fs.readdirSync(target).filter(
        (file) => {
            const file_path = path.join(target, file);
            // skip files
            if (fs.statSync(file_path).isFile())
                return false;
            // To be a valid package, they must contains package.json
            return fs.existsSync(path.join(file_path, 'package.json'));
        }
    );
}
function _getScriptModules() {
    /** @type {{ [key: string]: ScriptModule }} */
    const modules = {};
    const script_packages = _getScriptPackages();
    script_packages.forEach(script_pkg => {
        /** @type {PackageDesc} */
        const pkg = JSON.parse(
            fs.readFileSync(
                path.resolve(engine_root, 'Script/Packages', script_pkg, 'package.json')
            )
        );

        if(!Array.isArray(pkg.modules))
            throw new Error(`Invalid Package '${script_pkg}'. package must be or contains modules definitions!`);
        if(typeof pkg.name === 'undefined')
            pkg.name = script_pkg;

        /** @type {ScriptModule} */
        let script_module;
        // Add package to modules data set.
        if(modules[pkg.name])
            script_module = modules[pkg.name];
        else {
            modules[pkg.name] = script_module = {
                module_names: [],
                bindings: pkg.bindings ?? []
            };
        }

        script_module.module_names = [
            ...script_module.module_names,
            ...pkg.modules
        ];
    });

    return modules;
}
function _genTsConfigs() {
    console.log('- Generating TsConfigs');
    const tsconfig_files = fs.readdirSync(script_dir, { recursive: true})
        .filter(x => path.basename(x) === 'tsconfig.js')
        .map(x => path.resolve(script_dir, x));

    console.log(`- Found (${tsconfig_files.length}) tsconfig scripts.`);
    const tsconfig_call_params = {
        constants,
        engine_root,
        artifacts_root: engineGetArtifactsRoot()
    };
    tsconfig_files.forEach(x => {
        const call = require(x);
        const tsconfig_instance = call(tsconfig_call_params);

        const dir = path.dirname(x);
        const data_2_write = JSON.stringify(tsconfig_instance, null, '\t');

        fs.writeFileSync(path.join(dir, 'tsconfig.json'), data_2_write);
    });

    console.log('- TsConfigs generated with success!');
}
async function _genCombinedDefinition() {
    console.log('- Generating Combined engine.d.ts');
    // Writing basic tsconfig file to generated typescript types
    const ts_cfg_base = JSON.parse(
        fs.readFileSync(
            path.resolve(script_dir, 'TypeScript/tsconfig.json')
        ).toString()
    );
    ts_cfg_base.exclude = [
        'dist/'+constants.engine_typescript_definitions+'.d.ts'
    ];
    fs.writeFileSync(
        path.resolve(engineGetArtifactsRoot(), 'Build/TypeScript/tsconfig.json'),
        JSON.stringify(ts_cfg_base, null, '\t')
    );


    const dts_gen_args = [
        path.resolve(engine_root, 'Build/node_modules/dts-generator/bin/dts-generator'),
        '--name', constants.engine_typescript_definitions,
        '--project', path.resolve(engineGetArtifactsRoot(), 'Build/TypeScript'),
        '--out', path.resolve(
            engineGetArtifactsRoot(), 
            'Build/TypeScript/dist', 
            constants.engine_typescript_definitions + '.d.ts'
        )
    ];
    await execAsync('node', dts_gen_args);
}

async function bindingsGenerate() {
    console.log('- Generating Bindings');

    if(fs.existsSync(typescript_generated_types_path)) {
        console.log('- Clearing Generated Definitions Files');
        fs.removeSync(typescript_generated_types_path);
    }

    const script_modules = _getScriptModules();
    const binding_calls = Object.keys(script_modules).map(module_name => {
        return async ()=> {
            console.log(`- Generating ${module_name} Bindings`);

            const args = [
                'bind',
                engine_root,
                `Script/Packages/${module_name}/`
            ];
            await execAsync(engine_tool, args, { cwd : engine_root });  
            console.log(`- ${module_name} bindings was generated with success!`);
        };
    });

    for(let i = 0; i < binding_calls.length; ++i)
        await binding_calls[i]();

    console.log('- Finished Bindings Generate');
}

async function bindingsBuildTypescript() {
    const excluded_items = new Set([
        'TypeScript'
    ]);

    _genTsConfigs();

    console.log('- Building TypeScript Scripts');
    
    // find directory that contains tsconfig.json and build
    const script_path = path.join(engine_root, 'Script');
    const typescript_projects = fs.readdirSync(script_path).filter(x => {
        const target_path = path.resolve(script_path, x);
        // Skip typescript project if is on exclusion list
        if(excluded_items.has(x))
            return false;
        if(fs.statSync(target_path).isFile())
            return false;
        return fs.existsSync(path.resolve(target_path, 'tsconfig.json'));
    }).map(x => path.resolve(script_path, x));
    typescript_projects.unshift(script_path); // script path is typescript project

    const tsc_path = path.resolve(engine_root, 'Build/node_modules/typescript/lib/tsc.js');
    // run typescript build
    for(let i = 0; i < typescript_projects.length; ++i) {
        const tsc_proj = typescript_projects[i];
        await execAsync('node', [tsc_path, '--project', tsc_proj]);
    }

    await _genCombinedDefinition();

    console.log('- Copying Generated files to Artifacts');

    const editor_modules_dir = path.resolve(engineGetArtifactsRoot(), 'Build/Resources/EditorData/EditorScripts/', constants.engine_editor_name, 'modules');
    const web_editor_modules_dir = path.resolve(engine_root, 'Data/CodeEditor/source/editorCore/modules');
    const node_modules_dir = path.resolve(engine_root, 'Build/node_modules');

    fs.mkdirSync(editor_modules_dir, { recursive: true});
    // TypeScript
    fs.copySync(
        path.resolve(node_modules_dir, 'typescript/lib/typescript.js'), 
        path.resolve(web_editor_modules_dir, 'typescript.js')
    );

    // copy lib.core.d.ts into the tool data directory
    fs.mkdirSync(
        path.resolve(
            engineGetArtifactsRoot(), 
            'Build/Resources/EditorData/EditorScripts', 
            constants.engine_editor_name, 
            'TypeScriptSupport'
        ),
        { recursive : true }
    );
    // copy the combined EngineCore.d.ts to the tool data directory
    fs.copySync(
        path.resolve(engineGetArtifactsRoot(), 'Build/TypeScript/dist/', constants.engine_typescript_definitions+'.d.ts'),
        path.resolve(engineGetArtifactsRoot(), 'Data/TypeScriptSupport/', constants.engine_typescript_definitions+'.d.ts')
    );
    console.log('- Finished TypeScript Build Scripts');
}

async function bindingsCleanTypescript() {
    console.log('- Cleaning Generated TypeScript TsConfigs');

    const ts_config_projects = fs.readdirSync(script_dir, { recursive : true })
        .filter(x => fs.statSync(path.join(script_dir, x)).isFile() && path.basename(x) === 'tsconfig.js')
        .map(x => path.resolve(script_dir, path.dirname(x), 'tsconfig.json'));

    let removed_count = 0;
    ts_config_projects.forEach(x => {
        if(fs.existsSync(x))
            ++removed_count;
        fs.removeSync(x);
    });

    console.log(`- Removed (${removed_count}) files.`);
}

async function bindingsLintTypescript() {
    const typescript_projects = [
        constants.engine_editor_name+'/**/*.ts',
        'AtomicWebViewEditor/**/*.ts'
    ];
    const file_mask = '{' + typescript_projects.join(',') +'}';
    const script_path = path.resolve(engine_root, 'Script');
    
    console.log('TSLINT: Linting files in '+file_mask);

    const lint_cfg = JSON.parse(fs.readFileSync(path.join(script_path, 'tslint.json')));
    const tslint_cfg = {
        configuration: lint_cfg,
        formatter: 'prose'
    };

    // TODO: change this to eslint
    // lint
    // Since TSLint does not yet support recursively searching for files, then we need to
    // create a command per file.  The main issue with this is that it will abort on the first error instead
    // of listing out all lint errors
    const results = await glob(file_mask, { cwd : script_path });
    const lint_errors = [];

    results.forEach(filename => {
        // skip declaration types
        if(filename.endsWith('.d.ts'))
            return;
        const contents = fs.readFileSync(path.resolve(script_path, filename), 'utf-8');
        const lint_instance = new TsLint(filename, contents, tslint_cfg);
        const result = lint_instance.lint();
        if(result.failureCount > 0)
            lint_errors.push(result.output);
    });

    if(lint_errors.length > 0) {
        console.warn('TSLINT: WARNING - Lint errors detected');
        console.warn(lint_errors.join(' '));
        throw new Error('TSLint errors detected');
    }

    console.log('TSLINT: Success!');
}

module.exports = {
    bindingsGenerate,
    bindingsBuildTypescript,
    bindingsLintTypescript,
    bindingsCleanTypescript
}