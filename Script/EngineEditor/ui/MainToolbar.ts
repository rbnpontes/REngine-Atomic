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

import EditorUI = require("./EditorUI");

class MainToolbar extends EngineCore.UIWidget {

    translateButton: EngineCore.UIButton;
    rotateButton: EngineCore.UIButton;
    scaleButton: EngineCore.UIButton;
    axisButton: EngineCore.UIButton;
    playButton: EngineCore.UIButton;
    pauseButton: EngineCore.UIButton;
    stepButton: EngineCore.UIButton;

    constructor(parent: EngineCore.UIWidget) {

        super();

        this.load("editor/ui/maintoolbar.tb.txt");

        this.translateButton = <EngineCore.UIButton>this.getWidget("3d_translate");
        this.rotateButton = <EngineCore.UIButton>this.getWidget("3d_rotate");
        this.scaleButton = <EngineCore.UIButton>this.getWidget("3d_scale");

        this.axisButton = <EngineCore.UIButton>this.getWidget("3d_axismode");
        this.playButton = <EngineCore.UIButton>this.getWidget("maintoolbar_play");
        this.pauseButton = <EngineCore.UIButton>this.getWidget("maintoolbar_pause");
        this.stepButton = <EngineCore.UIButton>this.getWidget("maintoolbar_step");

        this.translateButton.value = 1;

        parent.addChild(this);

        this.subscribeToEvent(EngineEditor.GizmoAxisModeChangedEvent((ev) => this.handleGizmoAxisModeChanged(ev)));
        this.subscribeToEvent(EngineEditor.GizmoEditModeChangedEvent((ev) => this.handleGizmoEditModeChanged(ev)));

        this.subscribeToEvent(this, EngineCore.UIWidgetEvent((data) => this.handleWidgetEvent(data)));

        this.subscribeToEvent(EngineEditor.EditorPlayerStartedEvent(() => {
            var skin = <EngineCore.UISkinImage> this.playButton.getWidget("skin_image");
            skin.setSkinBg("StopButton");
            skin = <EngineCore.UISkinImage> this.pauseButton.getWidget("skin_image");
            skin.setSkinBg("PauseButton");
        }));
        this.subscribeToEvent(EngineEditor.EditorPlayerStoppedEvent(() => {
            var skin = <EngineCore.UISkinImage> this.playButton.getWidget("skin_image");
            skin.setSkinBg("PlayButton");
            skin = <EngineCore.UISkinImage> this.pauseButton.getWidget("skin_image");
            skin.setSkinBg("PauseButton");
        }));
        this.subscribeToEvent(EngineEditor.EditorPlayerPausedEvent(() => {
            var skin = <EngineCore.UISkinImage> this.pauseButton.getWidget("skin_image");
            skin.setSkinBg("PlayButton");
        }));

        this.subscribeToEvent(EngineEditor.EditorPlayerResumedEvent(() => {
            var skin = <EngineCore.UISkinImage> this.pauseButton.getWidget("skin_image");
            skin.setSkinBg("PauseButton");
        }));

        // TODO: We need better control over playmode during NET compiles
        this.subscribeToEvent(ToolCore.NETBuildBeginEvent((data) => {
            this.playButton.disable();
        }));

        this.subscribeToEvent(ToolCore.NETBuildResultEvent((data) => {
            this.playButton.enable();
        }));

    }

    handleGizmoAxisModeChanged(ev: EngineEditor.GizmoAxisModeChangedEvent) {
        if (ev.mode) {
            this.axisButton.value = 0;
            this.axisButton.text = "Local";
        } else {
            this.axisButton.value = 1;
            this.axisButton.text = "World";
        }
    }

    handleGizmoEditModeChanged(ev: EngineEditor.GizmoEditModeChangedEvent) {

        this.translateButton.value = 0;
        this.rotateButton.value = 0;
        this.scaleButton.value = 0;

        switch (ev.mode) {
            case 1:
                this.translateButton.value = 1;
                break;
            case 2:
                this.rotateButton.value = 1;
                break;
            case 3:
                this.scaleButton.value = 1;
                break;
        }

    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK && ev.target) {

            if (ev.target.id == "3d_translate" || ev.target.id == "3d_rotate" || ev.target.id == "3d_scale") {

                var mode = 1;
                if (ev.target.id == "3d_rotate")
                    mode = 2;
                else if (ev.target.id == "3d_scale")
                    mode = 3;

                this.sendEvent(EngineEditor.GizmoEditModeChangedEventData({ mode: mode }));

                return true;

            } else if (ev.target.id == "3d_axismode") {

                EditorUI.getShortcuts().toggleGizmoAxisMode();
                return true;

            } else if (ev.target.id == "maintoolbar_play") {
                EditorUI.getShortcuts().invokePlayOrStopPlayer();
                return true;

            } else if (ev.target.id == "maintoolbar_pause") {
                EditorUI.getShortcuts().invokePauseOrResumePlayer();
                return true;

            } else if (ev.target.id == "maintoolbar_step") {
                EditorUI.getShortcuts().invokeStepPausedPlayer();
                return true;
            }

        }

    }

}



export = MainToolbar;
