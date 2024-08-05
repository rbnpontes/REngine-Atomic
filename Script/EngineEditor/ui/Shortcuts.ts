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
import Preferences = require("editor/Preferences");
import * as EditorEvents from "../editor/EditorEvents";

class Shortcuts extends EngineCore.ScriptObject {

    constructor() {

        super();

        this.subscribeToEvent(EngineCore.UIShortcutEvent((ev) => this.handleUIShortcut(ev)));
        this.subscribeToEvent(EngineCore.KeyDownEvent((ev) => this.handleKeyDown(ev)));
    }

    //this should be moved somewhere else...
    invokePlayOrStopPlayer(debug: boolean = false) {

        this.sendEvent(EngineEditor.EditorSaveAllResourcesEventType);

        if (EngineCore.editorMode.isPlayerEnabled()) {
            this.sendEvent(EditorEvents.IPCPlayerExitRequestEventType);
        } else {

            var playerWindow = Preferences.getInstance().playerWindow;

            if (playerWindow) {

                if ((playerWindow.monitor + 1) > EngineCore.graphics.getNumMonitors()) {
                    //will use default settings if monitor is not available
                    var args = "--resizable";
                    EngineCore.editorMode.playProject(args, debug);

                } else {

                    if (playerWindow.width == 0 || playerWindow.height == 0) {

                        playerWindow.width = EngineCore.graphics.width * .75;
                        // 16:9
                        playerWindow.height = playerWindow.width * .56;

                        let pos = EngineCore.graphics.windowPosition;

                        playerWindow.x = pos[0] + (EngineCore.graphics.width / 2 - playerWindow.width / 2);
                        playerWindow.y = pos[1] + (EngineCore.graphics.height / 2 - playerWindow.height / 2);

                        // if too small a window, use default (which maximizes)
                        if (playerWindow.width < 480) {
                            playerWindow.width = playerWindow.height = playerWindow.x = playerWindow.y = 0;
                        }

                    }

                    var args = "--windowposx " + playerWindow.x + " --windowposy " + playerWindow.y + " --windowwidth " + playerWindow.width + " --windowheight " + playerWindow.height + " --resizable" + " --fromEditorPlay";

                    if (playerWindow.maximized) {
                        args += " --maximize";
                    }

                    EngineCore.editorMode.playProject(args, debug);
                }

            } else {
                EngineCore.editorMode.playProject("", debug);
            }
        }
    }

    invokePauseOrResumePlayer() {
        if (EngineCore.editorMode.isPlayerEnabled()) {
            this.sendEvent(EditorEvents.IPCPlayerPauseResumeRequestEventType);
        }
    }

    invokeStepPausedPlayer() {
        if (EngineCore.editorMode.isPlayerEnabled()) {
            this.sendEvent(EditorEvents.IPCPlayerPauseStepRequestEventType);
        }
    }

    invokeFormatCode() {

        var editor = EditorUI.getCurrentResourceEditor();

        if (editor && editor.typeName == "JSResourceEditor") {

            (<EngineEditor.JSResourceEditor>editor).formatCode();

        }

    }

    invokeFileClose() {
        this.invokeResourceFrameShortcut("close");
    }

    invokeFileSave() {
        this.sendEvent(EngineEditor.EditorSaveResourceEventType);
    }

    invokeUndo() {
        this.invokeResourceFrameShortcut("undo");
    }

    invokeRedo() {
        this.invokeResourceFrameShortcut("redo");
    }

    invokeCut() {
        this.invokeResourceFrameShortcut("cut");
    }

    invokeCopy() {
        this.invokeResourceFrameShortcut("copy");
    }

    invokePaste() {
        this.invokeResourceFrameShortcut("paste");
    }

    invokeFrameSelected() {
        this.invokeResourceFrameShortcut("frameselected");
    }


    invokeSelectAll() {
        this.invokeResourceFrameShortcut("selectall");
    }

    invokeGizmoEditModeChanged(mode: EngineEditor.EditMode) {

        this.sendEvent(EngineEditor.GizmoEditModeChangedEventData({ mode: mode }));

    }

    toggleGizmoAxisMode() {
        var editor = EditorUI.getCurrentResourceEditor();

        if (editor && editor instanceof EngineEditor.SceneEditor3D) {
            var mode = editor.getGizmo().axisMode ? EngineEditor.AxisMode.AXIS_WORLD : EngineEditor.AxisMode.AXIS_LOCAL;
            this.sendEvent(EngineEditor.GizmoAxisModeChangedEventData({ mode: mode }));
        }
    }

    invokeResourceFrameShortcut(shortcut: EngineEditor.EditorShortcutType) {
        if (!ToolCore.toolSystem.project) return;
        var resourceFrame = EditorUI.getMainFrame().resourceframe.currentResourceEditor;
        if (resourceFrame) {
            resourceFrame.invokeShortcut(shortcut);
        }
    }

