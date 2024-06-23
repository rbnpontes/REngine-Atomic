const path = require('path');
const axios = require('axios');
const fs = require('fs-extra');
const seven = require('node-7z');
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
 * 
 * @typedef {'Windows' | 'Linux' | 'MacOS_arm64' | 'MacOS_x86_64'} CefPlatformType
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
 * @param {CefPlatformType} platform 
 * @param {Boolean} noclean
 */
async function cefFetchBinaries(platform, noclean) {
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
async function cefFetchResources(noclean) {
    const filename = 'Resources.7zip';
    const output_path = path.resolve(engineGetArtifactsRoot(), 'CEF', filename);
    const bin_info = _getCefBinInfo();
    await _downloadData(filename, bin_info.resources, output_path, noclean);
}

/**
 * Extract CEF binaries
 * @param {CefPlatformType} platform 
 * @returns 
 */
async function cefExtractBinaries(platform) {
    console.log('- Extracting Binaries of '+platform);
    const archive_name = platform + '.7zip';
    const output_path = path.resolve(engineGetArtifactsRoot(), 'CEF', platform);
    const target_path = path.resolve(engineGetArtifactsRoot(), 'CEF', archive_name);

    if(fs.existsSync(output_path)) {
        console.log(`- ${archive_name} is already extracted. Skipping!!!`);
        return;
    }

    seven.extractFull(target_path, output_path);

    console.log(`- ${platform} binaries has been extracted with success!`);
}
/**
 * Extract CEF resources
 */
async function cefExtractResources() {
    console.log('- Extracing CEF Resources');
    const archive_name = 'Resources.7zip';
    const output_path = path.resolve(engineGetArtifactsRoot(), 'CEF', 'Resources');
    const target_path = path.resolve(engineGetArtifactsRoot(), 'CEF', archive_name);

    if(fs.existsSync(output_path)) {
        console.log(`- ${archive_name} is already extracted. Skipping!!!`);
        return;
    }

    seven.extractFull(target_path, output_path);

    console.log('- CEF Resources has been extrated with success!');
}

/**
 * Validate CEF resources
 * @param {CefPlatformType} platform
 */
async function cefPrepare(platform) {
    const artifacts_cef_root = path.join(engineGetArtifactsRoot(), 'CEF');
    const has_downloaded_bin = ()=> {
        return fs.existsSync(path.resolve(artifacts_cef_root, platform + '.7zip'));
    };
    const has_downloaded_res = ()=> {
        // On MacOS, we must skip resource download
        if(process.platform == 'darwin')
            return true;
        return fs.existsSync(path.resolve(artifacts_cef_root, 'Resources.7zip'));
    };
    const has_extracted_bin = ()=> {
        return fs.existsSync(path.resolve(artifacts_cef_root, platform));
    };
    const has_extracted_res = ()=> {
        if(process.platform == 'darwin')
            return true;
        return fs.existsSync(path.resolve(artifacts_cef_root, 'Resources'));
    };

    console.log('- Preparing CEF Dependencies');
    if(!has_downloaded_bin())
        await cefFetchBinaries(platform);
    if(!has_downloaded_res())
        await cefFetchResources(platform);
    if(!has_extracted_bin())
        await cefExtractBinaries(platform);
    if(!has_extracted_res())
        await cefExtractResources(platform);

    console.log('- Finished CEF Dependencies');
}

module.exports = {
    cefFetchBinaries,
    cefFetchResources,
    cefExtractBinaries,
    cefExtractResources,
    cefPrepare
};