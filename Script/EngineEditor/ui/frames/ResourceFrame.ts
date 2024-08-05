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
import ResourceEditorProvider from "../ResourceEditorProvider";

// the root content of editor widgets (rootContentWidget property) are extended with an editor field
// so we can access the editor they belong to from the widget itself
interface EditorRootContentWidget extends EngineCore.UIWidget {
    editor: EngineEditor.ResourceEditor;
}

class ResourceFrame extends ScriptWidget {

    tabcontainer            : EngineCore.UITabContainer;
    resourceLayout          : EngineCore.UILayout;
    resourceViewContainer   : EngineCore.UILayout;
    currentResourceEditor   : EngineEditor.ResourceEditor;
    wasClosed: boolean;
    resourceEditorProvider: ResourceEditorProvider;

    // editors have a rootCotentWidget which is what is a child of the tab container

    // editors can be looked up by the full path of what they are editing
    editors: { [path: string]: EngineEditor.ResourceEditor; } = {};

    show(value: boolean) {

        if (value) {

        }

    }

    handleSaveResource(ev: EngineEditor.EditorSaveResourceEvent) {

        if (this.currentResourceEditor) {
            this.currentResourceEditor.save();
            // Grab the path to this file and pass it to the save resource
            this.sendEvent(EngineEditor.EditorSaveResourceNotificationEventData({
                path: ev.path || this.currentResourceEditor.fullPath
            }));
        }

    }

    handleDeleteResource(ev: EngineEditor.EditorDeleteResourceEvent) {
        var editor = this.editors[ev.path];
        if (editor) {
            editor.close(true);
            delete this.editors[ev.path];
        }
    }

    handleSaveAllResources() {

        for (var i in this.editors) {
            this.editors[i].save();
            this.sendEvent(EngineEditor.EditorSaveResourceNotificationEventData({
                path: this.editors[i].fullPath
            }));
        }

    }

    handleEditResource(ev: EngineEditor.EditorEditResourceEvent) {

        var path = ev.path;

        if (this.editors[path]) {

            this.navigateToResource(path, ev.lineNumber);

            return;

        }

        var editor = this.resourceEditorProvider.getEditor(ev.path, this.tabcontainer, ev.lineNumber);
        if (editor) {

            // cast and add editor lookup on widget itself
            (<EditorRootContentWidget> editor.rootContentWidget).editor = editor;

            this.editors[path] = editor;
            this.tabcontainer.currentPage = this.tabcontainer.numPages - 1;
            editor.setFocus();
        }


    }

    navigateToResource(fullpath: string, lineNumber = -1, tokenPos: number = -1) {
        if (this.wasClosed) return;

        if (!this.editors[fullpath]) {
            return;
        }

        var editor = this.editors[fullpath];

        if (this.currentResourceEditor == editor) {
            this.setCursorPositionInEditor(editor, lineNumber, tokenPos);
            return;
        }

        var root = this.tabcontainer.contentRoot;

        var i = 0;

        for (var child = root.firstChild; child; child = child.next, i++) {
            if (editor.rootContentWidget == child) {
                break;
            }
        }

        if (i < this.tabcontainer.numPages) {

            this.tabcontainer.currentPage = i;

            editor.setFocus();

            this.setCursorPositionInEditor(editor, lineNumber, tokenPos);
        }

    }

    /**
     * 
     * Set the cursor to the correct line number in the editor
     * @param {Editor.ResourceEditor} editor
     * @param {any} [lineNumber=-1]
     * @param {any} [tokenPos=-1]
     * 
     * @memberOf ResourceFrame
     */
    setCursorPositionInEditor(editor: EngineEditor.ResourceEditor, lineNumber = -1, tokenPos = -1) {
        if (editor instanceof EngineEditor.JSResourceEditor) {
            if (lineNumber != -1) {
                (<EngineEditor.JSResourceEditor>editor).gotoLineNumber(lineNumber);
            }

            if (tokenPos != -1) {
                (<EngineEditor.JSResourceEditor>editor).gotoTokenPos(tokenPos);
            }
        }
    }

