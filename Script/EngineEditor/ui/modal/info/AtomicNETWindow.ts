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

import EditorUI = require("../../EditorUI");
import ModalWindow = require("../ModalWindow");

class AtomicNETWindow extends ModalWindow {

    constructor() {

        super();

        this.settings = EngineCore.UI_WINDOW_SETTINGS.UI_WINDOW_SETTINGS_DEFAULT & ~EngineCore.UI_WINDOW_SETTINGS.UI_WINDOW_SETTINGS_CLOSE_BUTTON;

        // we're not calling this.init here as it calls resizeToFitContent
        // and center, which screw up the generated About text being resized

        this.text = "Atomic C# Requirements";
        this.load("editor/ui/atomicnetwindow.tb.txt");

        this.downloadButton = <EngineCore.UIButton>this.getWidget("download_button");

        this.atomicnet_text = <EngineCore.UIEditField>this.getWidget("atomicnet_text");
        this.atomicnet_text.text = this.generateAtomicNETText();

        this.resizeToFitContent();
        this.center();

    }

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = ev.target.id;

            if (id == "ok") {

                this.hide();

                return true;
            }

            if (id == "download_button") {

                this.hide();

                if (EngineCore.platform == "Windows")
                    EngineCore.fileSystem.systemOpen( "https://www.visualstudio.com/vs/community/" );

                if (EngineCore.platform == "MacOSX")
                    EngineCore.fileSystem.systemOpen( "https://www.xamarin.com/download/");

                if (EngineCore.platform == "Linux")
                    EngineCore.fileSystem.systemOpen( "https://github.com/AtomicGameEngine/AtomicGameEngine/wiki/Detailed-instructions-for-building-on-Linux");

            }
        }
    }


    generateAtomicNETText(): string {
        // start at Atomic.platform == "Windows"
        let ideText:string = "Visual Studio";
        if (EngineCore.platform == "MacOSX") ideText = "Visual Studio for Mac";
        if (EngineCore.platform == "Linux") ideText = "MonoDevelop";


        let installText = `Please install ${ideText} with <color #D4FB79>Xamarin.Android</color> and <color #D4FB79>Xamarin.iOS</color>`;

        if (EngineCore.platform != "Linux" )
            this.downloadButton.text = `Download ${ideText}`;
        else
            this.downloadButton.text = `Install ${ideText}`;

        let text = "";

        text += `
Atomic C# is integrated with <color #D4FB79>Visual Studio</color>, <color #D4FB79>Visual Studio for Mac</color>, and <color #D4FB79>MonoDevelop</color> to provide a first class editing, debugging, and deployment experience.

${installText}

`;

        return text;

    }

    atomicnet_text: EngineCore.UIEditField;
    downloadButton: EngineCore.UIButton;
}

export = AtomicNETWindow;
