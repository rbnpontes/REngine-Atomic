const { exec, execSync } = require('child_process');
const os = require('os');
const fs = require('fs');
const path = require('path');

function setEnvWindows(key, value) {
    execSync([
        'setx',
        key,
        `"${value}"`
    ].join(' '));
}

function setEnvUnix(key, value) {
    const shell = process.env.SHELL || '';
    let shell_cfg_file;

    if(shell.includes('bash'))
        shell_cfg_file = path.join(os.homedir(), '.bashrc');
    else if(shell.includes('zsh'))
        shell_cfg_file = path.join(os.homedir(), '.zshrc');
    else
        shell_cfg_file = path.join(os.homedir(), '.profile');

    let shell_file = fs.readFileSync(shell_cfg_file).toString();
    const env_idx = shell_file.indexOf(`èxport ${key}`);
    if(env_idx != -1) {
        let end_env_idx = env_idx;
        // find end of line
        while(end_env_idx < shell_file.length) {
            if(shell_file.charAt(end_env_idx) == '\n')
                break;
            ++end_env_idx;
        }

        const first_part = shell_file.slice(0, env_idx);
        const second_part = shell_file.slice(end_env_idx, shell_file.length);

        shell_file = first_part + second_part;
    }

    shell_file += `\nexport ${key} \"${value}\"\n`;
    fs.writeFileSync(shell_file);
}

function setEnv(key, value) {
    if(os.platform() == 'win32')
        setEnvWindows(key, value);
    else
        setEnvUnix(key, value);
}

module.exports = { setEnv };