//
// Copyright (c) 2014-2016 THUNDERBEAST GAMES LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

import EditorUI = require("../ui/EditorUI");
/**
 * Interface around a project template
 */
export interface ProjectTemplateDefinition {
    name: string;
    desc: string;
    screenshot: string;
    folder: string;
    languages: string[];
    appDelegateClass: string;
    namespace: string;
}

// Supported project languages
var projectLanguages = ["CSharp", "JavaScript", "TypeScript"];

/**
 * Returns the structured template definition for the provided project type.
 * Note that the path to the templates folder will be fully resolved
 * @param  {string} projectType A template type.  One of project_empty, project_2d, project_3d
 * @return {ProjectTemplateDefinition}              the template definition for the proved project type
 */
export function getNewProjectTemplateDefinition(projectType: string): ProjectTemplateDefinition {

    var env = ToolCore.toolEnvironment;
    let fileSystem = EngineCore.fileSystem;
    let exampleInfoDir = env.toolDataDir + "ExampleInfo/";
    var projectTemplateFolder = env.toolDataDir + "ProjectTemplates/";
    var projectTemplateJsonFile = projectTemplateFolder + "ProjectTemplates.json";
    let jsonFile = new EngineCore.File(projectTemplateJsonFile, EngineCore.FileMode.FILE_READ);

    if (!jsonFile.isOpen()) {
        return null;
    }
    // Update all the paths to a more fully qualified path
    let json = JSON.parse(jsonFile.readText());
    let projectTemplate = <ProjectTemplateDefinition>json[projectType];

    if (projectTemplate) {

        projectTemplate.screenshot = exampleInfoDir + projectTemplate.screenshot;
        projectTemplate.folder = projectTemplateFolder + projectTemplate.folder + "/";

        projectTemplate.languages = [];

        for (var i = 0; i < projectLanguages.length; i++) {

            if (fileSystem.dirExists(projectTemplate.folder + projectLanguages[i])) {
                projectTemplate.languages.push(projectLanguages[i]);
            }

        }

    }

    return projectTemplate;
}

/**
 * Return an array of all of the example project definitions.
 * Note that the paths in both screenshot and folder will be fully resolved.
 * @return {ProjectTemplateDefinition[]} Array of example project definitions.
 */
export function getExampleProjectTemplateDefinitions(): ProjectTemplateDefinition[] {

    let env = ToolCore.toolEnvironment;
    let fileSystem = EngineCore.fileSystem;
    let exampleInfoDir = env.toolDataDir + "ExampleInfo/";
    let exampleJsonFile = exampleInfoDir + "Examples.json";
    let jsonFile = new EngineCore.File(exampleJsonFile, EngineCore.FileMode.FILE_READ);

    if (!jsonFile.isOpen()) {
        return;
    }

    let exampleFoldersJson = JSON.parse(jsonFile.readText());

    // list of example folders
    let exampleFolders : string[] = exampleFoldersJson.examples;

    let examples : ProjectTemplateDefinition[] = [];

    // Update all the paths to a more fully qualified path
    exampleFolders.forEach(exampleFolder => {

        let folder = EngineCore.addTrailingSlash(env.toolDataDir + "EngineExamples/" + exampleFolder);

        // if we're building from source tree, adjust the path
        if (EngineCore.AtomicBuildInfo.getDevBuild()) {
            folder = EngineCore.addTrailingSlash(env.rootSourceDir + "Submodules/EngineExamples/" + exampleFolder);
        }

        let screenshot = folder + "Screenshot.png";

        if (!fileSystem.dirExists(folder)) {
            console.log("Skipping non-existent example ", folder);
            return;
        }

        let exampleJsonFilename = folder + "AtomicExample.json";

        if (!fileSystem.fileExists(exampleJsonFilename)) {
            console.log("Skipping non-existent example json", exampleJsonFilename);
            return;
        }

        jsonFile = new EngineCore.File(exampleJsonFilename, EngineCore.FileMode.FILE_READ);

        if (!jsonFile.isOpen()) {
            console.log("Unable to open example json", exampleJsonFilename);
            return;
        }


        let exampleJson = JSON.parse(jsonFile.readText());

        if (!exampleJson.name || !exampleJson.desc) {
            console.log("Unable to parse example json", exampleJsonFilename);
            return;
        }

        let allLanguages = ["CSharp", "JavaScript", "TypeScript"];
        let languages = [];

        for (var i = 0; i < allLanguages.length; i++) {

            if (fileSystem.dirExists(folder + allLanguages[i])) {
                languages.push(allLanguages[i]);
            }

        }

        examples.push({
            name : exampleJson.name,
            desc : exampleJson.desc,
            screenshot : screenshot,
            folder : folder,
            languages : languages,
            appDelegateClass : exampleJson.appDelegateClass,
            namespace : exampleJson.namespace
        });

    });

    return examples;
}

