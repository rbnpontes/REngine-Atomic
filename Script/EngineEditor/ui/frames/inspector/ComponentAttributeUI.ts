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

import EditorUI = require("ui/EditorUI");
import InspectorUtils = require("./InspectorUtils");
import AttributeInfoEdit = require("./AttributeInfoEdit");
import SerializableEditType = require("./SerializableEditType");

class LightCascadeAttributeEdit extends AttributeInfoEdit {

    splitFields: EngineCore.UIEditField[] = [];

    createEditWidget() {

        var panel = new EngineCore.UILayout();
        panel.axis              = EngineCore.UI_AXIS.UI_AXIS_Y;
        panel.layoutSize        = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_PREFERRED;
        panel.layoutPosition    = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;

        var lp = new EngineCore.UILayoutParams();
        lp.width = 180;
        panel.layoutParams = lp;

        this.editWidget = panel;

        var _this = this;

        function createHandler(index, field) {

            return function(ev: EngineCore.UIWidgetEditCompleteEvent) {

                for (var i in _this.editType.objects) {
                    var o = <EngineCore.Light>_this.editType.objects[i];
                    o.setShadowCascadeParameter(this.index, Number(this.field.text));
                }

                o.scene.sendEvent(EngineEditor.SceneEditEndEventType);
                _this.refresh();

            }.bind({ index: index, field: field });

        }

        var flp = new EngineCore.UILayoutParams();
        flp.width = 60;

        for (var i = 0; i < 4; i++) {
            var field = InspectorUtils.createAttrEditField("Split " + i, panel);
            field.layoutParams = flp;
            field.subscribeToEvent(field, EngineCore.UIWidgetEditCompleteEvent(createHandler(i, field)));
            this.splitFields.push(field);
        }

    }

    refresh() {

        // Vector 4 internally
        for (var i = 0; i < 4; i++) {

            var uniform = this.editType.getUniformValue(this.attrInfo, i);

            var field = this.splitFields[i];

            if (uniform) {

                var object = this.editType.getFirstObject();

                if (object) {
                    var value = object.getAttribute(this.attrInfo.name);
                    field.text = parseFloat(value[i].toFixed(5)).toString();
                }
                else {

                    field.text = "???";
                }
            }
            else {
                field.text = "--";
            }

        }


    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent): boolean {
        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CHANGED) {
            return true;
        }
        return false;

    }

}

interface MaterialEdit {

    index: number;
    pathReference: string;
    editField: EngineCore.UIEditField;
    selectButton: EngineCore.UIButton;

}

class SubmeshAttributeEdit extends AttributeInfoEdit {

    materialIndexes: number[] = [];

    materialEdits: { [index: number]: MaterialEdit } = {};

    mainLayout: EngineCore.UILayout;
    enabledCheckBox: EngineCore.UICheckBox;
    nameField: EngineCore.UITextField;
    name: string;
    matIndex: number;

    constructor(name: string) {

        super();
        this.name = name;
        this.hideName = true;


    }

    openResourceSelectionBox(materialIndex: number, resourceTypeName: string, importerName: string) {

        this.matIndex = materialIndex;

        EditorUI.getModelOps().showResourceSelection("Select " + resourceTypeName + " Resource", importerName, resourceTypeName, (retObject: any) => {

            if (retObject == null) {
                this.editType.onAttributeInfoEdited(this.attrInfo, null, this.matIndex);
                this.refresh();
                return;
            }

            var resource: EngineCore.Resource = null;

            if (retObject instanceof ToolCore.Asset) {

                resource = (<ToolCore.Asset>retObject).getResource(resourceTypeName);

            } else if (retObject instanceof EngineCore.Resource) {

                resource = <EngineCore.Resource>retObject;

            }

            this.sendEvent(EngineEditor.InspectorProjectReferenceEventData({ "path": resource.name }));
            this.editType.onAttributeInfoEdited(this.attrInfo, resource, materialIndex);
            this.refresh();

        });

    }

