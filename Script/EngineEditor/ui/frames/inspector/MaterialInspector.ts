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

import ScriptWidget = require("ui/ScriptWidget");
import EditorUI = require("ui/EditorUI");

import TextureSelector = require("./TextureSelector");

var techniqueSource = new EngineCore.UIMenuItemSource();

var solidSource = new EngineCore.UIMenuItemSource();
solidSource.addItem(new EngineCore.UIMenuItem("Diffuse", "Diffuse"));
solidSource.addItem(new EngineCore.UIMenuItem("Diffuse Emissive", "Diffuse Emissive"));
solidSource.addItem(new EngineCore.UIMenuItem("Diffuse Normal", "Diffuse Normal"));
solidSource.addItem(new EngineCore.UIMenuItem("Diffuse Specular", "Diffuse Specular"));
solidSource.addItem(new EngineCore.UIMenuItem("Diffuse Normal Specular", "Diffuse Normal Specular"));
solidSource.addItem(new EngineCore.UIMenuItem("Diffuse Unlit", "Diffuse Unlit"));
solidSource.addItem(new EngineCore.UIMenuItem("No Texture", "No Texture"));

var tranSource = new EngineCore.UIMenuItemSource();
tranSource.addItem(new EngineCore.UIMenuItem("Alpha", "Alpha"));
tranSource.addItem(new EngineCore.UIMenuItem("Alpha Mask", "Alpha Mask"));
tranSource.addItem(new EngineCore.UIMenuItem("Additive", "Additive"));
tranSource.addItem(new EngineCore.UIMenuItem("Additive Alpha", "Additive Alpha"));
tranSource.addItem(new EngineCore.UIMenuItem("Emissive Alpha", "Emissive Alpha"));
tranSource.addItem(new EngineCore.UIMenuItem("Alpha AO", "Alpha AO"));
tranSource.addItem(new EngineCore.UIMenuItem("Alpha Mask AO", "Alpha Mask AO"));

var lightmapSource = new EngineCore.UIMenuItemSource();
lightmapSource.addItem(new EngineCore.UIMenuItem("Lightmap", "Lightmap"));
lightmapSource.addItem(new EngineCore.UIMenuItem("Lightmap Alpha", "Lightmap Alpha"));

var projectSource = new EngineCore.UIMenuItemSource();
var _ = new EngineCore.UIMenuItem();

var techniqueLookup = {
    "Techniques/Diff.xml": "Diffuse",
    "Techniques/DiffEmissive.xml": "Diffuse Emissive",
    "Techniques/DiffNormal.xml": "Diffuse Normal",
    "Techniques/DiffSpec.xml": "Diffuse Specular",
    "Techniques/DiffNormalSpec.xml": "Diffuse Normal Specular",
    "Techniques/DiffUnlit.xml": "Diffuse Unlit",
    "Techniques/DiffAlpha.xml": "Alpha",
    "Techniques/DiffAlphaMask.xml": "Alpha Mask",
    "Techniques/DiffAdd.xml": "Additive",
    "Techniques/NoTexture.xml": "No Texture",
    "Techniques/DiffLightMap.xml": "Lightmap",
    "Techniques/DiffLightMapAlpha.xml": "Lightmap Alpha"
};

var techniqueReverseLookup = {};
var projectTechniques = {};
var projectTechniquesAddress = {};

for (var key in techniqueLookup) {

    techniqueReverseLookup[techniqueLookup[key]] = key;

}

class MaterialInspector extends ScriptWidget {

    currentTexture: EngineCore.UITextureWidget = null;
    tunit: number;
    textureWidget: EngineCore.UITextureWidget;

    constructor() {

        super();

        this.fd.id = "Vera";
        this.fd.size = 11;

        this.subscribeToEvent(ToolCore.ResourceAddedEvent((ev: ToolCore.ResourceAddedEvent) => this.refreshTechniquesPopup()));
    }

