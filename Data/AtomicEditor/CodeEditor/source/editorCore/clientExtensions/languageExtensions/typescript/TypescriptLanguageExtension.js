System.register(["./workerprocess/workerProcessTypes", "../../ClientExtensionEventNames", "./tsLanguageSupport"], function (exports_1, context_1) {
    "use strict";
    var WorkerProcessTypes, ClientExtensionEventNames_1, tsLanguageSupport, TypescriptLanguageExtension, BuiltinServiceProviderOverride, CustomCompletionProvider, CustomHoverProvider, CustomSignatureProvider;
    var __moduleName = context_1 && context_1.id;
    return {
        setters: [
            function (WorkerProcessTypes_1) {
                WorkerProcessTypes = WorkerProcessTypes_1;
            },
            function (ClientExtensionEventNames_1_1) {
                ClientExtensionEventNames_1 = ClientExtensionEventNames_1_1;
            },
            function (tsLanguageSupport_1) {
                tsLanguageSupport = tsLanguageSupport_1;
            }
        ],
        execute: function () {
            TypescriptLanguageExtension = class TypescriptLanguageExtension {
                constructor() {
                    this.name = "ClientTypescriptLanguageExtension";
                    this.description = "This extension handles typescript language features such as completion, compilation, etc.";
                    this.active = false;
                    this.fullCompile = true;
                }
                initialize(serviceLocator) {
                    this.serviceLocator = serviceLocator;
                    serviceLocator.clientServices.register(this);
                }
                isValidFiletype(path) {
                    let ext = path.split(".").pop();
                    return ext == "ts" || ext == "js";
                }
                isJsFile(path) {
                    let ext = path.split(".").pop();
                    return ext == "js";
                }
                isTranspiledJsFile(path, tsconfig) {
                    if (this.isJsFile(path)) {
                        const tsFilename = path.replace(/\.js$/, ".ts");
                        return tsconfig.files.find(f => f.endsWith(tsFilename)) != null;
                    }
                    return false;
                }
                workerRequest(responseChannel, message) {
                    let worker = this.worker;
                    return new Promise((resolve, reject) => {
                        const responseCallback = function (e) {
                            if (e.data.command == responseChannel) {
                                worker.port.removeEventListener("message", responseCallback);
                                resolve(e.data);
                            }
                        };
                        this.worker.port.addEventListener("message", responseCallback);
                        this.worker.port.postMessage(message);
                    });
                }
                configureEditor(ev) {
                    if (this.isValidFiletype(ev.filename)) {
                        let editor = ev.editor;
                        this.editor = editor;
                        this.active = true;
                        this.overrideBuiltinServiceProviders();
                        this.serviceLocator.clientServices.getHostInterop().addCustomHostRoutine("TypeScript_DoFullCompile", (jsonTsConfig) => {
                            let tsConfig = JSON.parse(jsonTsConfig);
                            this.doFullCompile(tsConfig);
                        });
                        this.filename = ev.filename;
                        if (this.isJsFile(ev.filename)) {
                            monaco.languages.typescript.javascriptDefaults.setCompilerOptions({
                                noEmit: true,
                                noResolve: true,
                                allowNonTsExtensions: true,
                                noLib: true,
                                target: monaco.languages.typescript.ScriptTarget.ES5
                            });
                            monaco.languages.typescript.javascriptDefaults.setDiagnosticsOptions({
                                noSemanticValidation: true,
                                noSyntaxValidation: true
                            });
                            monaco.languages.registerCompletionItemProvider("javascript", new CustomCompletionProvider(this));
                            monaco.languages.registerHoverProvider("javascript", new CustomHoverProvider(this));
                            monaco.languages.registerSignatureHelpProvider("javascript", new CustomSignatureProvider(this));
                        }
                        else {
                            monaco.languages.register({
                                id: "atomic-ts"
                            });
                            monaco.languages.typescript.typescriptDefaults.setCompilerOptions({
                                noEmit: true,
                                noResolve: true,
                                noLib: true,
                                target: monaco.languages.typescript.ScriptTarget.ES5
                            });
                            monaco.languages.typescript.typescriptDefaults.setDiagnosticsOptions({
                                noSemanticValidation: true,
                                noSyntaxValidation: true
                            });
                            monaco.languages.registerCompletionItemProvider("typescript", new CustomCompletionProvider(this));
                            monaco.languages.registerSignatureHelpProvider("typescript", new CustomSignatureProvider(this));
                            monaco.languages.registerHoverProvider("typescript", new CustomHoverProvider(this));
                        }
                    }
                }
                overrideBuiltinServiceProviders() {
                    monaco.languages.registerSignatureHelpProvider = ((original) => {
                        return function (languageId, provider) {
                            if (provider["isOverride"]) {
                                return original(languageId, provider);
                            }
                        };
                    })(monaco.languages.registerSignatureHelpProvider);
                    monaco.languages.registerCompletionItemProvider = ((original) => {
                        return function (languageId, provider) {
                            if (provider["isOverride"]) {
                                return original(languageId, provider);
                            }
                        };
                    })(monaco.languages.registerCompletionItemProvider);
                    monaco.languages.registerHoverProvider = ((original) => {
                        return function (languageId, provider) {
                            if (provider["isOverride"]) {
                                return original(languageId, provider);
                            }
                        };
                    })(monaco.languages.registerHoverProvider);
                }
                codeLoaded(ev) {
                    if (this.isValidFiletype(ev.filename)) {
                        this.buildWorker();
                        let tsConfig = JSON.parse(window["TypeScriptLanguageExtension"]["tsConfig"]);
                        let model = this.editor.getModel();
                        let handle;
                        model.onDidChangeContent(() => {
                            clearTimeout(handle);
                            handle = setTimeout(() => this.getAnnotations(), 500);
                        });
                        this.worker.port.postMessage({
                            command: WorkerProcessTypes.Connect,
                            sender: "Typescript Language Extension",
                            filename: ev.filename,
                            tsConfig: tsConfig,
                            code: ev.code
                        });
                        this.preferencesChanged();
                    }
                }
                handleWorkerMessage(e) {
                    switch (e.data.command) {
                        case WorkerProcessTypes.Message:
                            console.log(e.data.message);
                            break;
                        case WorkerProcessTypes.Alert:
                            alert(e.data.message);
                            break;
                        case WorkerProcessTypes.AnnotationsUpdated:
                            this.setAnnotations(e.data);
                            break;
                        case WorkerProcessTypes.SaveFile:
                            this.saveFile(e.data);
                            break;
                        case WorkerProcessTypes.DisplayFullCompileResults:
                            this.displayFullCompileResults(e.data);
                            break;
                    }
                }
                saveFile(event) {
                    if (this.active) {
                        this.serviceLocator.clientServices.getHostInterop().saveFile(event.filename, event.code);
                    }
                }
                formatCode() {
                    if (this.active) {
                        const action = this.editor.getAction("editor.action.format");
                        if (action && action.isSupported()) {
                            let wasEmpty = false;
                            let cursorPosition;
                            if (this.editor.getSelection().isEmpty()) {
                                wasEmpty = true;
                                cursorPosition = this.editor.getPosition();
                                this.editor.setSelection(this.editor.getModel().getFullModelRange());
                            }
                            action.run().then(() => {
                                if (wasEmpty && cursorPosition) {
                                    this.editor.setPosition(cursorPosition);
                                }
                            });
                        }
                    }
                }
                getAnnotations() {
                    const message = {
                        command: WorkerProcessTypes.GetAnnotations,
                        code: this.editor.getModel().getValue(),
                        filename: this.filename,
                        fileExt: null,
                        editor: null
                    };
                    this.worker.port.postMessage(message);
                }
                setAnnotations(event) {
                    let model = this.editor.getModel();
                    let markers = event.annotations
                        .filter(ann => ann.start != undefined)
                        .map(ann => {
                        return {
                            code: ann.code,
                            severity: monaco.Severity.Error,
                            message: ann.message,
                            startLineNumber: model.getPositionAt(ann.start).lineNumber,
                            startColumn: model.getPositionAt(ann.start).column,
                            endLineNumber: model.getPositionAt(ann.start + ann.length).lineNumber,
                            endColumn: model.getPositionAt(ann.start + ann.length).column
                        };
                    });
                    monaco.editor.setModelMarkers(this.editor.getModel(), "typescript", markers);
                }
                buildWorker() {
                    if (!this.worker) {
                        this.worker = new SharedWorker("./source/editorCore/clientExtensions/languageExtensions/typescript/workerprocess/workerLoader.js");
                        this.worker.port.addEventListener("message", this.handleWorkerMessage.bind(this), false);
                        addEventListener("beforeunload", () => {
                            this.worker.port.postMessage({ command: WorkerProcessTypes.Disconnect });
                        });
                        this.worker.port.start();
                    }
                }
                save(ev) {
                    if (this.active && this.isValidFiletype(ev.filename)) {
                        const message = {
                            command: ClientExtensionEventNames_1.default.CodeSavedEvent,
                            filename: ev.filename,
                            fileExt: ev.fileExt,
                            code: ev.code,
                            editor: null
                        };
                        this.worker.port.postMessage(message);
                    }
                }
                delete(ev) {
                    if (this.active && this.isValidFiletype(ev.path)) {
                        const message = {
                            command: ClientExtensionEventNames_1.default.ResourceDeletedEvent,
                            path: ev.path
                        };
                        this.worker.port.postMessage(message);
                    }
                }
                rename(ev) {
                    if (this.active && this.isValidFiletype(ev.path)) {
                        const message = {
                            command: ClientExtensionEventNames_1.default.ResourceRenamedEvent,
                            path: ev.path,
                            newPath: ev.newPath
                        };
                        this.worker.port.postMessage(message);
                    }
                }
                preferencesChanged() {
                    if (this.active) {
                        let compileOnSave = this.serviceLocator.clientServices.getUserPreference("HostTypeScriptLanguageExtension", "CompileOnSave", true);
                        const message = {
                            command: WorkerProcessTypes.SetPreferences,
                            preferences: {
                                compileOnSave: compileOnSave
                            }
                        };
                        this.worker.port.postMessage(message);
                    }
                }
                doFullCompile(tsConfig) {
                    if (this.active) {
                        const message = {
                            command: WorkerProcessTypes.DoFullCompile,
                            tsConfig: tsConfig
                        };
                        this.worker.port.postMessage(message);
                    }
                }
                displayFullCompileResults(results) {
                    let messageArray = results.annotations.map((result) => {
                        return `${result.text} at line ${result.row} col ${result.column} in ${result.file}`;
                    });
                    window.atomicQueryPromise("TypeScript.DisplayCompileResults", results);
                }
            };
            exports_1("default", TypescriptLanguageExtension);
            BuiltinServiceProviderOverride = class BuiltinServiceProviderOverride {
                constructor(extension) {
                    this.isOverride = true;
                    this.extension = extension;
                }
            };
            CustomCompletionProvider = class CustomCompletionProvider extends BuiltinServiceProviderOverride {
                get triggerCharacters() {
                    return ["."];
                }
                provideCompletionItems(model, position, token) {
                    const message = {
                        command: WorkerProcessTypes.MonacoProvideCompletionItems,
                        uri: this.extension.filename,
                        source: model.getValue(),
                        positionOffset: model.getOffsetAt(position)
                    };
                    return this.extension.workerRequest(WorkerProcessTypes.MonacoProvideCompletionItemsResponse, message)
                        .then((e) => {
                        return e.completions.map(completion => {
                            completion.kind = tsLanguageSupport.Kind.convertKind(completion.completionKind);
                            return completion;
                        }).filter(completion => {
                            return completion.kind != monaco.languages.CompletionItemKind.Keyword;
                        });
                    });
                }
                resolveCompletionItem(item, token) {
                    const message = {
                        command: WorkerProcessTypes.MonacoResolveCompletionItem,
                        item: item
                    };
                    return this.extension.workerRequest(WorkerProcessTypes.MonacoResolveCompletionItemResponse, message)
                        .then((e) => {
                        return e;
                    });
                }
            };
            CustomHoverProvider = class CustomHoverProvider extends BuiltinServiceProviderOverride {
                _offsetToPosition(uri, offset) {
                    let model = monaco.editor.getModel(uri);
                    return model.getPositionAt(offset);
                }
                _textSpanToRange(uri, span) {
                    let p1 = this._offsetToPosition(uri, span.start);
                    let p2 = this._offsetToPosition(uri, span.start + span.length);
                    let { lineNumber: startLineNumber, column: startColumn } = p1;
                    let { lineNumber: endLineNumber, column: endColumn } = p2;
                    return { startLineNumber, startColumn, endLineNumber, endColumn };
                }
                provideHover(model, position) {
                    let resource = model.uri;
                    const message = {
                        command: WorkerProcessTypes.MonacoGetQuickInfo,
                        uri: this.extension.filename,
                        source: model.getValue(),
                        positionOffset: model.getOffsetAt(position)
                    };
                    return this.extension.workerRequest(WorkerProcessTypes.MonacoGetQuickInfoResponse, message)
                        .then((e) => {
                        if (e.contents) {
                            return {
                                range: this._textSpanToRange(resource, e.textSpan),
                                contents: [e.documentation, { language: "typescript", value: e.contents }]
                            };
                        }
                    });
                }
            };
            CustomSignatureProvider = class CustomSignatureProvider extends BuiltinServiceProviderOverride {
                constructor() {
                    super(...arguments);
                    this.signatureHelpTriggerCharacters = ["(", ","];
                }
                provideSignatureHelp(model, position) {
                    let resource = model.uri;
                    const message = {
                        command: WorkerProcessTypes.MonacoGetSignature,
                        uri: this.extension.filename,
                        source: model.getValue(),
                        positionOffset: model.getOffsetAt(position)
                    };
                    return this.extension.workerRequest(WorkerProcessTypes.MonacoGetSignatureResponse, message)
                        .then((e) => {
                        if (e.signatures) {
                            let result = {
                                signatures: e.signatures,
                                activeSignature: e.selectedItemIndex,
                                activeParameter: e.argumentIndex
                            };
                            return result;
                        }
                    });
                }
            };
            exports_1("CustomSignatureProvider", CustomSignatureProvider);
        }
    };
});
//# sourceMappingURL=TypescriptLanguageExtension.js.map