    createMaterialEdit(materialIndex: number) {

        var o = InspectorUtils.createAttrEditFieldWithSelectButton("Material", this.mainLayout);

        var lp = new EngineCore.UILayoutParams();
        lp.width = 140;
        lp.height = 24;
        o.editField.layoutParams = lp;
        o.editField.readOnly = true;

        var selectButton = o.selectButton;

        var materialEdit: MaterialEdit = { index: materialIndex, pathReference: "" , editField: o.editField, selectButton: selectButton };
        this.materialEdits[materialIndex] = materialEdit;

        var resourceTypeName = "Material";
        var importerName = ToolCore.assetDatabase.getResourceImporterName(resourceTypeName);


        selectButton.onClick = () => {

            this.openResourceSelectionBox(materialIndex, resourceTypeName, importerName);
           // this.sendEvent(EditorEvents.InspectorProjectReference, { "path": pathName });

        };

        // handle dropping of component on field
        o.editField.subscribeToEvent(o.editField, EngineCore.DragEndedEvent((ev: EngineCore.DragEndedEvent) => {

            if (ev.target == o.editField) {

                var dragObject = ev.dragObject;

                var importer;

                if (dragObject.object && dragObject.object.typeName == "Asset") {

                    var asset = <ToolCore.Asset>dragObject.object;

                    if (asset.importerTypeName == importerName) {
                        importer = asset.importer;
                    }

                }

                if (importer) {

                    var resource = asset.getResource(resourceTypeName);
                    this.sendEvent(EngineEditor.InspectorProjectReferenceEventData({ "path": resource.name }));
                    this.editType.onAttributeInfoEdited(this.attrInfo, resource, materialIndex);
                    this.refresh();
                }
            }

        }));

        o.editField.subscribeToEvent(o.editField, EngineCore.UIWidgetEvent((ev: EngineCore.UIWidgetEvent) => {

            if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_POINTER_DOWN && o.editField.text != "") {

                var pathName = materialEdit.pathReference;
                this.sendEvent(EngineEditor.InspectorProjectReferenceEventData({ "path": pathName }));

            } else if (o.editField.text == "") {

                this.openResourceSelectionBox(materialIndex, resourceTypeName, importerName);
            }

        }));
    }

    createEditWidget() {

        var mainLayout = this.mainLayout = new EngineCore.UILayout();
        mainLayout.axis = EngineCore.UI_AXIS.UI_AXIS_Y;
        mainLayout.layoutSize = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_AVAILABLE;
        mainLayout.layoutPosition = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
        mainLayout.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_LEFT_RIGHT;
        mainLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;

        var cb = InspectorUtils.createAttrCheckBox(this.name, mainLayout);
        this.enabledCheckBox = cb.checkBox;

        cb.checkBox.subscribeToEvent(cb.checkBox, EngineCore.UIWidgetEvent((ev: EngineCore.UIWidgetEvent) => {

            if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CHANGED) {

                var scene: EngineCore.Scene;

                for (var i in this.editType.objects) {

                    var staticModel = <EngineCore.StaticModel>this.editType.objects[i];

                    scene = staticModel.scene;

                    if (cb.checkBox.value) {

                        staticModel.showGeometry(this.name);

                    } else {

                        staticModel.hideGeometry(this.name);

                    }

                }

                scene.sendEvent(EngineEditor.SceneEditEndEventType);

                return true;

            }

            return false;


        }));


        this.nameField = cb.textField;

        this.editWidget = mainLayout;
    }

    refresh() {

        var editType = this.editType;

        var object = this.editType.getFirstObject();

        if (!object) {
            this.visibility = EngineCore.UI_WIDGET_VISIBILITY.UI_WIDGET_VISIBILITY_GONE;
            return;
        }

        this.visibility = EngineCore.UI_WIDGET_VISIBILITY.UI_WIDGET_VISIBILITY_VISIBLE;

        var enabled = (<EngineCore.StaticModel>object).getGeometryVisible(this.name);
        var mixed = false;
        for (var i in editType.objects) {

            object = editType.objects[i];
            var _enabled = (<EngineCore.StaticModel>object).getGeometryVisible(this.name);
            if (_enabled != enabled) {
                mixed = true;
                break;
            }
        }

        if (mixed) {

            this.enabledCheckBox.skinBg = "TBGreyCheckBoxNonUniform";
            this.enabledCheckBox.value = 1;

        } else {

            this.enabledCheckBox.skinBg = "TBCheckBox";
            this.enabledCheckBox.value = enabled ? 1 : 0;

        }

        for (var i in this.materialIndexes) {

            var idx = this.materialIndexes[i];

            if (!this.materialEdits[idx]) {
                this.createMaterialEdit(idx);
            }

            var matEdit = this.materialEdits[idx];

            var uniform = editType.getUniformValue(this.attrInfo, matEdit.index);

            if (uniform) {

                var object = editType.getFirstObject();

                if (object) {

                    // for cached resources, use the asset name, otherwise use the resource path name
                    var resource: EngineCore.Resource;
                    if (matEdit.index != -1) {
                        resource = object.getAttribute(this.attrInfo.name).resources[matEdit.index];
                    } else {
                        resource = <EngineCore.Resource>object.getAttribute(this.attrInfo.name);
                    }

                    var text = "";

                    if (resource) {

                        if (resource instanceof EngineCore.Material) {

                            text = (<EngineCore.Material>resource).name;

                        } else {
                            text = "???";
                        }
                    }

                    var pathinfo = EngineCore.splitPath(text);
                    matEdit.editField.text = pathinfo.fileName;
                    matEdit.pathReference = text;
                }


            } else {
                matEdit.editField.text = "--";
            }

        }

    }

}