    createShaderParametersSection(): EngineCore.UISection {

        var section = new EngineCore.UISection();
        section.text = "Shader Paramaters";
        section.value = 1;
        section.fontDescription = this.fd;

        var attrsVerticalLayout = new EngineCore.UILayout(EngineCore.UI_AXIS.UI_AXIS_Y);
        attrsVerticalLayout.spacing = 3;
        attrsVerticalLayout.layoutPosition = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
        attrsVerticalLayout.layoutSize = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_AVAILABLE;

        section.contentRoot.addChild(attrsVerticalLayout);

        var params = this.material.getShaderParameters();

        for (var i in params) {

            var attrLayout = new EngineCore.UILayout();
            attrLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;

            var name = new EngineCore.UITextField();
            name.textAlign = EngineCore.UI_TEXT_ALIGN.UI_TEXT_ALIGN_LEFT;
            name.skinBg = "InspectorTextAttrName";

            name.text = params[i].name;
            name.fontDescription = this.fd;

            attrLayout.addChild(name);

            var field = new EngineCore.UIEditField();
            field.textAlign = EngineCore.UI_TEXT_ALIGN.UI_TEXT_ALIGN_LEFT;
            field.skinBg = "TBAttrEditorField";
            field.fontDescription = this.fd;
            var lp = new EngineCore.UILayoutParams();
            lp.width = 140;
            field.layoutParams = lp;

            field.id = params[i].name;
            field.text = params[i].valueString;

            field.subscribeToEvent(field, EngineCore.UIWidgetEvent(function (ev: EngineCore.UIWidgetEvent) {

                if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CHANGED) {

                    var field = <EngineCore.UIEditField>ev.target;
                    this.material.setShaderParameter(field.id, field.text);

                }

            }.bind(this)));

            attrLayout.addChild(field);

            attrsVerticalLayout.addChild(attrLayout);

            // print(params[i].name, " : ", params[i].value, " : ", params[i].type);
        }
        return section;
    }

    getTextureThumbnail(texture: EngineCore.Texture): EngineCore.Texture {

        if (!texture) return null;

        var db = ToolCore.getAssetDatabase();
        var asset = db.getAssetByPath(texture.name);

        if (!asset)
            return texture;

        var thumbnail = asset.cachePath + "_thumbnail.png";
        var cache = EngineCore.getResourceCache();

        var thumb = <EngineCore.Texture2D>cache.getTempResource("Texture2D", thumbnail);

        if (thumb)
            return thumb;

        return texture;

    }

    onTechniqueSet(techniqueName: string) {

        this.techniqueButton.text = techniqueName;

        var cache = EngineCore.getResourceCache();
        var technique = <EngineCore.Technique>cache.getResource("Technique", techniqueReverseLookup[techniqueName]);
        var resourcePath = ToolCore.toolSystem.project.getResourcePath();

        if (technique == null) {
            var techniquePath = "";

            for (var i in projectTechniques) {
                if (techniqueName == projectTechniques[i]) {
                    techniquePath = projectTechniquesAddress[i];
                    break;
                }
            }
            techniquePath = techniquePath.replace(resourcePath, "");
            technique = <EngineCore.Technique>cache.getResource("Technique", techniquePath);
        }
        this.material.setTechnique(0, technique);
    }

    createTechniquePopup(): EngineCore.UIWidget {

        this.refreshTechniquesPopup();

        var button = this.techniqueButton = new EngineCore.UIButton();
        var technique = this.material.getTechnique(0);
        var techniqueName = "";

        if (technique != null) {
            techniqueName = technique.name.replace("Techniques/", "").replace(".xml", "");
        } else {
            techniqueName = "UNDEFINED";
        }

        button.text = techniqueName;

        button.fontDescription = this.fd;

        var lp = new EngineCore.UILayoutParams();
        lp.width = 140;
        button.layoutParams = lp;

        button.onClick = function () {

            var menu = new EngineCore.UIMenuWindow(button, "technique popup");

            menu.fontDescription = this.fd;
            menu.show(techniqueSource);

            button.subscribeToEvent(button, EngineCore.UIWidgetEvent(function (ev: EngineCore.UIWidgetEvent) {

                if (ev.type != EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK)
                    return;

                if (ev.target && ev.target.id == "technique popup") {

                    this.onTechniqueSet(ev.refid);

                }

            }.bind(this)));

        }.bind(this);

        return button;

    }

    acceptAssetDrag(importerTypeName: string, ev: EngineCore.DragEndedEvent): ToolCore.AssetImporter {

        var dragObject = ev.dragObject;

        if (dragObject.object && dragObject.object.typeName == "Asset") {

            var asset = <ToolCore.Asset>dragObject.object;

            if (asset.importerTypeName == importerTypeName) {
                return asset.importer;
            }

        }

        return null;

    }

    openTextureSelectionBox(textureUnit: number, textureWidget: EngineCore.UITextureWidget) {

        EditorUI.getModelOps().showResourceSelection("Select Texture", "TextureImporter", "Texture2D", function (asset: ToolCore.Asset, args: any) {

            if (asset == null) {
                this.createTextureRemoveButtonCallback(this.tunit, this.textureWidget);
                return;
            }

            var texture = <EngineCore.Texture2D>EngineCore.cache.getResource("Texture2D", asset.path);

            if (texture) {
                this.material.setTexture(textureUnit, texture);
                textureWidget.texture = this.getTextureThumbnail(texture);
            }

        }.bind(this));

    }

    // Big Texture Button(referenced texture file path in project frame)
    createTextureButtonCallback(textureUnit: number, textureWidget: EngineCore.UITextureWidget) {

        return () => {

            var texture = this.material.getTexture(textureUnit);

            if (textureWidget.getTexture() != null) {
                this.sendEvent(EngineEditor.InspectorProjectReferenceEventData({ "path": texture.getName() }));
            } else {
                this.openTextureSelectionBox(textureUnit, textureWidget);
            }

            return true;

        };

    }

    // Small Texture Button (Opens texture selection window)
    createTextureReferenceButtonCallback(textureUnit: number, textureWidget: EngineCore.UITextureWidget) {

        return () => {
            this.tunit = textureUnit;
            this.textureWidget = textureWidget;
            this.openTextureSelectionBox(textureUnit, textureWidget);
            return true;
        };
    }

    //Remove Texture Button
    createTextureRemoveButtonCallback(textureUnit: number, textureWidget: EngineCore.UITextureWidget) {

        var texture = this.material.getTexture(textureUnit);

        if (texture != null && textureWidget != null) {
            textureWidget.setTexture(null);
            this.material.setTexture(textureUnit, null);
        }

    }

    createTextureSection(): EngineCore.UISection {

        var section = new EngineCore.UISection();
        section.text = "Textures";
        section.value = 1;
        section.fontDescription = this.fd;

        var attrsVerticalLayout = new EngineCore.UILayout(EngineCore.UI_AXIS.UI_AXIS_Y);
        attrsVerticalLayout.spacing = 3;
        attrsVerticalLayout.layoutPosition = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
        attrsVerticalLayout.layoutSize = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_AVAILABLE;

        section.contentRoot.addChild(attrsVerticalLayout);

        // TODO: Filter on technique
        var textureUnits = [
            EngineCore.TextureUnit.TU_DIFFUSE, 
            EngineCore.TextureUnit.TU_NORMAL, 
            EngineCore.TextureUnit.TU_SPECULAR, 
            EngineCore.TextureUnit.TU_EMISSIVE
        ];

        for (var i in textureUnits) {

            var tunit: EngineCore.TextureUnit = textureUnits[i];

            var tunitName = EngineCore.Material.getTextureUnitName(tunit);

            var attrLayout = new EngineCore.UILayout();
            attrLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;

            var name = new EngineCore.UITextField();
            name.textAlign = EngineCore.UI_TEXT_ALIGN.UI_TEXT_ALIGN_LEFT;
            name.skinBg = "InspectorTextAttrName";

            name.text = tunitName;
            name.fontDescription = this.fd;

            attrLayout.addChild(name);

            var textureWidget = new EngineCore.UITextureWidget();

            var tlp = new EngineCore.UILayoutParams();
            tlp.width = 64;
            tlp.height = 64;
            textureWidget.layoutParams = tlp;
            textureWidget.texture = this.getTextureThumbnail(this.material.getTexture(tunit));

            var textureButton = new EngineCore.UIButton();
            textureButton.skinBg = "TBButton.flatoutline";
            textureButton["tunit"] = tunit;
            textureButton["textureWidget"] = textureWidget;

            //Create drop-down buttons to open Texture Selection Dialog Box
            var textureRefButton = new EngineCore.UIButton();
            textureRefButton.skinBg = "arrow.down";
            textureRefButton["tunit"] = tunit;
            textureRefButton["textureWidget"] = textureWidget;

            textureButton.onClick = this.createTextureButtonCallback(tunit, textureWidget);
            textureRefButton.onClick = this.createTextureReferenceButtonCallback(tunit, textureWidget);

            textureButton.contentRoot.addChild(textureWidget);

            attrLayout.addChild(textureButton);
            attrLayout.addChild(textureRefButton);

            attrsVerticalLayout.addChild(attrLayout);

            // handle dropping of texture on widget
            textureButton.subscribeToEvent(textureButton, EngineCore.DragEndedEvent((ev: EngineCore.DragEndedEvent) => {

                var importer = this.acceptAssetDrag("TextureImporter", ev);

                if (importer) {

                    var textureImporter = <ToolCore.TextureImporter>importer;
                    var asset = textureImporter.asset;

                    var texture = <EngineCore.Texture2D>EngineCore.cache.getResource("Texture2D", asset.path);

                    if (texture) {

                        this.material.setTexture(ev.target["tunit"], texture);
                        (<EngineCore.UITextureWidget>ev.target["textureWidget"]).texture = this.getTextureThumbnail(texture);

                        // note, ButtonID has been commented out because it doesn't appear to be used anywhere
                        this.sendEvent(EngineEditor.InspectorProjectReferenceEventData({ "path": texture.getName() /* "ButtonID": texture.getName() */ }));
                    }
                }
            }));

        }

        return section;

    }

    loadProjectTechniques(directory: string, menuItem: EngineCore.UIMenuItemSource) {

        var resourcePath = ToolCore.toolSystem.project.getResourcePath();
        var TechniqueAssets = ToolCore.getAssetDatabase().getFolderAssets(directory);

        for (var i = 0; i < TechniqueAssets.length; i++) {

            var asset = TechniqueAssets[i];

            if (TechniqueAssets[i].isFolder()) {

                if (this.scanDirectoryForTechniques(asset.path)) {

                    var subfoldersource = new EngineCore.UIMenuItemSource();
                    _ = new EngineCore.UIMenuItem(TechniqueAssets[i].name);
                    _.subSource = subfoldersource;
                    menuItem.addItem(_);

                    this.loadProjectTechniques(asset.path, subfoldersource);
                }
            }
            else {
                projectTechniques[i] = TechniqueAssets[i].name;
                projectTechniquesAddress[i] = TechniqueAssets[i].path;
                menuItem.addItem(new EngineCore.UIMenuItem(projectTechniques[i], projectTechniques[i]));
            }
        }
    }

    scanDirectoryForTechniques(directory: string): boolean {

        var techniqueAssets = ToolCore.getAssetDatabase().getFolderAssets(directory);

        for (var i = 0; i < techniqueAssets.length; i++) {

            var asset = techniqueAssets[i];

            if (techniqueAssets[i].isFolder()) {
                if (this.scanDirectoryForTechniques(asset.path)) {
                    return true;
                }
            }
            else if (techniqueAssets[i].getExtension() == ".xml") {
                return true;
            }
        }
        return false;
    }

    refreshTechniquesPopup() {

        techniqueSource.clear();

        _ = new EngineCore.UIMenuItem("Solid");
        _.subSource = solidSource;
        techniqueSource.addItem(_);

        _ = new EngineCore.UIMenuItem("Transparency");
        _.subSource = tranSource;
        techniqueSource.addItem(_);

        _ = new EngineCore.UIMenuItem("Lightmap");
        _.subSource = lightmapSource;
        techniqueSource.addItem(_);

        var projectTechniquesPath = ToolCore.toolSystem.project.getResourcePath() + "Techniques";

        if (EngineCore.fileSystem.dirExists(projectTechniquesPath)) {

            if (this.scanDirectoryForTechniques(projectTechniquesPath)) {

                projectSource.clear();

                _ = new EngineCore.UIMenuItem("Project");
                _.subSource = projectSource;
                techniqueSource.addItem(_);

                this.loadProjectTechniques(projectTechniquesPath, projectSource);
            }
        }
    }

    inspect(asset: ToolCore.Asset, material: EngineCore.Material) {
        // Add folders to resource directory

        this.asset = asset;
        this.material = material;

        var mlp = new EngineCore.UILayoutParams();
        mlp.width = 310;

        var materialLayout = new EngineCore.UILayout();
        materialLayout.spacing = 4;

        materialLayout.layoutDistribution   = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;
        materialLayout.layoutPosition       = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
        materialLayout.layoutParams = mlp;
        materialLayout.axis = EngineCore.UI_AXIS.UI_AXIS_Y;

        // node attr layout

        var materialSection = new EngineCore.UISection();
        materialSection.text = "Material";
        materialSection.value = 1;
        materialSection.fontDescription = this.fd;
        materialLayout.addChild(materialSection);

        var attrsVerticalLayout = new EngineCore.UILayout(EngineCore.UI_AXIS.UI_AXIS_Y);
        attrsVerticalLayout.spacing = 3;
        attrsVerticalLayout.layoutPosition  = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
        attrsVerticalLayout.layoutSize      = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_PREFERRED;

        // NAME
        var nameLayout = new EngineCore.UILayout();
        nameLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;

        var name = new EngineCore.UITextField();
        name.textAlign = EngineCore.UI_TEXT_ALIGN.UI_TEXT_ALIGN_LEFT;
        name.skinBg = "InspectorTextAttrName";

        name.text = "Name";
        name.fontDescription = this.fd;

        nameLayout.addChild(name);

        var field = new EngineCore.UIEditField();
        field.textAlign = EngineCore.UI_TEXT_ALIGN.UI_TEXT_ALIGN_LEFT;
        field.skinBg = "TBAttrEditorField";
        field.fontDescription = this.fd;
        var lp = new EngineCore.UILayoutParams();
        lp.width = 160;
        field.layoutParams = lp;

        field.text = EngineCore.splitPath(material.name).fileName;

        nameLayout.addChild(field);

        attrsVerticalLayout.addChild(nameLayout);

        // TECHNIQUE LAYOUT

        var techniqueLayout = new EngineCore.UILayout();
        techniqueLayout.layoutSize = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_GRAVITY;
        techniqueLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_PREFERRED;

        name = new EngineCore.UITextField();
        name.textAlign = EngineCore.UI_TEXT_ALIGN.UI_TEXT_ALIGN_LEFT;
        name.skinBg = "InspectorTextAttrName";

        name.text = "Technique";
        name.fontDescription = this.fd;

        techniqueLayout.addChild(name);

        var techniquePopup = this.createTechniquePopup();

        techniqueLayout.addChild(techniquePopup);

        attrsVerticalLayout.addChild(techniqueLayout);

        materialSection.contentRoot.addChild(attrsVerticalLayout);

        materialLayout.addChild(this.createTextureSection());
        materialLayout.addChild(this.createShaderParametersSection());

        var button = new EngineCore.UIButton();
        button.fontDescription = this.fd;
        button.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_RIGHT;
        button.text = "Save";

        button.onClick = function () {

            var importer = <ToolCore.MaterialImporter>this.asset.getImporter();
            importer.saveMaterial();

        }.bind(this);

        materialLayout.addChild(button);

        this.addChild(materialLayout);

    }

    techniqueButton : EngineCore.UIButton;
    material        : EngineCore.Material;
    asset           : ToolCore.Asset;
    nameTextField   : EngineCore.UITextField;
    fd              : EngineCore.UIFontDescription = new EngineCore.UIFontDescription();


}

export = MaterialInspector;
