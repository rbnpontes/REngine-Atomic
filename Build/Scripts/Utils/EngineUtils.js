const path = require('path');

const engine_root = path.resolve(__dirname, '../../../');
const artifacts_root = path.resolve(engine_root, 'Artifacts');
const source_root = path.resolve(engine_root, 'Source');
const generated_script_root_dir = path.resolve(artifacts_root, 'Build/Source/Generated');

function engineGetRoot() { return engine_root; }
function engineGetArtifactsRoot() { return artifacts_root; }
function engineGetSourceRoot() { return source_root; }
function engineGetGenScriptsRoot() { return generated_script_root_dir; }

module.exports = {
    engineGetRoot,
    engineGetArtifactsRoot,
    engineGetSourceRoot,
    engineGetGenScriptsRoot
};