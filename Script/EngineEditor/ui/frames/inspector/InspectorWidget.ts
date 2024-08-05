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


class InspectorWidget extends ScriptWidget {

    constructor() {

        super();

        var fd = this.attrFontDesc = new EngineCore.UIFontDescription();
        fd.id = "Vera";
        fd.size = 11;

        var nlp = new EngineCore.UILayoutParams();
        nlp.width = 310;

        var layout = this.rootLayout = new EngineCore.UILayout();
        layout.spacing = 4;

        layout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;
        layout.layoutPosition     = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
        layout.layoutParams = nlp;
        layout.axis = EngineCore.UI_AXIS.UI_AXIS_Y;

        this.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_ALL;
        this.addChild(layout);

        this.subscribeToEvent(EngineCore.UIWidgetEvent((data) => this.handleWidgetEvent(data)));

    }

    onApply() {
      console.log("Apply Pressed!");
    }

    createAttrName(name:string):EngineCore.UITextField {

      var nameField = new EngineCore.UITextField();
      nameField.textAlign = EngineCore.UI_TEXT_ALIGN.UI_TEXT_ALIGN_LEFT;
      nameField.skinBg = "InspectorTextAttrName";
      nameField.text = name;
      nameField.fontDescription = this.attrFontDesc;
      return nameField;
    }

    createSection(parent:EngineCore.UIWidget, text:string, expanded:number):EngineCore.UILayout {

      var section = new EngineCore.UISection();

      section.text = text;
      section.value = expanded;
      section.fontDescription = this.attrFontDesc;

      var layout = this.createVerticalAttrLayout();
      parent.addChild(section);
      section.contentRoot.addChild(layout);

      return layout;

    }

    createVerticalAttrLayout():EngineCore.UILayout {

      var layout = new EngineCore.UILayout(EngineCore.UI_AXIS.UI_AXIS_Y);
      layout.spacing = 3;
      layout.layoutPosition = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
      layout.layoutSize = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_AVAILABLE;

      return layout;

    }

    createApplyButton():EngineCore.UIButton {

      var button = new EngineCore.UIButton();
      button.fontDescription = this.attrFontDesc;
      button.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_RIGHT;
      button.text = "Apply";

      button.onClick = function() {

        this.onApply();

      }.bind(this);

      return button;

    }

    createAttrCheckBox(name:string, parent:EngineCore.UIWidget):EngineCore.UICheckBox {

      var attrLayout = new EngineCore.UILayout();
      attrLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;

      var _name = this.createAttrName(name);
      attrLayout.addChild(_name);

      var box = new EngineCore.UICheckBox();
      box.skinBg = "TBCheckBox";
      attrLayout.addChild(box);
      parent.addChild(attrLayout);

      return box;

    }

    createAttrEditField(name:string, parent:EngineCore.UIWidget):EngineCore.UIEditField {

      var attrLayout = new EngineCore.UILayout();
      attrLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;

      var _name = this.createAttrName(name);
      attrLayout.addChild(_name);

      var edit = new EngineCore.UIEditField();
      edit.textAlign = EngineCore.UI_TEXT_ALIGN.UI_TEXT_ALIGN_LEFT;
      edit.skinBg = "TBAttrEditorField";
      edit.fontDescription = this.attrFontDesc;
      var lp = new EngineCore.UILayoutParams();
      lp.width = 140;
      edit.layoutParams = lp;

      attrLayout.addChild(edit);
      parent.addChild(attrLayout);

      return edit;

    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent):boolean {

      return false;

    }

    createPreviewAnimationButton(asset: ToolCore.Asset): EngineCore.UIButton {

        var button = new EngineCore.UIButton();
        button.fontDescription = this.attrFontDesc;
        button.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_RIGHT;
        button.text = "Preview Animation";

        button.onClick = function () {
            this.onPreviewAnimation(asset);
            // button is deleted in callback, so make sure we return
            // that we're handled
            return true;
        }.bind(this);

        return button;
    }

    onPreviewAnimation(asset: ToolCore.Asset) {

        var mainFrame = EditorUI.getMainFrame();
        mainFrame.showAnimationToolbar(asset);
    }

    rootLayout:EngineCore.UILayout;
    attrFontDesc:EngineCore.UIFontDescription;
}

export  = InspectorWidget;