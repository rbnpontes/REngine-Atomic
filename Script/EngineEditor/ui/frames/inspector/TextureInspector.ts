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
import InspectorWidget = require("./InspectorWidget");
import InspectorUtils = require("./InspectorUtils");

import TextureSelector = require("./TextureSelector");

class TextureInspector extends InspectorWidget {

    constructor() {

        super();

        this.fd.id = "Vera";
        this.fd.size = 11;

    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent): boolean {

        return false;

    }

    getTextureThumbnail(texture: EngineCore.Texture2D): EngineCore.Texture {

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

    createTextureSection(): EngineCore.UISection {

        var section = new EngineCore.UISection();
        section.text = "Texture Image";
        section.value = 1;
        section.fontDescription = this.fd;

        var attrsVerticalLayout = new EngineCore.UILayout(EngineCore.UI_AXIS.UI_AXIS_Y);
        attrsVerticalLayout.spacing = 3;
        attrsVerticalLayout.layoutPosition  = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_CENTER;
        attrsVerticalLayout.layoutSize      = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_AVAILABLE;

        section.contentRoot.addChild(attrsVerticalLayout);

        var attrLayout = new EngineCore.UILayout();
        attrLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_PREFERRED;

        var textureWidget = new EngineCore.UITextureWidget();

        var tlp = new EngineCore.UILayoutParams();
        tlp.width = 200;
        tlp.height = 200;
        textureWidget.layoutParams = tlp;
        textureWidget.texture = this.getTextureThumbnail(this.texture);

        var textureButton = new EngineCore.UIButton();
        textureButton.skinBg = "TBButton.flatoutline";
        textureButton["textureWidget"] = textureWidget;

        textureButton.contentRoot.addChild(textureWidget);

        attrLayout.addChild(textureButton);

        attrsVerticalLayout.addChild(attrLayout);

        return section;

    }

    inspect(texture: EngineCore.Texture2D, asset: ToolCore.Asset) {

        this.texture = texture;
        this.asset = asset;
        this.importer = <ToolCore.TextureImporter>asset.importer;

        var mlp = new EngineCore.UILayoutParams();
        mlp.width = 310;

        var textureLayout = new EngineCore.UILayout();
        textureLayout.spacing = 4;

        textureLayout.layoutDistribution    = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;
        textureLayout.layoutPosition        = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
        textureLayout.layoutParams = mlp;
        textureLayout.axis = EngineCore.UI_AXIS.UI_AXIS_Y;

        var textureSection = new EngineCore.UISection();
        textureSection.text = "Texture";
        textureSection.value = 1;
        textureSection.fontDescription = this.fd;
        textureLayout.addChild(textureSection);

        var attrsVerticalLayout = new EngineCore.UILayout(EngineCore.UI_AXIS.UI_AXIS_Y);
        attrsVerticalLayout.spacing = 3;
        attrsVerticalLayout.layoutPosition = EngineCore.UI_LAYOUT_POSITION.UI_LAYOUT_POSITION_LEFT_TOP;
        attrsVerticalLayout.layoutSize = EngineCore.UI_LAYOUT_SIZE.UI_LAYOUT_SIZE_PREFERRED;

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

        field.text = EngineCore.splitPath(asset.name).fileName;

        nameLayout.addChild(field);

        attrsVerticalLayout.addChild(nameLayout);

        var maxSizeLayout = new EngineCore.UILayout();
        maxSizeLayout.layoutDistribution = EngineCore.UI_LAYOUT_DISTRIBUTION.UI_LAYOUT_DISTRIBUTION_GRAVITY;

        //COMPRESSION SIZE
        var maxSize = InspectorUtils.createAttrName("Max Size");
        this.populateCompressionSizeList();

        maxSizeLayout.addChild(maxSize);
        maxSizeLayout.addChild(this.compressionSize);

        attrsVerticalLayout.addChild(maxSizeLayout);
        attrsVerticalLayout.addChild(this.createApplyButton());

        textureSection.contentRoot.addChild(attrsVerticalLayout);

        textureLayout.addChild(this.createTextureSection());

        this.addChild(textureLayout);

    }

    onApply() {

        this.importer.setCompressedImageSize(Number(this.compressionSize.text));
        this.asset.import();
        this.asset.save();

    }

    populateCompressionSizeList() {
        this.compressionSize = new EngineCore.UISelectDropdown();
        this.compressionSizeSource = new EngineCore.UISelectItemSource();

        for (var i = 0; i < this.compressionSizes.length; i ++) {
            var size = new EngineCore.UISelectItem();
            size.setString(this.compressionSizes[i].toString());
            this.compressionSizeSource.addItem(size);
        }

        this.compressionSize.setSource(this.compressionSizeSource);

        if (this.importer.getCompressedImageSize() != 0) {
            this.compressionSize.setText(this.importer.getCompressedImageSize().toString());
        }
        else {
            this.compressionSize.setText("NONE");
        }
    }

    texture: EngineCore.Texture2D;

    techniqueButton: EngineCore.UIButton;
    material: EngineCore.Material;
    asset: ToolCore.Asset;
    nameTextField: EngineCore.UITextField;
    compressionSize: EngineCore.UISelectDropdown;
    compressionSizeSource: EngineCore.UISelectItemSource;
    fd: EngineCore.UIFontDescription = new EngineCore.UIFontDescription();
    importer: ToolCore.TextureImporter;
    compressionSizes: number[] = [32, 64, 128, 256, 512, 1024, 2048, 4096, 8192];

}

export = TextureInspector;
