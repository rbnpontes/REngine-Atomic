const path = require('path');
const axios = require('axios');
const fs = require('fs-extra');
const { engineGetRoot, engineGetArtifactsRoot } = require('../Utils/EngineUtils');
/**
 * @typedef {Object} CefBinInfoBinaries
 * @property {string} Windows
 * @property {string} Linux
 * @property {string} MacOS_arm64
 * @property {string} MacOS_x86_64
 * 
 * @typedef {Object} CefBinInfo
 * @property {string} resources
 * @property {CefBinInfoBinaries} binaries
 */

/**
 * 
 * @returns {CefBinInfo}
 */
function _getCefBinInfo() {
    const root = engineGetRoot();
    const filepath = path.resolve(root, 'Source/ThirdParty/CEF/cef_bin_info.json');
    return require(filepath);
}

/**
 * @param {string} identifier
 * @param {string} url 
 * @param {string} target_path 
 * @param {Boolean} noclean
 */
async function _downloadData(identifier, url, target_path, noclean) {
    console.log(`- Starting ${identifier} Download`);
    const dir = path.dirname(target_path);

    if(fs.existsSync(target_path)) {
        if(!noclean)
            fs.removeSync(target_path);
        else {
            console.log(`- File ${identifier} is already downloaded. Skipping!`);
            return;
        }
    }
    
    if (!fs.existsSync(dir)) {
        fs.mkdirpSync(dir);
        fs.createFileSync(target_path);
    }
    
    const writer = fs.createWriteStream(target_path);
    const task = axios({
        method: 'GET',
        url: url,
        responseType: 'stream',
        onDownloadProgress : (e)=> {
            console.log(`- Downloading ${identifier}: (${e.loaded}/${e.total})`);
        }
    }).then(res => {
        return new Promise((resolve, reject)=> {
            res.data.pipe(writer);
            writer.on('error', e=> {
                writer.close();
                reject(e);
            });
            writer.on('close', ()=> {
                resolve();
            });
        });
    });

    await task;
    console.log(`- Download ${identifier} finished.`);
}

/**
 * Download CEF Binaries
 * @param {'Windows' | 'Linux' | 'MacOS_arm64' | 'MacOS_x86_64'} platform 
 * @param {Boolean} noclean
 */
async function fetchCefBinaries(platform, noclean) {
    const filename = platform+'.7zip';
    const output_path = path.resolve(engineGetArtifactsRoot(), 'CEF', filename);
    const bin_info = _getCefBinInfo();
    const bin_url = bin_info.binaries[platform];
    if(!bin_url)
        throw new Error('Invalid platform type. Platform does\'t matches any of CEF binaries.');
    await _downloadData(filename, bin_url, output_path, noclean);
}

/**
 * Download CEF Resources
 * @param {Boolean} noclean 
 */
async function fetchCefResources(noclean) {
    const filename = 'Resources.7zip';
    const output_path = path.resolve(engineGetArtifactsRoot(), 'CEF', filename);
    const bin_info = _getCefBinInfo();
    await _downloadData(filename, bin_info.resources, output_path, noclean);
}

module.exports = {
    fetchCefBinaries,
    fetchCefResources
};