    invokeScreenshot() {
        var features = Preferences.getInstance().editorFeatures; // get prefs
        var pic_ext = features.screenshotFormat;
        var pic_path = features.screenshotPath;
        var dx = new Date();  // get the date NOW
        var datestring = dx.getFullYear() + "_" + ("0" + (dx.getMonth() + 1 )).slice(-2) + "_"  + ("0" + dx.getDate()).slice(-2)
            + "_" + ("0" + dx.getHours()).slice(-2) + "_" + ("0" + dx.getMinutes()).slice(-2) + "_" + ("0" + dx.getSeconds()).slice(-2);
        pic_path += "/Screenshot_" + datestring + "." + pic_ext;  // form filename
        var myimage = new EngineCore.Image; // make an image to save
        if (EngineCore.graphics.takeScreenShot(myimage)) { // take the screenshot
            var saved_pic = false;
            var jpgquality = 92; // very good quality jpeg 
            if ( pic_ext == "png" ) saved_pic = myimage.savePNG(pic_path);
            else if ( pic_ext == "jpg" ) saved_pic = myimage.saveJPG(pic_path, jpgquality);
            else if ( pic_ext == "tga" ) saved_pic = myimage.saveTGA(pic_path);
            else if ( pic_ext == "bmp" ) saved_pic = myimage.saveBMP(pic_path);
            else if ( pic_ext == "dds" ) saved_pic = myimage.saveDDS(pic_path);
            if (saved_pic)  EditorUI.showEditorStatus ( "Saved screenshot " + pic_path );
            else EditorUI.showEditorStatus ( "Error - could not save screenshot " + pic_path );
        }
        else EditorUI.showEditorStatus ( "Error - could not take screenshot.");
    }

    handleKeyDown(ev: EngineCore.KeyDownEvent) {

        // if the right mouse buttons isn't down
        if (!(ev.buttons & EngineCore.MOUSEB_RIGHT)) {

            // TODO: Make these customizable

            if (!EngineCore.ui.focusedWidget && !this.cmdKeyDown()) {

                if (ev.key == EngineCore.KEY_ESCAPE) {

                    if (EngineCore.ui.consoleIsVisible) {
                        EngineCore.ui.showConsole(false);
                    }
                }

                if (ev.key == EngineCore.KEY_W) {
                    this.invokeGizmoEditModeChanged(EngineEditor.EditMode.EDIT_MOVE);
                } else if (ev.key == EngineCore.KEY_E) {
                    this.invokeGizmoEditModeChanged(EngineEditor.EditMode.EDIT_ROTATE);
                } else if (ev.key == EngineCore.KEY_R) {
                    this.invokeGizmoEditModeChanged(EngineEditor.EditMode.EDIT_SCALE);
                } else if (ev.key == EngineCore.KEY_X) {
                    this.toggleGizmoAxisMode();
                } else if (ev.key == EngineCore.KEY_F) {
                    this.invokeFrameSelected();
                }
            }

        }

    }

    cmdKeyDown(): boolean {

        var cmdKey;
        if (EngineCore.platform == "MacOSX") {
            cmdKey = (EngineCore.input.getKeyDown(EngineCore.KEY_LGUI) || EngineCore.input.getKeyDown(EngineCore.KEY_RGUI));
        } else {
            cmdKey = (EngineCore.input.getKeyDown(EngineCore.KEY_LCTRL) || EngineCore.input.getKeyDown(EngineCore.KEY_RCTRL));
        }

        return cmdKey;


    }

    // global shortcut handler
    handleUIShortcut(ev: EngineCore.UIShortcutEvent) {

        var cmdKey = this.cmdKeyDown();

        if ( !cmdKey && ev.qualifiers > 0 ) { // check the event, the qualifier may have been programmitically set
            cmdKey = ( ev.qualifiers == EngineCore.QUAL_CTRL );
        }

        if (cmdKey) {

            if (ev.key == EngineCore.KEY_S) {
                this.invokeFileSave();
            }
            else if (ev.key == EngineCore.KEY_W) {
                this.invokeFileClose();
            }
            else if (ev.key == EngineCore.KEY_I) {
                this.invokeFormatCode();
            }
            else if (ev.key == EngineCore.KEY_P) {
                this.invokePlayOrStopPlayer();
            } else if (ev.key == EngineCore.KEY_B) {
                if (ev.qualifiers & EngineCore.QUAL_SHIFT) {
                    EditorUI.getModelOps().showBuildSettings();
                } else {
                    EditorUI.getModelOps().showBuild();
                }
            }
            else if (ev.key == EngineCore.KEY_U) {
                if (ev.qualifiers & EngineCore.QUAL_SHIFT) {
                    this.invokeStepPausedPlayer();
                } else {
                    this.invokePauseOrResumePlayer();
                }
            }
            else if (ev.key == EngineCore.KEY_9) {
                this.invokeScreenshot();
            }

        }

    }

}

export = Shortcuts;
