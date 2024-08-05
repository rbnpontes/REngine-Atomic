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

import EditorUI = require("../EditorUI");
import ModalWindow = require("./ModalWindow");
import ResourceOps = require("resources/ResourceOps");
import ProjectTemplates = require("resources/ProjectTemplates");

export class ResourceDelete extends ModalWindow {

    constructor(asset: ToolCore.Asset) {

        super();

        this.asset = asset;
        this.init("Delete Resource", "editor/ui/resourcedelete.tb.txt");
        var message = <EngineCore.UIEditField>this.getWidget("message");

        var text = "Are you sure you want to delete resource:\n\n";
        text += asset.path;
        text += "\n\nThis operation cannot be undone";

        message.text = text;

        this.resizeToFitContent();
        this.center();

    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "delete") {

                this.hide();

                let eventData = {
                    path: this.asset.path
                };

                var db = ToolCore.getAssetDatabase();
                db.deleteAsset(this.asset);

                this.sendEvent(EngineEditor.EditorDeleteResourceNotificationEventData(eventData));

                return true;
            }

            if (id == "cancel") {

                this.hide();

                return true;
            }

        }

    }

    asset: ToolCore.Asset;

}

export class CreateFolder extends ModalWindow {

    constructor(resourcePath: string) {

        super();

        this.resourcePath = resourcePath;
        this.init("New Folder", "editor/ui/resourcenewfolder.tb.txt");
        this.nameField = <EngineCore.UIEditField>this.getWidget("folder_name");
    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "create") {

                var resourcePath = EngineCore.addTrailingSlash(this.resourcePath) + this.nameField.text;

                if (ResourceOps.CreateNewFolder(resourcePath)) {

                    this.hide();

                }

                return true;

            }

            if (id == "cancel") {

                this.hide();

                return true;
            }

        }

    }

    resourcePath: string;
    nameField: EngineCore.UIEditField;

}

export class CreateComponent extends ModalWindow {

    constructor(resourcePath: string) {

        super();

        this.resourcePath = resourcePath;
        this.init("New Component", "editor/ui/resourcecreatecomponent.tb.txt");
        this.nameField = <EngineCore.UIEditField>this.getWidget("component_name");
        this.templateField = <EngineCore.UISelectDropdown>this.getWidget("template_list");
        this.loadTemplatesList();
    }

    /**
     *
     * Gets the template definitions and loads it up
     */
    loadTemplatesList() {
        this.templates = ProjectTemplates.GetNewFileTemplateDefinitions("component");
        this.templateFieldSource.clear();

        this.templates.forEach( template => {
            this.templateFieldSource.addItem(new EngineCore.UISelectItem(template.name));
        });

        this.templateField.source = this.templateFieldSource;
        this.templateField.value = 0;
    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "create") {

                var componentName = this.nameField.text;
                var outputFile = EngineCore.addTrailingSlash(this.resourcePath) + componentName;
                let selectedTemplate : EngineEditor.Templates.FileTemplateDefinition = null;
                this.templates.forEach(t => {
                    if (t.name == this.templateField.text) {
                        selectedTemplate = t;
                    }
                });

                if (selectedTemplate) {
                    // Check to see if we have a file extension.  If we don't then use the one defined in the template
                    if (outputFile.indexOf(".") == -1) {
                        outputFile += selectedTemplate.ext;
                    }

                    if (ResourceOps.CreateNewComponent(outputFile, componentName, selectedTemplate)) {

                        this.hide();

                        this.sendEvent(EngineEditor.EditorEditResourceEventData({ path: outputFile, lineNumber: 0 }));

                    }

                    return true;
                } else {
                    return false;
                }

            }

            if (id == "cancel") {

                this.hide();

                return true;
            }

        }

    }

    templates: EngineEditor.Templates.FileTemplateDefinition[];
    resourcePath: string;
    nameField: EngineCore.UIEditField;
    templateField: EngineCore.UISelectDropdown;
    templateFieldSource: EngineCore.UISelectItemSource = new EngineCore.UISelectItemSource();

}

export class CreateScript extends ModalWindow {

    constructor(resourcePath: string) {

        super();

        this.resourcePath = resourcePath;
        this.init("New Script", "editor/ui/resourcecreatescript.tb.txt");
        this.nameField = <EngineCore.UIEditField>this.getWidget("script_name");
        this.templateField = <EngineCore.UISelectDropdown>this.getWidget("template_list");
        this.loadTemplatesList();
    }

