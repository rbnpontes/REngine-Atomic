const path = require('path');
const fs = require('fs-extra');

module.exports = (params)=> {
    const { constants, artifacts_root, engine_root } = params;

    const ts_config_base = {
        compilerOptions : {
            module: 'system',
            target: 'ES6',
            declaration: false,
            noImplicitAny: false,
            removeComments: true,
            noLib: false,
            outDir: path.resolve(engine_root, 'Data/CodeEditor/source/editorCore'),
            sourceMap: true,
            forceConsistentCasingInFileNames: true
        },
        filesGlob: [
            './**/*.ts',
            '../TypeScript/*.ts',
            path.resolve(artifacts_root, 'Build/TypeScript/') + path.sep + '*.ts',
            '!'+path.resolve(artifacts_root, 'Build/TypeScript/', constants.engine_net_name+'.d.ts'), 
        ],
        atom: {
            rewriteTsconfig: true
        },
        files: [
            ...fs.readdirSync(__dirname, { recursive: true })
                .filter(x => x.endsWith('.ts'))
                .map(x => './'+x),
            ...fs.readdirSync(path.resolve(engine_root, 'Script/TypeScript'))
                .filter(x => x.endsWith('.ts'))
                .map(x => '../TypeScript/'+x),
            ...fs.readdirSync(path.resolve(artifacts_root, 'Build/TypeScript'))
                .filter(x => x.endsWith('.ts'))
                .map(x => path.resolve(artifacts_root, 'Build/TypeScript', x))
        ]
    }

    return ts_config_base;
};