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

import PlayerOutput = require("./PlayerOutput");
import Preferences = require("../../editor/Preferences");

class PlayMode extends EngineCore.ScriptObject {

    inErrorState: boolean;
    myPlayer: PlayerOutput;

    constructor() {

        super();
        this.myPlayer = null;
        this.subscribeToEvent(EngineCore.IPCJSErrorEvent((ev: EngineCore.IPCJSErrorEvent) => this.handleIPCJSError(ev)));
        this.subscribeToEvent(EngineEditor.EditorPlayerStartedEvent((ev) => this.handlePlayerStarted(ev)));
        this.subscribeToEvent(EngineEditor.EditorPlayerStoppedEvent((ev) => this.handlePlayerStopped(ev)));

    }

    handlePlayerStarted(ev) {

        this.inErrorState = false;

        if ( this.myPlayer != null ) {
             this.myPlayer.remove();
        }

        this.myPlayer = new PlayerOutput();
    }

    handlePlayerStopped(ev) {

        if ( Preferences.getInstance().editorFeatures.closePlayerLog ) {
             this.myPlayer.remove();
        }

    }


    handleIPCJSError(ev: EngineCore.IPCJSErrorEvent) {

        if (this.inErrorState)
            return;

        this.inErrorState = true;

        var errorMessage = ev.errorFileName + " - " + ev.errorLineNumber + " : " + ev.errorMessage;
        this.sendEvent(EngineEditor.EditorModalEventData({ type: EngineEditor.EDITOR_MODALERROR, title: "Player JavaScript Error", message: errorMessage }));

        EngineCore.graphics.raiseWindow();

    }

}

export = PlayMode;
