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
import ModalWindow = require("../ModalWindow");

class EULAWindow extends ModalWindow {

    constructor() {

        super();

        this.settings = EngineCore.UI_WINDOW_SETTINGS.UI_WINDOW_SETTINGS_DEFAULT & ~EngineCore.UI_WINDOW_SETTINGS.UI_WINDOW_SETTINGS_CLOSE_BUTTON;

        this.init("License Agreement", "editor/ui/eulaagreement.tb.txt");

        this.age_license            = <EngineCore.UIEditField> this.getWidget("age_license");
        this.thirdparty_license     = <EngineCore.UIEditField> this.getWidget("thirdparty_license");
        this.externaltool_license   = <EngineCore.UIEditField> this.getWidget("externaltool_license");
        this.eulaCheck              = <EngineCore.UICheckBox> this.getWidget("eula_check");

        var container = this.getWidget("tabcontainer");
        container.value = 0;

        var cache = EngineCore.cache;

        var file = cache.getFile("EngineCore/eulas/atomic_game_engine_eula.txt");
        this.age_license.text = file.readText();

        file = cache.getFile("EngineCore/eulas/atomic_thirdparty_eula.txt");
        this.thirdparty_license.text = file.readText();

        file = cache.getFile("EngineCore/eulas/atomic_external_tools_eula.txt");
        this.externaltool_license.text = file.readText();

    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "quit") {

                this.sendEvent(EngineCore.ExitRequestedEventType);
                return true;

            } else if (id == "ok") {

                if (!this.eulaCheck.value) {
                  EditorUI.showModalError("License Agreement", "Please agree to licensing terms and conditions to continue");
                    return true;
                }

                this.hide();

                var licenseSystem = ToolCore.getLicenseSystem();
                licenseSystem.licenseAgreementConfirmed();

                this.sendEvent(ToolCore.LicenseEulaAcceptedEventType);

                return true;
            }

        }

    }

    age_license             : EngineCore.UIEditField;
    thirdparty_license      : EngineCore.UIEditField;
    externaltool_license    : EngineCore.UIEditField;
    eulaCheck               : EngineCore.UICheckBox;

}

export = EULAWindow;
