var fs = require('fs-extra');
var path = require("path");
var host = require("./Host");
var buildTasks = require("./BuildTasks");
var config = require("./BuildConfig");
const { getCMakeFlags } = require('./CMakeUtils');

const nodeSpawn = require('child_process').spawn;

var atomicRoot = config.atomicRoot;

namespace('build', function() {

  // converts / to \ and removes trailing slash
  function fixpath(path) {
    return path.replace(/\//g, "\\").replace(/\\$/, "");
  }

  // spawn cmake process
  function spawnCMake(vsver) {

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

    // we're using nodeSpawn here instead of jake.exec as the later was having much trouble with quotes
    var cmakeProcess = nodeSpawn("cmd.exe", args);

    cmakeProcess.stdout.on('data', (data) => {
      process.stdout.write(data.toString());
    });

    cmakeProcess.stderr.on('data', (data) => {
      process.stdout.write(data.toString());
    });

    cmakeProcess.on('exit', (code) => {

      if (code != 0) {
        fail(`CMake process exited with code ${code}`);
      }

      console.log("\n\n" + vsver + " solution created in " + fixpath(slnRoot) + "\n\n");

      complete();

    });

  }

  task('genvs2017', {
    async: true
  }, function() {

    spawnCMake("VS2017");

  }, {
    printStdout: true,
    printStderr: true
  });

  // Generate a Visual Studio 2015 solution
  task('genvs2015', {
    async: true
  }, function() {

    spawnCMake("VS2015");

  });

  task('genvs2022', {
    async: true
  }, function() {
    spawnCMake("VS2022");
  });
});// end of build namespace
