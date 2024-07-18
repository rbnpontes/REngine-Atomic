const path = require('path');
const fs = require('fs-extra');
const os = require('os');
const { glob } = require('glob');
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
function getScriptPackages() {
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
function getScriptModules() {
    /** @type {{ [key: string]: ScriptModule }} */
    const modules = {};
    const script_packages = getScriptPackages();
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

async function bindingsGenerate() {
    console.log('- Generating Bindings');

    const script_modules = getScriptModules();
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

    console.log('- Generating Combined engine.d.ts');
    const dts_gen_args = [
        path.resolve(engine_root, 'Build/node_modules/dts-generator/bin/dts-generator'),
        '--name', constants.engine_typescript_definitions,
        '--project', path.resolve(engine_root, 'Script/TypeScript'),
        '--out', path.resolve(
            engineGetArtifactsRoot(), 
            'Script/TypeScript/dist', 
            constants.engine_typescript_definitions + '.d.ts'
        )
    ];
    await execAsync('node', dts_gen_args);

    console.log('- Copying Generated files to Artifacts');

    const editor_modules_dir = path.resolve(engineGetArtifactsRoot(), 'Build/Resources/EditorData/EditorScripts/', constants.engine_editor_name, 'modules');
    const web_editor_modules_dir = path.resolve(engine_root, 'Data/CodeEditor/source/editorCore/modules');
    const node_modules_dir = path.resolve(engine_root, 'Build/node_modules');

    fs.mkdirSync(editor_modules_dir);
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
        )
    );
    // copy the combined EngineCore.d.ts to the tool data directory
    fs.copySync(
        path.resolve(engineGetArtifactsRoot(), 'Script/TypeScript/dist/', constants.engine_typescript_definitions+'.d.ts'),
        path.resolve(engineGetArtifactsRoot(), 'Data/TypeScriptSupport/', constants.engine_typescript_definitions+'.d.ts')
    );
    console.log('- Finished TypeScript Build Scripts');
}

async function bindingsCleanTypescript() {
    const exclude_list = [
        'Work.d.ts',
        'duktape.d.ts',
        'tsconfig.json',
        'README.md'
    ];
    const typescript_build_dir = path.resolve(engine_root, 'Script/TypeScript');
    
    console.log('- Cleaning TypeScript definition directory');

    let cleared_count = 0;
    fs.readdirSync(typescript_build_dir).forEach(x => {
        const filepath = path.join(typescript_build_dir, x);
        if(exclude_list.find(x => x.endsWith(filepath)))
            return;
        fs.removeSync(filepath);
        ++cleared_count;
    });

    console.log(`- Finished Typescript cleaning definition directory, Removed (${cleared_count}) files.`);
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