/**
 * Return an array of all of the file type templates for the provided file type
 * @param  {string} fileTemplateType
 * @return {[FileTemplateDefinition]}
 */
export function GetNewFileTemplateDefinitions(fileTemplateType: string) : EngineEditor.Templates.FileTemplateDefinition[] {
    const templateDefinitions = "AtomicEditor/templates/file_template_definitions.json";
    const file = EngineCore.cache.getFile(templateDefinitions);

    if (!file) {
        return [];
    }

    const templates = JSON.parse(file.readText());
    return templates[fileTemplateType] || [];
}

// AtomicNET

export interface AtomicNETProjectInfo {
    name: string;
    appID: string;
    platforms: string[];
    projectFolder: string;
    projectTemplate: ProjectTemplateDefinition;
}

var atomicNETProjectInfo:AtomicNETProjectInfo;

/**
 * Processes an AtomicNET template, replacing strings with settings
 * @param  {string} filename
 * @param  {string} templateFilename
 * @return {boolean}
 */
function processAtomicNETTemplate(filename:string, templateFilename:string) : boolean {

    let file = new EngineCore.File(templateFilename, EngineCore.FileMode.FILE_READ);

    if (!file.isOpen()) {
        console.log("Failed to open: ", templateFilename);
        return false;
    }

    let text = file.readText();

    let _namespace = "";
    if (atomicNETProjectInfo.projectTemplate.namespace) {
        _namespace = "using " + atomicNETProjectInfo.projectTemplate.namespace + ";";
    }

    let appDelegateClass = "AtomicMain";
    if (atomicNETProjectInfo.projectTemplate.appDelegateClass) {
        appDelegateClass = atomicNETProjectInfo.projectTemplate.appDelegateClass;
    }

    text = text.split("$$APPLICATION_NAME$$").join(atomicNETProjectInfo.name);
    text = text.split("$$APPLICATION_ID$$").join(atomicNETProjectInfo.appID);
    text = text.split("$$APPLICATION_APPDELEGATECLASS$$").join(appDelegateClass);
    text = text.split("$$APPLICATION_NAMESPACE$$").join(_namespace);

    let fileOut = new EngineCore.File(filename, EngineCore.FileMode.FILE_WRITE);

    if (!fileOut.isOpen()) {
        console.log("Failed to open for write: ", filename);
        return false;
    }

    fileOut.writeString(text);

    file.close();

    return true;
}

/**
 * Generates the Android portion of an AtomicNET project
 * @return {boolean}
 */
function generateAtomicNETAndroidProject():boolean {

    let env = ToolCore.toolEnvironment;
    let utils = new EngineEditor.FileUtils();
    let templateFolder = env.toolDataDir + "EngineNET/ProjectTemplate/";
    let androidFolder = EngineCore.addTrailingSlash(atomicNETProjectInfo.projectFolder) + "Project/EngineNET/Platforms/Android/";

    let fileSystem = EngineCore.fileSystem;

    // Create necessary folders
    let folders = ["Properties", "Resources/drawable", "Resources/values"];
    for (var i = 0; i < folders.length; i++) {

        let folder = androidFolder + folders[i];

        if (!fileSystem.dirExists(folder)) {

            if (!utils.createDirs(folder))
                return false;
        }

    }

    let textFiles = [".cs", ".xml"];

    let files = ["MainActivity.cs", "Resources/Resource.Designer.cs", "Resources/drawable/icon.png",
                 "Resources/values/Strings.xml", "Properties/AndroidManifest.xml", "Properties/AssemblyInfo.cs"];

    for (var i = 0; i < files.length; i++) {

        let templateName = templateFolder + "Platforms/Android/" + files[i];
        let filename = androidFolder + files[i];

        if (textFiles.indexOf(EngineCore.getExtension(templateName)) == -1) {

            if (!fileSystem.copy(templateName, filename)) {

                console.log("Failed to copy: ", templateName, " to ",  filename);
                return false;
            }

        } else {

            if (!processAtomicNETTemplate(filename, templateName)) {
                return false;
            }

        }

    }

    return true;
}

