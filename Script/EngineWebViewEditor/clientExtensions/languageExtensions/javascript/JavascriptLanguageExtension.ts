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

/**
 * Resource extension that handles configuring the editor for Javascript
 */
export default class JavascriptLanguageExtension implements EngineEditor.ClientExtensions.WebViewServiceEventListener {
    name: string = "ClientJavascriptLanguageExtension";
    description: string = "Javascript language services for the editor.";

    private editor: monaco.editor.IStandaloneCodeEditor;
    private active = false;

    private serviceLocator: EngineEditor.ClientExtensions.ClientServiceLocator;

    /**
    * Initialize the language service
     * @param  {Editor.ClientExtensions.ClientServiceLocator} serviceLocator
     */
    initialize(serviceLocator: EngineEditor.ClientExtensions.ClientServiceLocator) {
        // initialize the language service
        this.serviceLocator = serviceLocator;
        serviceLocator.clientServices.register(this);
    }

    /**
     * Determines if the file name/path provided is something we care about
     * @param  {string} path
     * @return {boolean}
     */
    private isValidFiletype(path: string): boolean {
        let ext = path.split(".").pop();
        return ext == "js";
    }

    /**
     * Called when the editor needs to be configured for a particular file
     * @param  {Editor.ClientExtensions.EditorFileEvent} ev
     */
    configureEditor(ev: EngineEditor.ClientExtensions.EditorFileEvent) {
        if (this.isValidFiletype(ev.filename)) {
            this.editor = ev.editor; // cache this so that we can reference it later
            this.active = true;
        } else {
            this.active = false;
        }
    }

    /**
     * Format the code
     * @memberOf JavascriptLanguageExtension
     */
    formatCode() {
        // do nothing.  This is being handled by the TypeScriptLanguageExtension
    }
}
