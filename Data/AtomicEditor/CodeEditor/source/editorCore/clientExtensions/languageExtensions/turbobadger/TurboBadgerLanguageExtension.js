System.register([], function (exports_1, context_1) {
    "use strict";
    var TurboBadgerLanguageExtension;
    var __moduleName = context_1 && context_1.id;
    return {
        setters: [],
        execute: function () {
            TurboBadgerLanguageExtension = class TurboBadgerLanguageExtension {
                constructor() {
                    this.name = "ClientTurboBadgerLanguageExtension";
                    this.description = "TurboBadger language services for the editor.";
                    this.active = false;
                }
                initialize(serviceLocator) {
                    this.serviceLocator = serviceLocator;
                    serviceLocator.clientServices.register(this);
                }
                isValidFiletype(path) {
                    return path.toLowerCase().endsWith(".tb.txt") || path.toLowerCase().endsWith(".tb");
                }
                configureEditor(ev) {
                    if (this.isValidFiletype(ev.filename)) {
                        this.active = true;
                        monaco.languages.register({ id: "turbobadger" });
                        let editor = ev.editor;
                        monaco.editor.setModelLanguage(editor.getModel(), "turbobadger");
                        editor.updateOptions({
                            renderWhitespace: "all",
                            useTabStops: true,
                        });
                    }
                }
                codeLoaded(ev) {
                    if (this.isValidFiletype(ev.filename)) {
                        let editor = ev.editor;
                        editor.getModel().updateOptions({
                            insertSpaces: false
                        });
                    }
                }
                formatCode() {
                    if (this.active) {
                        alert("Code formatted not available for this syntax.");
                    }
                }
            };
            exports_1("default", TurboBadgerLanguageExtension);
        }
    };
});
//# sourceMappingURL=TurboBadgerLanguageExtension.js.map