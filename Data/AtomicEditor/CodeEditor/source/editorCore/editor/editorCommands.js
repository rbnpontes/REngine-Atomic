System.register(["./editor", "../clientExtensions/ServiceLocator", "../interop", "../clientExtensions/ClientExtensionEventNames"], function (exports_1, context_1) {
    "use strict";
    var internalEditor, ServiceLocator_1, interop_1, ClientExtensionEventNames_1;
    var __moduleName = context_1 && context_1.id;
    function configure(fileExt, filename) {
        let monacoEditor = internalEditor.getInternalEditor();
        updateEditorPrefs();
        ServiceLocator_1.default.sendEvent(ClientExtensionEventNames_1.default.ConfigureEditorEvent, {
            fileExt: fileExt,
            filename: filename,
            editor: monacoEditor
        });
        const noOpCommand = () => { };
        monacoEditor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KEY_I, noOpCommand, null);
        updateEditorPrefs();
    }
    exports_1("configure", configure);
    function updateEditorPrefs() {
        const renderWhitespaceAdapter = (setting) => {
            switch (setting.toLowerCase()) {
                case "true": return "all";
                case "false": return "none";
                default: return setting;
            }
        };
        let monacoEditor = internalEditor.getInternalEditor();
        monacoEditor.updateOptions({
            renderWhitespace: renderWhitespaceAdapter(ServiceLocator_1.default.clientServices.getApplicationPreference("codeEditor", "showInvisibles", "none")),
            mouseWheelScrollSensitivity: 2,
            fontSize: ServiceLocator_1.default.clientServices.getApplicationPreference("codeEditor", "fontSize", 12),
            fontFamily: ServiceLocator_1.default.clientServices.getApplicationPreference("codeEditor", "fontFamily", "")
        });
        monaco.editor.setTheme(ServiceLocator_1.default.clientServices.getApplicationPreference("codeEditor", "theme", "vs-dark"));
    }
    exports_1("updateEditorPrefs", updateEditorPrefs);
    function getSourceText() {
        return internalEditor.getInternalEditor().getModel().getValue();
    }
    exports_1("getSourceText", getSourceText);
    function loadCodeIntoEditor(code, filename, fileExt) {
        let monacoEditor = internalEditor.getInternalEditor();
        let model = monaco.editor.createModel(code, null, monaco.Uri.parse(filename));
        model.updateOptions({
            insertSpaces: ServiceLocator_1.default.clientServices.getApplicationPreference("codeEditor", "useSoftTabs", true),
            tabSize: ServiceLocator_1.default.clientServices.getApplicationPreference("codeEditor", "tabSize", 4)
        });
        monacoEditor.setModel(model);
        ServiceLocator_1.default.sendEvent(ClientExtensionEventNames_1.default.CodeLoadedEvent, {
            code: code,
            filename: filename,
            fileExt: fileExt,
            editor: monacoEditor
        });
        monacoEditor.onDidChangeModelContent((listener) => {
            interop_1.default.getInstance().notifyEditorChange();
        });
    }
    exports_1("loadCodeIntoEditor", loadCodeIntoEditor);
    function resourceRenamed(path, newPath) {
        let data = {
            path: path,
            newPath: newPath
        };
        ServiceLocator_1.default.sendEvent(ClientExtensionEventNames_1.default.ResourceRenamedEvent, data);
    }
    exports_1("resourceRenamed", resourceRenamed);
    function resourceDeleted(path) {
        let data = {
            path: path
        };
        ServiceLocator_1.default.sendEvent(ClientExtensionEventNames_1.default.ResourceDeletedEvent, data);
    }
    exports_1("resourceDeleted", resourceDeleted);
    function codeSaved(path, fileExt, contents) {
        let data = {
            filename: path,
            fileExt: fileExt,
            editor: internalEditor.getInternalEditor(),
            code: contents
        };
        ServiceLocator_1.default.sendEvent(ClientExtensionEventNames_1.default.CodeSavedEvent, data);
    }
    exports_1("codeSaved", codeSaved);
    function editorLoaded() {
        ServiceLocator_1.default.clientServices.setPreferences(JSON.parse(window.HOST_Preferences.ProjectPreferences), JSON.parse(window.HOST_Preferences.ApplicationPreferences));
    }
    exports_1("editorLoaded", editorLoaded);
    function preferencesChanged(prefs) {
        ServiceLocator_1.default.clientServices.setPreferences(prefs.projectPreferences, prefs.applicationPreferences);
        updateEditorPrefs();
        ServiceLocator_1.default.sendEvent(ClientExtensionEventNames_1.default.PreferencesChangedEvent, prefs);
    }
    exports_1("preferencesChanged", preferencesChanged);
    function setEditor(editor) {
        internalEditor.setInternalEditor(editor);
    }
    exports_1("setEditor", setEditor);
    function formatCode() {
        ServiceLocator_1.default.sendEvent(ClientExtensionEventNames_1.default.FormatCodeEvent, null);
    }
    exports_1("formatCode", formatCode);
    function invokeShortcut(shortcut) {
        const ed = internalEditor.getInternalEditor();
        ed.focus();
        switch (shortcut) {
            case "cut":
            case "copy":
            case "paste":
                window.document.execCommand(shortcut);
                break;
            case "selectall":
                ed.setSelection(ed.getModel().getFullModelRange());
                break;
        }
    }
    exports_1("invokeShortcut", invokeShortcut);
    function gotoLineNumber(lineNumber) {
        const ed = internalEditor.getInternalEditor();
        ed.revealLineInCenterIfOutsideViewport(lineNumber);
        ed.setPosition(new monaco.Position(lineNumber, 0));
    }
    exports_1("gotoLineNumber", gotoLineNumber);
    function gotoTokenPos(tokenPos) {
        const ed = internalEditor.getInternalEditor();
        const pos = ed.getModel().getPositionAt(tokenPos);
        ed.revealPositionInCenterIfOutsideViewport(pos);
        ed.setPosition(pos);
    }
    exports_1("gotoTokenPos", gotoTokenPos);
    return {
        setters: [
            function (internalEditor_1) {
                internalEditor = internalEditor_1;
            },
            function (ServiceLocator_1_1) {
                ServiceLocator_1 = ServiceLocator_1_1;
            },
            function (interop_1_1) {
                interop_1 = interop_1_1;
            },
            function (ClientExtensionEventNames_1_1) {
                ClientExtensionEventNames_1 = ClientExtensionEventNames_1_1;
            }
        ],
        execute: function () {
        }
    };
});
//# sourceMappingURL=editorCommands.js.map