/**
 * Generates the iOS portion of an AtomicNET project
 * @return {boolean}
 */
function generateAtomicNETIOSProject():boolean {

    let env = ToolCore.toolEnvironment;
    let utils = new EngineEditor.FileUtils();
    let templateFolder = env.toolDataDir + "EngineNET/ProjectTemplate/";
    let iosFolder = EngineCore.addTrailingSlash(atomicNETProjectInfo.projectFolder) + "Project/EngineNET/Platforms/iOS/";

    let fileSystem = EngineCore.fileSystem;

    // Create necessary folders
    let folders = ["Properties", "Resources"];
    for (var i = 0; i < folders.length; i++) {

        let folder = iosFolder + folders[i];

        if (!fileSystem.dirExists(folder)) {

            if (!utils.createDirs(folder))
                return false;
        }

    }

    let textFiles = [".cs", ".plist"];

    let files = ["Main.cs", "AppUIDelegate.cs", "Entitlements.plist", "Info.plist",
                 "Properties/AssemblyInfo.cs"];

    for (var i = 0; i < files.length; i++) {

        let templateName = templateFolder + "Platforms/iOS/" + files[i];
        let filename = iosFolder + files[i];

        if (textFiles.indexOf(EngineCore.getExtension(templateName)) == -1) {

            if (!fileSystem.copy(templateName, filename)) {

                console.log("Failed to copy: ", templateName, " to ",  filename);
                return false;
            }

        } else {

            if (!processAtomicNETTemplate(filename, templateName)) {
                return false;
            }

        }

    }

    return true;
}


/**
 * Generates the Desktop portion of an AtomicNET project
 * @return {boolean}
 */
function generateAtomicNETDesktopProject():boolean {

    let env = ToolCore.toolEnvironment;
    let utils = new EngineEditor.FileUtils();
    let templateFolder = env.toolDataDir + "EngineNET/ProjectTemplate/";
    let desktopFolder = EngineCore.addTrailingSlash(atomicNETProjectInfo.projectFolder) + "Project/EngineNET/Platforms/Desktop/";

    let fileSystem = EngineCore.fileSystem;

    if (!fileSystem.dirExists(desktopFolder)) {

        if (!utils.createDirs(desktopFolder))
            return false;
    }

    if (!processAtomicNETTemplate(desktopFolder + "Program.cs", templateFolder + "Platforms/Desktop/Program.cs")) {
        return false;
    }

    return true;
}


/**
 * Generates an AtomicNET project from templates
 * @param  {AtomicNETProjectInfo} projectInfo
 * @return {boolean}
 */
export function generateAtomicNETProject(projectInfo:AtomicNETProjectInfo):boolean {

    atomicNETProjectInfo = projectInfo;

    let env = ToolCore.toolEnvironment;
    let templateFolder = env.toolDataDir + "EngineNET/ProjectTemplate/";
    let platformsFolder = EngineCore.addTrailingSlash(projectInfo.projectFolder) + "Project/EngineNET/Platforms/";

    let utils = new EngineEditor.FileUtils();
    let fileSystem = EngineCore.fileSystem;

    if (!fileSystem.dirExists(platformsFolder)) {

        if (!utils.createDirs(platformsFolder))
            return false;
    }

    if (!generateAtomicNETDesktopProject()) {
        return false;
    }

    if (projectInfo.platforms.indexOf("android") != -1) {

        if (!generateAtomicNETAndroidProject()) {
            return false;
        }

    }

    if (projectInfo.platforms.indexOf("ios") != -1) {

        if (!generateAtomicNETIOSProject()) {
            return false;
        }

    }


    return true;
}
