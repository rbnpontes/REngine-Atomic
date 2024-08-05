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

import EditorUI = require("../ui/EditorUI");
import Preferences = require("editor/Preferences");

class EditorLicense extends EngineCore.ScriptObject {

    constructor() {

        super();

        this.subscribeToEvent(ToolCore.LicenseEulaRequiredEvent((eventData) => this.handleLicenseEulaRequired(eventData)));
        this.subscribeToEvent(ToolCore.LicenseEulaAcceptedEvent((eventData) => this.handleLicenseEulaAccepted(eventData)));

    }

    handleLicenseEulaRequired(eventData) {

        var ops = EditorUI.getModelOps();
        ops.showEULAWindow();

    }

    handleLicenseEulaAccepted(eventData) {

        if (!EngineCore.AtomicBuildInfo.getDistBuild())
            return;

        var sha = EngineCore.AtomicBuildInfo.getGitSHA();

        if (sha == "Unversioned Build" || Preferences.getInstance().editorBuildData.lastEditorBuildSHA == sha)
            return;

        var ops = EditorUI.getModelOps();
        ops.showNewBuildWindow();

    }

}

export = EditorLicense;