    handleCloseResource(ev: EngineEditor.EditorResourceCloseEvent) {
        this.wasClosed = false;
        var editor = ev.editor;
        var navigate = ev.navigateToAvailableResource;

        if (!editor)
            return;

        editor.unsubscribeFromAllEvents();

        (<EditorRootContentWidget> editor.rootContentWidget).editor = null;

        var editors = Object.keys(this.editors);

        var closedIndex = editors.indexOf(editor.fullPath);

        // remove from lookup
        delete this.editors[editor.fullPath];

        var root = this.tabcontainer.contentRoot;

        root.removeChild(editor.rootContentWidget);

        if (editor != this.currentResourceEditor) {
            this.wasClosed = true;
            return;
        } else {
            this.currentResourceEditor = null;
            this.tabcontainer.currentPage = -1;
        }

        if (navigate) {
            var nextEditor = editors[closedIndex + 1];
            if (nextEditor) {
                this.navigateToResource(nextEditor);
            } else {
                this.navigateToResource(editors[closedIndex - 1]);
            }
        }

    }

    handleResourceEditorChanged(data: EngineEditor.EditorResourceEditorChangedEvent) {

        var editor = data.resourceEditor;
        this.currentResourceEditor = editor;

    }

    handleRenameResource(ev:EngineEditor.EditorRenameResourceNotificationEvent) {
        var editor = this.editors[ev.path];
        if (editor) {
            this.editors[ev.newPath] = editor;
            delete this.editors[ev.path];
        }
    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_TAB_CHANGED && ev.target == this.tabcontainer) {

            var w = <EditorRootContentWidget> this.tabcontainer.currentPageWidget;

            if (w && w.editor) {

                if (this.currentResourceEditor != w.editor) {

                    if (w.editor.typeName == "SceneEditor3D") {

                        this.sendEvent(EngineEditor.EditorActiveSceneEditorChangeEventData({ sceneEditor: (<EngineEditor.SceneEditor3D> w.editor) }));

                    }

                    this.sendEvent(EngineEditor.EditorResourceEditorChangedEventData({ resourceEditor: w.editor }));

                }

            }

        }

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_POINTER_UP) {
            this.wasClosed = false;
        }

        // bubble
        return false;

    }

    shutdown() {

        // on exit close all open editors
        for (var path in this.editors) {

            this.sendEvent(EngineEditor.EditorResourceCloseEventData({ editor: this.editors[path], navigateToAvailableResource: false }));

        }

    }

    handleProjectUnloaded(data) {

      for (var i in this.editors) {
          var editor = this.editors[i];
           this.sendEvent(EngineEditor.EditorResourceCloseEventData({ editor: editor, navigateToAvailableResource: false }));
           editor.close();
      }

    }

    constructor(parent: EngineCore.UIWidget) {

        super();

        this.load("editor/ui/resourceframe.tb.txt");

        this.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_ALL;

        this.resourceViewContainer  = <EngineCore.UILayout> parent.getWidget("resourceviewcontainer");
        this.tabcontainer           = <EngineCore.UITabContainer> this.getWidget("tabcontainer");
        this.resourceLayout         = <EngineCore.UILayout> this.getWidget("resourcelayout");

        this.resourceViewContainer.addChild(this);
        this.resourceEditorProvider = new ResourceEditorProvider(this);
        this.resourceEditorProvider.loadStandardEditors();

        this.subscribeToEvent(EngineEditor.ProjectUnloadedNotificationEvent((data) => this.handleProjectUnloaded(data)));
        this.subscribeToEvent(EngineEditor.EditorEditResourceEvent((data) => this.handleEditResource(data)));
        this.subscribeToEvent(EngineEditor.EditorSaveResourceEvent((data) => this.handleSaveResource(data)));
        this.subscribeToEvent(EngineEditor.EditorSaveAllResourcesEvent((data) => this.handleSaveAllResources()));
        this.subscribeToEvent(EngineEditor.EditorResourceCloseEvent((ev: EngineEditor.EditorResourceCloseEvent) => this.handleCloseResource(ev)));
        this.subscribeToEvent(EngineEditor.EditorRenameResourceNotificationEvent((ev: EngineEditor.EditorRenameResourceNotificationEvent) => this.handleRenameResource(ev)));
        this.subscribeToEvent(EngineEditor.EditorDeleteResourceNotificationEvent((data) => this.handleDeleteResource(data)));

        this.subscribeToEvent(EngineEditor.EditorResourceEditorChangedEvent((data) => this.handleResourceEditorChanged(data)));

        this.subscribeToEvent(EngineCore.UIWidgetEvent((data) => this.handleWidgetEvent(data)));

    }

}

export = ResourceFrame;
