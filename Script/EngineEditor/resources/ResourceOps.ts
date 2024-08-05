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
import ProjectTemplates = require("ProjectTemplates");

class ResourceOps extends EngineCore.ScriptObject {

    constructor() {

        super();

        this.subscribeToEvent(ToolCore.AssetImportErrorEvent((ev: ToolCore.AssetImportErrorEvent) => {

            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: "Asset Import Error", message: ev.error }));

        }));

        this.subscribeToEvent(EngineEditor.RequestProjectLoadEvent((ev: EngineEditor.RequestProjectLoadEvent) => { this.handleRequestProjectLoad(ev); }));

    }

    handleRequestProjectLoad(ev:EngineEditor.RequestProjectLoadEvent) {

        var fs = EngineCore.fileSystem;
        var projectPath = EngineCore.addTrailingSlash(EngineCore.getPath(ev.path));

        var openProject = () => this.sendEvent(EngineEditor.EditorLoadProjectEventData({ path: ev.path }));

        // Check whether there is a cache folder, if so, this project has been loaded before
        if (EngineCore.fileSystem.dirExists(projectPath + "Cache")) {
            openProject();
            return;
        } else {

            // this may be an example
            var parentPath = EngineCore.getParentPath(projectPath);
            var exampleInfoPath = parentPath + "AtomicExample.json";

            if (!fs.fileExists(exampleInfoPath)) {

                openProject();
                return;
            }

            var jsonFile = new EngineCore.File(exampleInfoPath, EngineCore.FileMode.FILE_READ);

            if (!jsonFile.isOpen()) {
                return;
            }

            var exampleJson = JSON.parse(jsonFile.readText());

            var allLanguages = ["CSharp", "JavaScript", "TypeScript"];
            var language = null;

            for (var i = 0; i < allLanguages.length; i++) {

                if (projectPath.indexOf(allLanguages[i]) != -1) {
                    language = allLanguages[i];
                    break;
                }
            }

            if (!language) {
                return;
            }

            var projectDef = {
                name : exampleJson.name ? exampleJson.name : "Anonymous Example",
                desc : exampleJson.desc ? exampleJson.desc : "",
                screenshot : parentPath + "Screenshot.png",
                folder : parentPath,
                languages : [language],
                appDelegateClass : exampleJson.appDelegateClass,
                namespace : exampleJson.namespace

            };

            var ops = EditorUI.getModelOps();
            ops.showCreateProject(projectDef, projectPath);

        }

    }

}

var resourceOps = new ResourceOps();

export function CreateNewFolder(resourcePath: string, reportError: boolean = true): boolean {

    var title = "New Folder Error";

    var fs = EngineCore.getFileSystem();

    if (fs.dirExists(resourcePath) || fs.fileExists(resourcePath)) {
        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Already exists: " + resourcePath }));
        return false;

    }

    if (!fs.createDir(resourcePath)) {

        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Could not create " + resourcePath }));

        return false;
    }

    var db = ToolCore.getAssetDatabase();
    db.scan();

    return true;

}

export function CreateNewComponent(resourcePath: string, componentName: string, template: EngineEditor.Templates.FileTemplateDefinition, reportError: boolean = true): boolean {

    var title = "New Component Error";

    var fs = EngineCore.fileSystem;

    if (fs.dirExists(resourcePath) || fs.fileExists(resourcePath)) {
        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Already exists: " + resourcePath }));
        return false;

    }

    var templateFilename = template.filename;
    var file = EngineCore.cache.getFile(templateFilename);

    if (!file) {

        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Failed to open template: " + templateFilename }));
        return false;

    }

    var out = new EngineCore.File(resourcePath, EngineCore.FileMode.FILE_WRITE);
    var success = out.copy(file);
    out.close();

    if (!success) {
        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Failed template copy: " + templateFilename + " -> " + resourcePath }));
        return false;
    }

    ToolCore.assetDatabase.scan();

    return true;

}

export function CreateNewScript(resourcePath: string, scriptName: string, template: EngineEditor.Templates.FileTemplateDefinition, reportError: boolean = true): boolean {

    var title = "New Script Error";

    var fs = EngineCore.fileSystem;

    if (fs.dirExists(resourcePath) || fs.fileExists(resourcePath)) {
        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Already exists: " + resourcePath }));
        return false;

    }

    var templateFilename = template.filename;
    var file = EngineCore.cache.getFile(templateFilename);

    if (!file) {

        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Failed to open template: " + templateFilename }));
        return false;

    }

    var out = new EngineCore.File(resourcePath, EngineCore.FileMode.FILE_WRITE);
    var success = out.copy(file);
    out.close();

    if (!success) {
        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Failed template copy: " + templateFilename + " -> " + resourcePath }));
        return false;
    }

    ToolCore.assetDatabase.scan();

    return true;

}

export function CreateNewScene(resourcePath: string, sceneName: string, reportError: boolean = true): boolean {

    var title = "New Scene Error";

    var fs = EngineCore.fileSystem;

    if (fs.dirExists(resourcePath) || fs.fileExists(resourcePath)) {
        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Already exists: " + resourcePath }));
        return false;

    }

    var templateFilename = "EngineEditor/templates/template_scene.scene";
    var file = EngineCore.cache.getFile(templateFilename);

    if (!file) {

        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Failed to open template: " + templateFilename }));
        return false;

    }

    var out = new EngineCore.File(resourcePath, EngineCore.FileMode.FILE_WRITE);
    var success = out.copy(file);
    out.close();

    if (!success) {
        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Failed template copy: " + templateFilename + " -> " + resourcePath }));
        return false;
    }

    ToolCore.assetDatabase.scan();

    return true;

}

export function CreateNewMaterial(resourcePath: string, materialName: string, reportError: boolean = true): boolean {

    var title = "New Material Error";

    var fs = EngineCore.fileSystem;

    if (fs.dirExists(resourcePath) || fs.fileExists(resourcePath)) {
        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Already exists: " + resourcePath }));
        return false;

    }

    var templateFilename = "EngineEditor/templates/template_material.material";
    var file = EngineCore.cache.getFile(templateFilename);

    if (!file) {

        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Failed to open template: " + templateFilename }));
        return false;

    }

    var out = new EngineCore.File(resourcePath, EngineCore.FileMode.FILE_WRITE);
    var success = out.copy(file);
    out.close();

    if (!success) {
        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Failed template copy: " + templateFilename + " -> " + resourcePath }));
        return false;
    }

    ToolCore.assetDatabase.scan();

    return true;

}

//TODO - Replace this by creating a temporary scene that cannot be saved
export function CreateNewAnimationPreviewScene(reportError: boolean = true): boolean {

    var title = "Animation Viewer Error";

    var templateFilename = "EngineEditor/templates/template_scene.scene";
    var templateFile = EngineCore.cache.getFile(templateFilename);


    if (!templateFile) {

        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Failed to open template scene: " + templateFile }));
        return false;

    }

    var animFilename = "EngineEditor/templates/animation_viewer.scene";
    var animFile = EngineCore.cache.getFile(animFilename);

    if (!animFile) {

        if (reportError)
            resourceOps.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: title, message: "Failed to open animation viewer: " + animFilename }));
        return false;

    }

    //Reset the animation viewer scene to a blank scene
    animFile = templateFile;

    resourceOps.sendEvent(EngineEditor.EditorEditResourceEventData({ path: animFilename, lineNumber: 0 }));

    return true;

}