    /**
     *
     * Gets the template definitions and loads it up
     */
    loadTemplatesList() {
        this.templates = ProjectTemplates.GetNewFileTemplateDefinitions("script");
        this.templateFieldSource.clear();

        this.templates.forEach( template => {
            this.templateFieldSource.addItem(new EngineCore.UISelectItem(template.name));
        });

        this.templateField.source = this.templateFieldSource;
        this.templateField.value = 0;
    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "create") {

                var scriptName = this.nameField.text;
                var outputFile = EngineCore.addTrailingSlash(this.resourcePath) + scriptName;
                let selectedTemplate : EngineEditor.Templates.FileTemplateDefinition = null;
                this.templates.forEach(t => {
                    if (t.name == this.templateField.text) {
                        selectedTemplate = t;
                    }
                });

                if (selectedTemplate) {
                    // Check to see if we have a file extension.  If we don't then use the one defined in the template
                    if (outputFile.lastIndexOf(`.${selectedTemplate.ext}`) != outputFile.length - selectedTemplate.ext.length) {
                        outputFile += selectedTemplate.ext;
                    }

                    if (ResourceOps.CreateNewScript(outputFile, scriptName, selectedTemplate)) {

                        this.hide();

                        this.sendEvent(EngineEditor.EditorEditResourceEventData({ path: outputFile, lineNumber: 0 }));

                    }

                    return true;
                } else {
                    return false;
                }

            }

            if (id == "cancel") {

                this.hide();

                return true;
            }

        }

    }

    templates: EngineEditor.Templates.FileTemplateDefinition[];
    resourcePath: string;
    nameField: EngineCore.UIEditField;
    templateField: EngineCore.UISelectDropdown;
    templateFieldSource: EngineCore.UISelectItemSource = new EngineCore.UISelectItemSource();

}

export class CreateScene extends ModalWindow {

    constructor(resourcePath: string) {

        super();

        this.resourcePath = resourcePath;
        this.init("New Scene", "editor/ui/resourcecreateresource.tb.txt");
        this.nameField = <EngineCore.UIEditField>this.getWidget("component_name");
    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "create") {

                var sceneName = this.nameField.text;
                var outputFile = EngineCore.addTrailingSlash(this.resourcePath) + sceneName;

                if (outputFile.indexOf(".scene") == -1) outputFile += ".scene";

                if (ResourceOps.CreateNewScene(outputFile, sceneName)) {

                    this.hide();

                    this.sendEvent(EngineEditor.EditorEditResourceEventData({ path: outputFile, lineNumber: 0 }));

                }

                return true;

            }

            if (id == "cancel") {

                this.hide();

                return true;
            }

        }

    }

    resourcePath: string;
    nameField: EngineCore.UIEditField;

}

export class CreateMaterial extends ModalWindow {

    constructor(resourcePath: string) {

        super();

        this.resourcePath = resourcePath;
        this.init("New Material", "editor/ui/resourcecreateresource.tb.txt");
        this.nameField = <EngineCore.UIEditField>this.getWidget("component_name");
    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "create") {

                var materialName = this.nameField.text;
                var outputFile = EngineCore.addTrailingSlash(this.resourcePath) + materialName;

                if (outputFile.indexOf(".material") == -1) outputFile += ".material";

                if (ResourceOps.CreateNewMaterial(outputFile, materialName)) {

                    this.hide();

                    this.sendEvent(EngineEditor.EditorEditResourceEventData({ path: outputFile, lineNumber: 0 }));

                }

                return true;

            }

            if (id == "cancel") {

                this.hide();

                return true;
            }

        }

    }

    resourcePath: string;
    nameField: EngineCore.UIEditField;

}

export class RenameAsset extends ModalWindow {

    constructor(asset: ToolCore.Asset) {

        super();

        this.asset = asset;
        this.init("Rename Resource", "editor/ui/renameasset.tb.txt");

        var currentName = <EngineCore.UITextField>this.getWidget("current_name");
        this.nameEdit = <EngineCore.UIEditField>this.getWidget("new_name");

        currentName.text = asset.name;
        this.nameEdit.text = asset.name;

        this.resizeToFitContent();
        this.center();

        this.nameEdit.setFocus();
    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "rename") {

                this.hide();

                if (this.asset.name != this.nameEdit.text) {
                    let oldPath = this.asset.path;
                    this.asset.rename(this.nameEdit.text);

                    let eventData: EngineEditor.EditorRenameResourceNotificationEvent = {
                        path: oldPath,
                        newPath: this.asset.path,
                        newName: this.nameEdit.text,
                        asset: this.asset
                    };

                    this.sendEvent(EngineEditor.EditorRenameResourceNotificationEventData(eventData));
                }

                return true;
            }

            if (id == "cancel") {

                this.hide();

                return true;
            }

        }

    }

    nameEdit: EngineCore.UIEditField;
    asset: ToolCore.Asset;

}
