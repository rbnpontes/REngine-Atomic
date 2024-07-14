System.register([], function (exports_1, context_1) {
    "use strict";
    var JavascriptLanguageExtension;
    var __moduleName = context_1 && context_1.id;
    return {
        setters: [],
        execute: function () {
            JavascriptLanguageExtension = class JavascriptLanguageExtension {
                constructor() {
                    this.name = "ClientJavascriptLanguageExtension";
                    this.description = "Javascript language services for the editor.";
                    this.active = false;
                }
                initialize(serviceLocator) {
                    this.serviceLocator = serviceLocator;
                    serviceLocator.clientServices.register(this);
                }
                isValidFiletype(path) {
                    let ext = path.split(".").pop();
                    return ext == "js";
                }
                configureEditor(ev) {
                    if (this.isValidFiletype(ev.filename)) {
                        this.editor = ev.editor;
                        this.active = true;
                    }
                    else {
                        this.active = false;
                    }
                }
                formatCode() {
                }
            };
            exports_1("default", JavascriptLanguageExtension);
        }
    };
});
//# sourceMappingURL=JavascriptLanguageExtension.js.map