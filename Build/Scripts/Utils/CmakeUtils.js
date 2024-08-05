const fs = require('fs');
const os = require('os');
const { execAsync } = require('../Utils/ProcessUtils');

/**
 * @typedef {Object} CMakeGenerateOptions
 * @property {string} generator Generator Type. Ex: 'Unix Makefile', 'Visual Studio 2022'
 * @property {string} source Path where is located source
 * @property {string} build Path where is project will be built
 * @property {string?} cwd Path where cmake will be running
 * @property {string[]?} additionalOptions Additional options that will be forward to cmake command
 */

/**
 * Execute CMake Generate Command
 * @param {CMakeGenerateOptions} options
 */
async function cmakeGenerate(options) {
    const { generator, source, build, additionalOptions, cwd } = options;

    // Create build path if isn't exists
    if (!fs.existsSync(build))
        fs.mkdirSync(build);

    const args = [
        '-S', source,
        '-B', build,
        '-G', generator,
        ...(additionalOptions ?? [])
    ];

    const err_code = await execAsync(
        os.platform() == 'win32' ? 'cmake.exe' : 'cmake',
        args,
        cwd
    );

    if(err_code != 0)
        throw new Error('CMake process exited with code ' + err_code);
}

module.exports = {
    cmakeGenerate
};