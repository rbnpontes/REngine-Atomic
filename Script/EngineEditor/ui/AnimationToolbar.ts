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
import HierarchyFrame = require("ui/frames/HierarchyFrame");
import InspectorUtils = require("ui/frames/inspector/InspectorUtils");
import ResourceOps = require("resources/ResourceOps");

class AnimationToolbar extends EngineCore.UIWidget {

    updateDelta: number = 0.0;
    updateYaw: number = 0.0;

    constructor(parent: EngineCore.UIWidget, properties: EngineCore.UIWidget, asset: ToolCore.Asset) {

        super();

        this.load("editor/ui/animationtoolbar.tb.txt");
        this.asset = asset;

        this.leftAnimContainer = <EngineCore.UILayout>this.getWidget("leftanimcontainer");
        this.rightAnimContainer = <EngineCore.UILayout>this.getWidget("rightanimcontainer");

        this.subscribeToEvent(this, EngineCore.UIWidgetEvent((ev) => this.handleWidgetEvent(ev)));
        this.subscribeToEvent(EngineEditor.EditorActiveSceneEditorChangeEvent((data) => this.handleActiveSceneEditorChanged(data)));
        this.subscribeToEvent(EngineEditor.EditorSceneClosedEvent((data) => this.handleSceneClosed(data)));

        var leftAnimationField = InspectorUtils.createAttrEditFieldWithSelectButton("Animation A", this.leftAnimContainer);
        leftAnimationField.selectButton.onClick = function () { this.openAnimationSelectionBox(leftAnimationField.editField, this.leftAnim); }.bind(this);

        var rightAnimationField = InspectorUtils.createAttrEditFieldWithSelectButton("Animation B", this.rightAnimContainer);
        rightAnimationField.selectButton.onClick = function () { this.openAnimationSelectionBox(rightAnimationField.editField, this.rightAnim); }.bind(this);

        this.leftAnimEditfield = leftAnimationField.editField;
        this.rightAnimEditfield = rightAnimationField.editField;

        var leftStateContainer = <EngineCore.UILayout>this.getWidget("leftstatedropdown");
        var rightStateContainer = <EngineCore.UILayout>this.getWidget("rightstatedropdown");

        ResourceOps.CreateNewAnimationPreviewScene();
        this.populateScene();

        parent.addChild(this);

        //Animation properties bar
        this.animationPropertiesContainer = new EngineCore.UILayout();

        this.animationSpeed = InspectorUtils.createAttrEditField("Playback Speed:", this.animationPropertiesContainer);
        this.animationSpeed.setAdaptToContentSize(true);

        this.blendSpeed = InspectorUtils.createAttrEditField("Blend Speed:", this.animationPropertiesContainer);
        this.blendSpeed.setAdaptToContentSize(true);

        var attrLayout = new EngineCore.UILayout();
        attrLayout.layoutSize = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_AVAILABLE;
        attrLayout.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_LEFT_RIGHT;
        attrLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;
        var nameField = new EngineCore.UITextField();
        nameField.textAlign = EngineCore.UI_TEXT_ALIGN.UI_TEXT_ALIGN_RIGHT;
        nameField.skinBg = "InspectorTextAttrName";
        nameField.text = "Spin Speed:";
        attrLayout.addChild(nameField);
        this.rotateModel = new EngineCore.UISlider();
        this.rotateModel.setLimits(0, 10);
        this.rotateModel.setValue(0);
        attrLayout.addChild(this.rotateModel);
        this.animationPropertiesContainer.addChild( attrLayout );

        //Set default values
        this.animationSpeed.setText("1");
        this.blendSpeed.setText("0");

        properties.addChild(this.animationPropertiesContainer);

        this.subscribeToEvent(EngineCore.UpdateEvent((ev) => this.handleUpdate(ev))); // if we want the model to rotate

    }

