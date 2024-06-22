var fs = require('fs-extra');
var path = require("path");
var host = require("./Host");
var buildTasks = require("./BuildTasks");
var config = require("./BuildConfig");
const os = require('os');
const { getCMakeFlags } = require('./CMakeUtils');
const { setEnv } = require('./EnvUtils');

const nodeSpawn = require('child_process').spawn;

const atomicRoot = config.atomicRoot;


async function setupVsEnvs() {
  if (os.platform() != 'win32') {
    console.warn('Setup Visual Studio Environment is not supported on this platform. Skipping!!!');
    return;
  }

  console.log('- Setup Visual Studio Tools Environment Variable');
  const vswhere_base_path = path.resolve(atomicRoot, 'Build/Windows/vswhere.exe');
  
  console.log('- Getting Visual Studio Installation Path');
  const task = new Promise((resolve, reject)=> {
    const messages = [];
    const proc = nodeSpawn(vswhere_base_path, ['-latest', '-property', 'installationPath']);
    proc.stdout.on('data', data => messages.push(data.toString()));
    proc.on('exit', (code)=> {
      if(code == 0)
        resolve(messages.join('\n').trim());
      reject('Failed to get Visual Studio Installation Path. Code='+code);
    });
  });
  const vs_installation_path = await task;

  // Search at Installation Path for the file VsMSBuildCmd.bat, if found then we retrieve their directory path.
  const findToolsDirectory = ()=> {
    console.log('- Searching for Visual Studio Tools');
    // find VsMSBuildCmd, this is required for Engine Tool
    const vs_build_cmd = fs.readdirSync(vs_installation_path, { recursive: true}).find(x => x.endsWith('VsMSBuildCmd.bat'));
    if(!vs_build_cmd)
      throw new Error('Not found Visual Studio Tools directory');
    return path.basename(path.join(vs_installation_path, vs_build_cmd));
  };
  // First, we will look by the default directory that is Common7/Tools, if they have VsMSBuildCmd.bat we skip
  // Search step.
  const tryDefaultCommon7Dir = ()=> {
    console.log('- Searching for Visual Studio Tools default directory.');
    const target_path = path.resolve(vs_installation_path, 'Common7/Tools');
    if(!fs.existsSync(target_path))
      return null;
    return fs.readdirSync(target_path).find(x => x.endsWith('VsMSBuildCmd.bat')) ? target_path : null;
  };

  let tools_dir = tryDefaultCommon7Dir();
  if(!tools_dir)
    tools_dir = findToolsDirectory();

  // Setup VS Tools Environment variable, this is required to make engine tool working.
  setEnv('VS_TOOLS', tools_dir);
  console.log(`- Success. ${tools_dir}`);
}

namespace('build', function () {

  // converts / to \ and removes trailing slash
  function fixpath(path) {
    return path.replace(/\//g, "\\").replace(/\\$/, "");
  }


  // spawn cmake process
  async function spawnCMake(vsver) {
    await setupVsEnvs();

    host.cleanCreateDir(atomicRoot + "/Artifacts/Build/Source/Generated");

    var slnRoot = fixpath(path.resolve(atomicRoot, "") + "-" + vsver);

    // we're running cmd.exe, this exits the shell when the command have finished
    var args = ["/C"];

    // Windows batch file which runs cmake
    args.push(fixpath(atomicRoot + "\\Build\\Scripts\\Windows\\GenerateVSSolution.bat"));

    // vsver VS2015/VS2017
    args.push(vsver);

    // Atomic root source dir
    args.push(fixpath(atomicRoot));

    // Folder to put generated solution in
    args.push(fixpath(slnRoot));

    // CMake flags
    args.push(getCMakeFlags(true));


    const cmakeProcessTask = new Promise((resolve, reject) => {
      // we're using nodeSpawn here instead of jake.exec as the later was having much trouble with quotes
      const cmakeProcess = nodeSpawn("cmd.exe", args);

      cmakeProcess.stdout.on('data', (data) => {
        process.stdout.write(data.toString());
      });
      cmakeProcess.stderr.on('data', (data) => {
        process.stdout.write(data.toString());
      });

      cmakeProcess.on('exit', (code) => {
        if (code == 0) {
          console.log("\n\n" + vsver + " solution created in " + fixpath(slnRoot) + "\n\n");
          resolve();
        }
        else
          reject(`CMake process exited with code ${code}`);
      });
    });

    await cmakeProcessTask;
  }

  task('genvs2017', async () => {
    await spawnCMake("VS2017");
  }, {
    printStdout: true,
    printStderr: true
  });

  // Generate a Visual Studio 2015 solution
  task('genvs2015', async () => {
    await spawnCMake("VS2015");
  });

  task('genvs2022', async () => {
    await spawnCMake("VS2022");
  });
});// end of build namespace

namespace('win', () => {
  task('setupvsenvs', setupVsEnvs);
});