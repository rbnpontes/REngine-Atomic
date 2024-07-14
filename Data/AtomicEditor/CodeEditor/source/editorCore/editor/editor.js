System.register([], function (exports_1, context_1) {
    "use strict";
    var editor;
    var __moduleName = context_1 && context_1.id;
    function getInternalEditor() {
        return editor;
    }
    exports_1("getInternalEditor", getInternalEditor);
    function setInternalEditor(editorInstance) {
        editor = editorInstance;
    }
    exports_1("setInternalEditor", setInternalEditor);
    return {
        setters: [],
        execute: function () {
        }
    };
});
//# sourceMappingURL=editor.js.map