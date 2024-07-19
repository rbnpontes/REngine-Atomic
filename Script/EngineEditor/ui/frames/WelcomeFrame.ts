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
import ScriptWidget = require("ui/ScriptWidget");
import Preferences = require("editor/Preferences");
import ProjectTemplates = require("resources/ProjectTemplates");

class WelcomeFrame extends ScriptWidget {

    constructor(parent: EngineCore.UIWidget) {

        super();

        this.load("editor/ui/welcomeframe.tb.txt");

        var recentProjects      = <EngineCore.UILayout>this.getWidget("recentprojects");

        this.examplesLayout     = <EngineCore.UILayout>this.getWidget("examples_layout");
        this.examplesCSharp     = <EngineCore.UIButton>this.getWidget("examples_csharp");
        this.examplesJavaScript = <EngineCore.UIButton>this.getWidget("examples_javascript");
        this.examplesTypeScript = <EngineCore.UIButton>this.getWidget("examples_typescript");

        this.examplesCSharp.onClick = () => { this.handleExampleFilter(); };
        this.examplesJavaScript.onClick = () => { this.handleExampleFilter(); };
        this.examplesTypeScript.onClick = () => { this.handleExampleFilter(); };

        this.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_ALL;

        this.recentList = new EngineCore.UIListView();
        this.recentList.rootList.id = "recentList";

        recentProjects.addChild(this.recentList);

        var container = <EngineCore.UILayout>parent.getWidget("resourceviewcontainer");

        container.addChild(this);

        this.updateRecentProjects();

        this.subscribeToEvent(EngineEditor.EditorCloseProjectEvent(() => {
            this.updateRecentProjects();
        }));

        this.initExampleBrowser();

    }

    handleClickedExample(example: ProjectTemplates.ProjectTemplateDefinition) {

        var ops = EditorUI.getModelOps();
        ops.showCreateProject(example);
    }

    addExample(example: ProjectTemplates.ProjectTemplateDefinition) {

        var fileSystem = EngineCore.getFileSystem();

        let languages = [];
        if (this.examplesCSharp.value) {
            languages.push("CSharp");
        }
        if (this.examplesJavaScript.value) {
            languages.push("JavaScript");
        }
        if (this.examplesTypeScript.value) {
            languages.push("TypeScript");
        }

        // If user doesn't select any languages, show 'em all'
        if (!languages.length) {
            languages = ["CSharp", "JavaScript", "TypeScript"];
        }

        let exists = false;

        for (var i = 0; i < languages.length; i++) {

            if (example.languages.indexOf(languages[i]) != -1) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            return;
        }

        if (!this.currentExampleLayout) {
            this.currentExampleLayout = new EngineCore.UILayout();
            this.currentExampleLayout.spacing = 8;
            this.examplesLayout.addChild(this.currentExampleLayout);
        }

        // 200x150

        var exampleLayout = new EngineCore.UILayout();
        exampleLayout.skinBg = "StarCondition";
        exampleLayout.axis = EngineCore.UI_AXIS.UI_AXIS_Y;
        exampleLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;
        exampleLayout.layoutSize = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_AVAILABLE;

        // IMAGE BUTTON

        var id = example.name;

        var button = new EngineCore.UIButton();
        button.skinBg = "StarButton";
        button.id = id;
        var image = new EngineCore.UIImageWidget();

        button.onClick = () => {

            this.handleClickedExample(example);

        };

        image.image = example.screenshot;
        image.skinBg = "ImageFrame";
        var rect = [0, 0, image.imageWidth / 2, image.imageHeight / 2];
        image.rect = rect;

        // NAME FIELD
        var nameField = new EngineCore.UITextField();
        nameField.skinBg = "ImageCaption";
        nameField.text = example.name;

        var nameRect = [0, image.imageHeight / 2 - 16, image.imageWidth / 2, image.imageHeight / 2];

        nameField.rect = nameRect;

        nameField.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_BOTTOM;

        image.addChild(nameField);

        button.addChild(image);

        var lp = new EngineCore.UILayoutParams();
        lp.minWidth = image.imageWidth / 2;
        lp.minHeight = image.imageHeight / 2;

        button.layoutParams = lp;

        button.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_LEFT;

        exampleLayout.addChild(button);

        // DESC TEXT

        var descField = new EngineCore.UIEditField();
        descField.styling = true;
        descField.multiline = true;
        descField.readOnly = true;
        descField.wrapping = true;

        descField.skinBg = "AccentColor4";

        descField.text = example.desc;

        descField.adaptToContentSize = true;

        lp.height = 42;

        lp.width = image.imageWidth / 2;

        descField.layoutParams = lp;

        exampleLayout.addChild(descField);

        this.currentExampleLayout.addChild(exampleLayout);

        this.exampleCount++;
        // three across, todo, be smarter about this
        if (!(this.exampleCount % 3)) {
            this.currentExampleLayout = null;
        }

    }

    handleExampleFilter() {

        this.initExampleBrowser();

    }

    initExampleBrowser() {

        this.exampleCount = 0;
        this.examplesLayout.deleteAllChildren();
        this.currentExampleLayout = null;

        let examples = ProjectTemplates.getExampleProjectTemplateDefinitions();
        for (var i = 0; i < examples.length; i++) {
            this.addExample(examples[i]);
        }
    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_RIGHT_POINTER_UP) {
            if (ev.target.id == "recentList") {
                this.openFrameMenu(ev.x, ev.y);
            }
        }

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "open project") {

                var utils = new EngineEditor.FileUtils();
                var path = utils.openProjectFileDialog();
                if (path) {

                    this.sendEvent(EngineEditor.RequestProjectLoadEventData({ path: path }));

                }

                return true;
            }

            if (id == "new project") {

                var mo = EditorUI.getModelOps();
                mo.showNewProject();
                return true;

            }

            if (id == "recentList") {
                if (!this.recentList.getSelectedItemID()) {
                    return;
                }
                var path: string = this.recent[this.recentList.getSelectedItemID()];
                this.sendEvent(EngineEditor.RequestProjectLoadEventData({ path: path }));
            }

            if (id == "recentProjectsContextMenu") {
                var prefs = Preferences.getInstance();
                if (ev.refid == "clear recent projects") {
                    prefs.deleteRecentProjects();
                    this.updateRecentProjects();
                }
            }
        }
    }

    updateRecentProjects() {

        this.recentList.deleteAllItems();

        var prefs = Preferences.getInstance();
        prefs.updateRecentProjects();

        this.recent = prefs.recentProjects;

        for (var i in this.recent) {
            this.recentList.addRootItem(this.recent[i], "Folder.icon", i);
        }

    }

    private openFrameMenu(x: number, y: number) {
        var menu        = new EngineCore.UIMenuWindow(this, "recentProjectsContextMenu");
        var menuButtons = new EngineCore.UISelectItemSource();
        menuButtons.addItem(new EngineCore.UISelectItem("Clear Recent Projects", "clear recent projects"));
        menu.show(menuButtons, x, y);
    }

    // examples
    exampleCount = 0;

    currentExampleLayout    : EngineCore.UILayout;
    examplesLayout          : EngineCore.UILayout;

    examplesCSharp          : EngineCore.UIButton;
    examplesJavaScript      : EngineCore.UIButton;
    examplesTypeScript      : EngineCore.UIButton;

    recent: string[] = [];
    recentList: EngineCore.UIListView;
}

export = WelcomeFrame;
