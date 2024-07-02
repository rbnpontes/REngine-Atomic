var fs = require('fs-extra');
var path = require("path");
var host = require("./Host");
var buildTasks = require("./BuildTasks");
var config = require("./BuildConfig");
const { getCMakeFlags } = require('./CMakeUtils');
const { jakeExecAsync } = require('./Utils/ProcessUtils');

var atomicRoot = config.atomicRoot;
var buildDir = config.artifactsRoot + "Build/Windows/";
var editorAppFolder = config.editorAppFolder

function copyAtomicNET() {

    if (!config["with-atomicnet"])
        return;

    fs.copySync(atomicRoot + "Artifacts/AtomicNET/" + config["config"],
    editorAppFolder + "Resources/ToolData/AtomicNET/" + config["config"]);

    fs.copySync(atomicRoot + "Script/AtomicNET/AtomicProject.json",
    editorAppFolder + "Resources/ToolData/AtomicNET/Build/Projects/AtomicProject.json");

}

// TODO: copy diligent binaries
function copyAtomicEditor() {

    // Copy the Editor binaries
    fs.copySync(buildDir + "Source/AtomicEditor/" + config["config"],
    config.artifactsRoot + "AtomicEditor");

    // copy AtomicTool
    fs.copySync(buildDir +  "Source/AtomicTool/" + config["config"] +"/AtomicTool.exe",
    editorAppFolder + "AtomicTool.exe");

    // We need some resources to run
    fs.copySync(atomicRoot + "Resources/CoreData",
    editorAppFolder + "Resources/CoreData");

    fs.copySync(atomicRoot + "Resources/PlayerData",
    editorAppFolder + "Resources/PlayerData");

    fs.copySync(atomicRoot + "Data/AtomicEditor",
    editorAppFolder + "Resources/ToolData");

    fs.copySync(atomicRoot + "Resources/EditorData",
    editorAppFolder + "Resources/EditorData");

    fs.copySync(atomicRoot + "Artifacts/Build/Resources/EditorData/AtomicEditor/EditorScripts",
    editorAppFolder + "Resources/EditorData/AtomicEditor/EditorScripts");

    fs.copySync(buildDir +  "Source/AtomicPlayer/Application/" + config["config"] +"/AtomicPlayer.exe",
    editorAppFolder + "Resources/ToolData/Deployment/Windows/x64/AtomicPlayer.exe");

    copyAtomicNET();

}

namespace('build', function() {
    function getVisualStudioVersion() {
        const expectedVersions = ['vs2015', 'vs2017', 'vs2022'];
        const cfgKeys = Object.keys(config);
        const vsver = cfgKeys.find(x => expectedVersions.indexOf(x) != -1);

        if(!vsver) {
            fail(`Invalid visual studio version. Use one of this [${expectedVersions.join(', ')}]`);
            return;
        }

        return vsver.toUpperCase();
    }

    async function buildEditorPhase2() {

    }
    async function buildEditor() {
        // Always cleanly create the editor target folder
        host.cleanCreateDir(editorAppFolder);

        // We clean atomicNET here as otherwise platform binaries would be deleted
        const createDirs = [config.artifactsRoot + "AtomicNET/", buildDir, host.getGenScriptRootDir()];
        const removeDirs = [config.artifactsRoot + "Build/Android/"];

        host.setupDirs(!config.noclean, createDirs, removeDirs);

        process.chdir(buildDir);
        const vsver = getVisualStudioVersion();

        const cmds = [];
        // Generate Engine solution, Engine Tool binary, and script bindings
        cmds.push(atomicRoot + "Build/Scripts/Windows/CompileAtomicEditorPhase1.bat " + config["config"] + " " +
                  vsver + " " + getCMakeFlags(false));

        await jakeExecAsync(cmds);

        await buildEditorPhase2();
        console.log(`- Finished. Atomic Editor built to ${editorAppFolder}`);
    }

    task('atomiceditor_phase2', {
        async: true
    }, function() {

        process.chdir(buildDir);

        const vsver = getVisualStudioVersion();

        var cmds = [];
        cmds.push(atomicRoot + "Build/Scripts/Windows/CompileAtomicEditorPhase2.bat " + config["config"] + " " + vsver);

        jake.exec(cmds, function() {

            copyAtomicEditor();

            if (config.package) {

                jake.Task['package:windows_editor'].invoke();

            }

            complete();

        }, {
            printStdout: true
        });

    });
    // Builds a standalone Atomic Editor, which can be distributed out of build tree
    task('atomiceditor', buildEditor);
    // task('atomiceditor', {
    //     async: true
    // }, function() {

    //     // Always cleanly create the editor target folder
    //     host.cleanCreateDir(editorAppFolder);

    //     // We clean atomicNET here as otherwise platform binaries would be deleted
    //     const createDirs = [config.artifactsRoot + "AtomicNET/", buildDir, host.getGenScriptRootDir()];

    //     const removeDirs = [config.artifactsRoot + "Build/Android/"];

    //     host.setupDirs(!config.noclean, createDirs, removeDirs);

    //     process.chdir(buildDir);

    //     const vsver = getVisualStudioVersion();

    //     var cmds = [];

    //     // Generate Atomic solution, AtomicTool binary, and script bindings
    //     cmds.push(atomicRoot + "Build/Scripts/Windows/CompileAtomicEditorPhase1.bat " + config["config"] + " " +
    //               vsver + " " + getCMakeFlags(false));

    //     jake.exec(cmds, function() {

    //         // var rootTask = jake.Task['build:atomiceditor_phase2'];

    //         // buildTasks.installBuildTasks(rootTask);

    //         // rootTask.addListener('complete', function () {

    //         //     console.log("\n\nAtomic Editor built to " + editorAppFolder + "\n\n");

    //         //     complete();
    //         // });

    //         // rootTask.invoke();
    //         complete();
    //     }, {
    //         printStdout: true,
    //         printStderr: true
    //     });

    // });

});// end of build namespace
