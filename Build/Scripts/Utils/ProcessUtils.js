function jakeExecAsync(cmds) {
    return new Promise((resolve) => {
        jake.exec(cmds, () => {
            resolve();
        }, { printStdout: true, printStderr: true });
    });
}
/**
 * @typedef {Object} ProcessExecuteOptions
 * @property {string?} cwd Path where this executable will be executed
 * @property {boolean} noLogs Disable logs
 */
/**
 * Execute process
 * @param {string} procName Process name, Executable name or Batch name
 * @param {string[]} args Arguments
 * @param {ProcessExecuteOptions?} options
 * @return {Promise<number>} returns a promise, when their is fullfiled a error code will provided.
 */
function execAsync(procName, args, options) {
    const { spawn } = require('child_process');
    return new Promise((resolve) => {
        console.log(`- Initializing Process: ${procName}`);
        const proc = spawn(procName, args, { cwd: options?.cwd, shell: true });
        if(!options?.noLogs) {
            proc.stdout.on('data', data => console.log(data.toString()));
            proc.stderr.on('data', data => console.error(data.toString()));
        }
        proc.on('exit', code => {
            resolve(code);
        });
    });
}

/**
 * Execute process but returns their output 
 * @param {string} procName Process name, Executable name or Batch name
 * @param {string[]} args Arguments
 * @param {string?} cwd Path where this executable will be executed
 * @return {Promise<string>} returns a promise, when their is fullfiled a text output will be retrieve if process has any output.
 */
function spawnAsync(procName, args, cwd) {
    return new Promise((resolve, reject)=> {
        const { spawn } = require('child_process');
        const proc = spawn(procName, args, { cwd });
        const messages = [];
        const err_messages = [];
        proc.stdout.on('data', data => messages.push(data.toString()));
        proc.stderr.on('data', data => err_messages.push(data.toString()));
        proc.on('exit', code => {
            if(code != 0) {
                err_messages.unshift(`process '${procName}' failed with exit code ${code}`);
                reject(new Error(err_messages.join('\n')));
            }

            resolve(messages.join('\n'));
        });
    });
}

module.exports = {
    jakeExecAsync,
    execAsync,
    spawnAsync
};
