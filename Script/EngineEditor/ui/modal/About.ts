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

import EditorUI = require("../EditorUI");
import ModalWindow = require("./ModalWindow");

class About extends ModalWindow {

    constructor() {

        super();

        // we're not calling this.init here as it calls resizeToFitContent
        // and center, which screw up the generated About text being resized

        this.text = "About the Atomic Game Engine";
        this.load("editor/ui/about.tb.txt");

        this.age_license            = <EngineCore.UIEditField>this.getWidget("age_license");
        this.thirdparty_license     = <EngineCore.UIEditField>this.getWidget("thirdparty_license");
        this.externaltool_license   = <EngineCore.UIEditField>this.getWidget("externaltool_license");
        this.about_text             = <EngineCore.UIEditField>this.getWidget("about_text");

        var cache = EngineCore.cache;

        var file = cache.getFile("AtomicEditor/eulas/atomic_game_engine_eula.txt");
        this.age_license.text = file.readText();

        file = cache.getFile("AtomicEditor/eulas/atomic_thirdparty_eula.txt");
        this.thirdparty_license.text = file.readText();

        file = cache.getFile("AtomicEditor/eulas/atomic_external_tools_eula.txt");
        this.externaltool_license.text = file.readText();

        this.about_text.text = this.generateAboutText();

        var get_pro = <EngineCore.UIButton>this.getWidget("purchase_pro");
        if (get_pro) {
            get_pro.onClick = function() {
                EngineCore.fileSystem.systemOpen("https://store.atomicgameengine.com/site");
            };
        }

        this.resizeToFitContent();
        this.center();

        this.getWidget("tabcontainer").value = 0;

    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "ok") {
                this.hide();
                return true;
            }
        }
    }


    generateAboutText(): string {

        var text = "";

        var buildName   = EngineCore.AtomicBuildInfo.getBuildName();
        var buildDate   = EngineCore.AtomicBuildInfo.getBuildDate();
        var buildTime   = EngineCore.AtomicBuildInfo.getBuildTime();
        var buildSHA    = EngineCore.AtomicBuildInfo.getGitSHA();
        var buildVendor = EngineCore.AtomicBuildInfo.getBuildVendor();

        var buildString = `<color #FFFFFF>'${buildName}' - ${buildDate} ${buildTime}\nGit: ${buildSHA}\nVendor: <color #76D6FF>${buildVendor}</color></color>`;

        text += "<widget TBImageWidget: filename: 'editor/images/atomic_logo.png'>\n\n";
        text += "(c) 2014-2016 THUNDERBEAST GAMES LLC\n\n";

        text += "<color #76D6FF>Build Information:</color>\n";

        text += buildString + "\n\n";

        let contributors = ["JoshEngebretson", "shaddockh", "rsredsq", "JimMarlowe", "mattbenic",
        "Type1J", "weinandvv", "JohnnyWahib", "raheelx", "CTrauma", "eugenegous", "christoffersch", "GarethNN",
        "LaraEngebretson", "bitonator", "rokups", "honigbeutler123", "benwolf", "jonaspm",
        "keithjohnston", "darrylryan", "Alan-FGR", "marynate", "Sleaker", "Tarik-B", "flyover", "buresu",
        "rosshadden", "bparvanov" ];

        contributors.sort();

        text += `<color #76D6FF>Atomic Contributors:</color>\n<color #88FF88>${contributors.join(", ")}</color>\n\n`;

        text += "<color #76D6FF>Installed platforms and modules:</color>\n\n";

        var licenseSystem = ToolCore.licenseSystem;

        var installedText = "    <widget TBSkinImage: skin: 'LogoMac-Small'> <widget TBSkinImage: skin: 'LogoWindows-Small'> <widget TBSkinImage: skin: 'LogoHTML5-Small'> ";

        installedText += "<widget TBSkinImage: skin: 'LogoAndroid-Small'> ";

        installedText += "<widget TBSkinImage: skin: 'LogoIOS-Small'> ";

        installedText += "<widget TBSkinImage: skin: 'Module2D-Small'> ";

        installedText += "<widget TBSkinImage: skin: 'Module3D-Small'> ";

        text += installedText + "\n\n\n";

        text += "<color #76D6FF>Special Thanks:</color>\n";
        text += "The Urho3D Project (http://urho3d.github.io), Sami Vaarala (http://www.duktape.org)\n";

        return text;

    }

    age_license         : EngineCore.UIEditField;
    thirdparty_license  : EngineCore.UIEditField;
    externaltool_license: EngineCore.UIEditField;
    about_text          : EngineCore.UIEditField;

}

export = About;
