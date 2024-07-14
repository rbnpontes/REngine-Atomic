System.register(["./editor/editorCommands"], function (exports_1, context_1) {
    "use strict";
    var editorCommands, DEBUG_PORT, DEBUG_ALERT, HostInteropType;
    var __moduleName = context_1 && context_1.id;
    return {
        setters: [
            function (editorCommands_1) {
                editorCommands = editorCommands_1;
            }
        ],
        execute: function () {
            DEBUG_PORT = 3335;
            DEBUG_ALERT = false;
            window.atomicQueryPromise = function (messageType, data) {
                return new Promise(function (resolve, reject) {
                    let queryMessage;
                    if (data) {
                        queryMessage = JSON.parse(JSON.stringify(data));
                        queryMessage.message = messageType;
                    }
                    else {
                        queryMessage = {
                            message: messageType
                        };
                    }
                    window.atomicQuery({
                        request: JSON.stringify(queryMessage),
                        persistent: false,
                        onSuccess: resolve,
                        onFailure: (error_code, error_message) => reject({ error_code: error_code, error_message: error_message })
                    });
                });
            };
            HostInteropType = class HostInteropType {
                constructor() {
                    this.fileName = null;
                    this.fileExt = null;
                    this.editorReady = new Promise((resolve, reject) => {
                        this.setCodeLoaded = resolve;
                    });
                }
                static getInstance() {
                    if (HostInteropType._inst == null) {
                        HostInteropType._inst = new HostInteropType();
                    }
                    return HostInteropType._inst;
                }
                loadCode(codeUrl) {
                    const fileExt = codeUrl.indexOf(".") != -1 ? codeUrl.split(".").pop() : "";
                    const filename = codeUrl.replace("atomic://", "");
                    this.fileName = filename;
                    this.fileExt = fileExt;
                    editorCommands.configure(fileExt, filename);
                    this.getResource(codeUrl).then((src) => {
                        editorCommands.loadCodeIntoEditor(src, filename, fileExt);
                        return window.atomicQueryPromise(HostInteropType.EDITOR_GET_USER_PREFS);
                    }).then(() => {
                        this.setCodeLoaded();
                    }).catch((e) => {
                        console.log("Error loading code: " + e.error_message);
                    });
                }
                saveCode() {
                    let source = editorCommands.getSourceText();
                    return window.atomicQueryPromise(HostInteropType.EDITOR_SAVE_CODE, {
                        payload: source
                    }).then(() => {
                        editorCommands.codeSaved(this.fileName, this.fileExt, source);
                    });
                }
                saveFile(filename, fileContents) {
                    return window.atomicQueryPromise(HostInteropType.EDITOR_SAVE_FILE, {
                        filename: filename,
                        payload: fileContents
                    });
                }
                editorLoaded() {
                    if (DEBUG_ALERT) {
                        alert(`Attach chrome dev tools to this instance by navigating to http://localhost:${DEBUG_PORT}`);
                    }
                    editorCommands.editorLoaded();
                    window.atomicQueryPromise(HostInteropType.EDITOR_LOAD_COMPLETE);
                }
                getResource(codeUrl) {
                    return new Promise(function (resolve, reject) {
                        const xmlHttp = new XMLHttpRequest();
                        xmlHttp.onreadystatechange = () => {
                            if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
                                resolve(xmlHttp.responseText);
                            }
                        };
                        xmlHttp.open("GET", codeUrl, true);
                        xmlHttp.send(null);
                    });
                }
                getFileResource(filename) {
                    return this.getResource(`atomic://${filename}`);
                }
                notifyEditorChange() {
                    window.atomicQueryPromise(HostInteropType.EDITOR_CHANGE).catch((e) => {
                        console.log("Error on change: " + e.error_message);
                    });
                }
                resourceRenamed(path, newPath) {
                    this.fileName = newPath;
                    editorCommands.resourceRenamed(path, newPath);
                }
                resourceDeleted(path) {
                    editorCommands.resourceDeleted(path);
                }
                preferencesChanged(prefs) {
                    editorCommands.preferencesChanged(prefs);
                }
                addCustomHostRoutine(routineName, callback) {
                    window[routineName] = callback;
                }
                setEditor(editor) {
                    editorCommands.setEditor(editor);
                }
                invokeShortcut(shortcut) {
                    editorCommands.invokeShortcut(shortcut);
                }
                formatCode() {
                    editorCommands.formatCode();
                }
                gotoLineNumber(lineNumber) {
                    this.editorReady.then(() => {
                        editorCommands.gotoLineNumber(lineNumber);
                    });
                }
                gotoTokenPos(tokenPos) {
                    editorCommands.gotoTokenPos(tokenPos);
                }
            };
            HostInteropType._inst = null;
            HostInteropType.EDITOR_SAVE_CODE = "editorSaveCode";
            HostInteropType.EDITOR_SAVE_FILE = "editorSaveFile";
            HostInteropType.EDITOR_LOAD_COMPLETE = "editorLoadComplete";
            HostInteropType.EDITOR_CHANGE = "editorChange";
            HostInteropType.EDITOR_GET_USER_PREFS = "editorGetUserPrefs";
            exports_1("default", HostInteropType);
        }
    };
});
//# sourceMappingURL=interop.js.map