class SubmeshListAttributeEdit extends AttributeInfoEdit {

    layout: EngineCore.UILayout;

    submeshEdits: { [name: string]: SubmeshAttributeEdit } = {};

    constructor() {

        super();

        this.hideName = true;

    }

    createEditWidget() {

        this.spacing = 0;

        var layout = this.layout = new EngineCore.UILayout();

        layout.axis = EngineCore.UI_AXIS.UI_AXIS_Y;
        layout.spacing = 2;
        layout.layoutSize = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_AVAILABLE;
        layout.layoutPosition = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
        layout.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_LEFT_RIGHT;
        layout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;

        var lp = new EngineCore.UILayoutParams();
        lp.width = 304;
        layout.layoutParams = lp;

        var name = new EngineCore.UITextField();
        name.textAlign = EngineCore.UI_TEXT_ALIGN.UI_TEXT_ALIGN_LEFT;
        name.skinBg = "InspectorTextAttrName";
        name.layoutParams = AttributeInfoEdit.attrNameLP;
        name.text = "Submeshes";

        layout.addChild(name);

        InspectorUtils.createSeparator(layout);

        this.editWidget = layout;

    }

    refresh() {

        var editType = this.editType;

        var object = this.editType.getFirstObject();

        if (!object) {
            this.visibility = EngineCore.UI_WIDGET_VISIBILITY.UI_WIDGET_VISIBILITY_GONE;
            return;
        }

        this.visibility = EngineCore.UI_WIDGET_VISIBILITY.UI_WIDGET_VISIBILITY_VISIBLE;

        var name: string;
        for (var i in editType.objects) {

            // TODO: check for multiple selection with different models and display multiple selection info box

            var staticModel = <EngineCore.StaticModel>editType.objects[i];
            var model = staticModel.model;

            if (!model)
                continue;

            for (var j = 0; j < model.numGeometries; j++) {

                name = model.getGeometryName(j);

                if (!name)
                    name = "Mesh";

                if (!this.submeshEdits.hasOwnProperty(name)) {

                    var submeshEdit = new SubmeshAttributeEdit(name);
                    submeshEdit.initialize(this.editType, this.attrInfo);
                    this.layout.addChild(submeshEdit);
                    this.submeshEdits[name] = submeshEdit;
                    InspectorUtils.createSeparator(this.layout);
                }

                this.submeshEdits[name].materialIndexes.push(j);
            }

        }

        for (name in this.submeshEdits)
            this.submeshEdits[name].refresh();

    }

}

AttributeInfoEdit.registerCustomAttr("AnimatedModel", "Material", SubmeshListAttributeEdit);
AttributeInfoEdit.registerCustomAttr("StaticModel", "Material", SubmeshListAttributeEdit);
AttributeInfoEdit.registerCustomAttr("Skybox", "Material", SubmeshListAttributeEdit);
AttributeInfoEdit.registerCustomAttr("Light", "CSM Splits", LightCascadeAttributeEdit);