    handleUpdate(ev) {

        var rotspeed = this.rotateModel.value;
        if (rotspeed > 0) {                  // only do work if the spinning is turned on
            this.updateDelta += ev.timeStep; // add some time to the clock
            if (this.updateDelta > (0.134 * (10 - rotspeed))) {  //see if we have reached our limit
                this.updateDelta = 0.0;      // reset the limit
                this.updateYaw += 2.1;       // increase the yaw
                if (this.updateYaw > 26.3)   // clamp the yaw
                    this.updateYaw = 0;
                this.modelNode.yaw(this.updateYaw); // and rotate the model.
            }

        }
    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent): boolean {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {
            if (this.animationController != null) {

                if (ev.target.id == "play_left") {
                    if (this.animationController.playExclusive(this.leftAnimEditfield.text, 0, true))
                        this.animationController.setSpeed(this.leftAnimEditfield.text, Number(this.animationSpeed.text));
                    else
                        this.showAnimationWarning();

                    return true;
                }
                if (ev.target.id == "play_right") {
                    if (this.animationController.playExclusive(this.rightAnimEditfield.text, 0, true))
                        this.animationController.setSpeed(this.rightAnimEditfield.text, Number(this.animationSpeed.text));
                    else
                        this.showAnimationWarning();

                    return true;
                }
                if (ev.target.id == "blend_left") {
                    if (this.animationController.playExclusive(this.leftAnimEditfield.text, 0, true, Number(this.blendSpeed.text)))
                        this.animationController.setSpeed(this.leftAnimEditfield.text, Number(this.animationSpeed.text));
                    else
                        this.showAnimationWarning();

                    return true;
                }
                if (ev.target.id == "blend_right") {
                    if (this.animationController.playExclusive(this.rightAnimEditfield.text, 0, true, Number(this.blendSpeed.text)))
                        this.animationController.setSpeed(this.rightAnimEditfield.text, Number(this.animationSpeed.text));
                    else
                        EditorUI.getModelOps().showError("Animation Toolbar Warning", "The animation cannot be played. Please make sure the animation you are trying to play exists in the AnimationController Component.");

                    return true;
                }
                if (ev.target.id == "stop") {
                    this.animationController.stopAll();
                    return true;
                }
            }
        }
        return true;
    }

    handleSceneClosed(ev: EngineEditor.EditorSceneClosedEvent) {
        if (ev.scene == this.scene) {
            EngineCore.fileSystem.delete(this.sceneAssetPath);

            if (this.animationPropertiesContainer)
                this.animationPropertiesContainer.remove();

            this.remove();
        }
    }

    closeViewer() {
        EngineCore.fileSystem.delete(this.sceneAssetPath);
            this.sceneEditor.close();
            if (this.animationPropertiesContainer)
                this.animationPropertiesContainer.remove();
            this.remove();
    }

    handleActiveSceneEditorChanged(event: EngineEditor.EditorActiveSceneEditorChangeEvent) {

        if (!event.sceneEditor)
            return;

        this.sceneEditor = event.sceneEditor;
        this.scene = event.sceneEditor.scene;

        if (this.scene) {
            this.unsubscribeFromEvents(this.scene);
            return;
        }

        if (!event.sceneEditor)
            return;
    }

    populateScene() {

        this.scene.setUpdateEnabled(true);

        var modelNode = this.asset.instantiateNode(this.scene, this.asset.name);
        this.modelNode = modelNode;

        this.sceneEditor.selection.addNode(modelNode, true);
        this.sceneEditor.sceneView3D.frameSelection();

        this.animatedModel = <EngineCore.AnimatedModel>modelNode.getComponent("AnimatedModel");
        this.animationController = <EngineCore.AnimationController>modelNode.getComponent("AnimationController");
        if ( this.animatedModel != null && this.animationController != null ) {
            var model = this.animatedModel.model;
            this.animatedModel.setBoneCreationOverride(true);
            this.animatedModel.setModel(model, true);

            var animComp = new EngineCore.AnimatedModel();
            var animContComp = new EngineCore.AnimationController();
        }
    }

    openAnimationSelectionBox(animationWidget: EngineCore.UIEditField, animationSlot: EngineCore.Animation) {

        EditorUI.getModelOps().showResourceSelection("Select Animation", "ModelImporter", "Animation", function (resource: EngineCore.Animation, args: any) {
            var animation = resource;
            if (animation) {
                animationSlot = animation;
                animationWidget.text = animation.getAnimationName();
            }
        });
    }
    showAnimationWarning() {
        EditorUI.getModelOps().showError("Animation Preview Warning", "The animation cannot be played. Please make sure the animation you are trying to play exists in the AnimationController Component.");
    }

    //Animation Toolbar Widgets
    animationController: EngineCore.AnimationController;
    animatedModel: EngineCore.AnimatedModel;
    scene: EngineCore.Scene = null;
    sceneEditor: EngineEditor.SceneEditor3D;
    modelNode: EngineCore.Node;

    leftAnimContainer: EngineCore.UILayout;
    rightAnimContainer: EngineCore.UILayout;
    blendFileContainer: EngineCore.UILayout;
    leftAnimEditfield: EngineCore.UIEditField;
    rightAnimEditfield: EngineCore.UIEditField;

    leftAnim: EngineCore.Animation;
    rightAnim: EngineCore.Animation;

    asset: ToolCore.Asset;
    sceneAssetPath: string;
    stateDropDownList: string[];
    //Animation Properties Widgets
    animationPropertiesContainer: EngineCore.UILayout;
    animationSpeed: EngineCore.UIEditField;
    blendSpeed: EngineCore.UIEditField;
    rotateModel: EngineCore.UISlider;
}

export = AnimationToolbar;
