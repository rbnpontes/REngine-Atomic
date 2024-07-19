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

class CSComponentClassSelector extends EngineCore.UIWindow {

    constructor(editField: EngineCore.UIEditField, component: EngineNETScript.CSComponent) {

        super();

        var assemblyFile = component.componentFile;

        this.text = "Select Class: " + assemblyFile.name;

        this.rect = [0, 0, 400, 512];

        var mainLayout = new EngineCore.UILayout();
        mainLayout.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_ALL;
        mainLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_AVAILABLE;
        mainLayout.axis = EngineCore.UI_AXIS.UI_AXIS_Y;
        this.contentRoot.addChild(mainLayout);

        // really want a grid container
        var scrollContainer = new EngineCore.UIScrollContainer();
        scrollContainer.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_ALL;
        scrollContainer.scrollMode = EngineCore.UI_SCROLL_MODE.UI_SCROLL_MODE_Y_AUTO;
        scrollContainer.adaptContentSize = true;

        var scrollLayout = new EngineCore.UILayout();
        scrollLayout.layoutPosition = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
        scrollLayout.layoutDistributionPosition = EngineCore.UI_LAYOUT_DISTRIBUTION_POSITION.UI_LAYOUT_DISTRIBUTION_POSITION_LEFT_TOP;
        scrollLayout.axis = EngineCore.UI_AXIS.UI_AXIS_Y;

        scrollContainer.contentRoot.addChild(scrollLayout);

        var window = this;

        for (var i in assemblyFile.classNames) {

            var classname = assemblyFile.classNames[i];
            var button = new EngineCore.UIButton();
            button.text = classname;

            button.onClick = function() {
                editField.text = this.text;
                component.componentClassName = this.text;
                window.close();
            }.bind(button);

            scrollLayout.addChild(button);
        }

        mainLayout.addChild(scrollContainer);

        EditorUI.getMainFrame().addChild(this);

        this.center();

        this.subscribeToEvent(EngineCore.UIWidgetEvent((data) => this.handleWidgetEvent(data)));

    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            if (ev.target != this && !this.isAncestorOf(ev.target)) {

                //this.close();

            }

        }

        return false;

    }

}

export = CSComponentClassSelector;
