System.register(["../../../../modules/typescript", "./TypescriptLanguageService", "./workerProcessTypes", "../../../ClientExtensionEventNames"], function (exports_1, context_1) {
    "use strict";
    var ts, TypescriptLanguageService_1, WorkerProcessTypes, ClientExtensionEventNames_1, WebFileSystem, TypescriptLanguageServiceWebWorker;
    var __moduleName = context_1 && context_1.id;
    function getResource(codeUrl) {
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
    function getFileResource(filename) {
        return getResource(`atomic://${filename}`);
    }
    return {
        setters: [
            function (ts_1) {
                ts = ts_1;
            },
            function (TypescriptLanguageService_1_1) {
                TypescriptLanguageService_1 = TypescriptLanguageService_1_1;
            },
            function (WorkerProcessTypes_1) {
                WorkerProcessTypes = WorkerProcessTypes_1;
            },
            function (ClientExtensionEventNames_1_1) {
                ClientExtensionEventNames_1 = ClientExtensionEventNames_1_1;
            }
        ],
        execute: function () {
            WebFileSystem = class WebFileSystem {
                constructor() {
                    this.fileCache = {};
                }
                setCommunicationPort(port) {
                    this.communicationPort = port;
                }
                fileExists(filename) {
                    return this.fileCache[filename] != null;
                }
                cacheFile(filename, file) {
                    this.fileCache[filename] = file;
                }
                getFile(filename) {
                    return this.fileCache[filename];
                }
                writeFile(filename, contents) {
                    if (this.communicationPort) {
                        const fileExt = filename.indexOf(".") != -1 ? filename.split(".").pop() : "";
                        let message = {
                            command: WorkerProcessTypes.SaveFile,
                            filename: filename,
                            code: contents,
                            fileExt: fileExt,
                            editor: null
                        };
                        this.communicationPort.postMessage(message);
                    }
                }
                getCurrentDirectory() {
                    return "";
                }
            };
            TypescriptLanguageServiceWebWorker = class TypescriptLanguageServiceWebWorker {
                constructor() {
                    this.connections = 0;
                    this.languageService = null;
                    this.projectLoaded = false;
                    this.options = {
                        compileOnSave: false
                    };
                    this.tsConfig = null;
                    this.fs = new WebFileSystem();
                }
                connect(e) {
                    let port = e.ports[0];
                    this.connections++;
                    port.addEventListener("message", (e) => {
                        try {
                            switch (e.data.command) {
                                case WorkerProcessTypes.Connect:
                                    this.handleHELO(port, e.data);
                                    break;
                                case WorkerProcessTypes.Disconnect:
                                    this.handleCLOSE(port, e.data);
                                    break;
                                case ClientExtensionEventNames_1.default.CodeSavedEvent:
                                    this.handleSave(port, e.data);
                                    break;
                                case ClientExtensionEventNames_1.default.ResourceRenamedEvent:
                                    this.handleRename(port, e.data);
                                    break;
                                case ClientExtensionEventNames_1.default.ResourceDeletedEvent:
                                    this.handleDelete(port, e.data);
                                    break;
                                case WorkerProcessTypes.GetAnnotations:
                                    this.handleGetAnnotations(port, e.data);
                                    break;
                                case WorkerProcessTypes.SetPreferences:
                                    this.setPreferences(port, e.data);
                                    break;
                                case WorkerProcessTypes.DoFullCompile:
                                    this.doFullCompile(port, e.data);
                                    break;
                                case WorkerProcessTypes.MonacoProvideCompletionItems:
                                    this.monacoHandleProvideCompletionItems(port, e.data);
                                    break;
                                case WorkerProcessTypes.MonacoResolveCompletionItem:
                                    this.monacoHandleResolveCompletionItem(port, e.data);
                                    break;
                                case WorkerProcessTypes.MonacoGetQuickInfo:
                                    this.monacoGetQuickInfo(port, e.data);
                                    break;
                                case WorkerProcessTypes.MonacoGetSignature:
                                    this.monacoGetSignature(port, e.data);
                                    break;
                            }
                        }
                        catch (e) {
                            port.postMessage({ command: WorkerProcessTypes.Message, message: `Error in TypescriptLanguageServiceWebWorker: ${e}\n${e.stack}` });
                        }
                    }, false);
                    port.start();
                }
                reset() {
                    this.languageService.reset();
                    this.projectLoaded = false;
                }
                loadProjectFiles() {
                    let promises = [];
                    let existingFiles = this.languageService.getProjectFiles();
                    existingFiles.forEach((f) => {
                        if (this.tsConfig.files.indexOf(f) == -1) {
                            this.languageService.deleteProjectFile(f);
                        }
                    });
                    this.tsConfig.files.forEach((f) => {
                        if (existingFiles.indexOf(f) == -1) {
                            promises.push(getFileResource(f).then((code) => {
                                this.languageService.addProjectFile(f, code);
                            }));
                        }
                    });
                    return Promise.all(promises).then(() => {
                        this.projectLoaded = true;
                    });
                }
                handleHELO(port, eventData) {
                    this.tsConfig = eventData.tsConfig;
                    if (!this.languageService) {
                        this.languageService = new TypescriptLanguageService_1.TypescriptLanguageService(this.fs, this.tsConfig);
                    }
                    const fn = this.resolvePartialFilename(eventData.filename);
                    this.languageService.addProjectFile(fn, eventData.code);
                    this.loadProjectFiles().then(() => {
                        this.handleGetAnnotations(port, eventData);
                    });
                }
                handleCLOSE(port, eventData) {
                    this.connections--;
                    if (this.connections <= 0) {
                        this.reset();
                    }
                }
                resolvePartialFilename(partial) {
                    let result = this.tsConfig.files.find(fn => fn.endsWith(partial));
                    return result || partial;
                }
                handleGetAnnotations(port, eventData) {
                    let filename = this.resolvePartialFilename(eventData.filename);
                    this.languageService.updateProjectFile(filename, eventData.code);
                    let message = {
                        command: WorkerProcessTypes.AnnotationsUpdated,
                        annotations: this.languageService.getDiagnostics(filename)
                    };
                    port.postMessage(message);
                }
                handleSave(port, eventData) {
                    let filename = this.resolvePartialFilename(eventData.filename);
                    this.languageService.updateProjectFile(filename, eventData.code);
                    this.handleGetAnnotations(port, eventData);
                    if (this.options.compileOnSave) {
                        this.fs.setCommunicationPort(port);
                        let results = this.languageService.compile([filename]);
                        this.fs.setCommunicationPort(null);
                    }
                }
                doFullCompile(port, eventData) {
                    this.tsConfig = eventData.tsConfig;
                    this.languageService.setTsConfig(eventData.tsConfig);
                    this.fs.setCommunicationPort(port);
                    let results = [];
                    let start = Date.now();
                    this.languageService.compile([], (filename, errors) => {
                        if (errors.length > 0) {
                            results = results.concat(errors.map(diagnostic => {
                                let row = 0;
                                let char = 0;
                                let filename = "";
                                if (diagnostic.file) {
                                    let lineChar = diagnostic.file.getLineAndCharacterOfPosition(diagnostic.start);
                                    row = lineChar.line;
                                    char = lineChar.character;
                                    filename = diagnostic.file.fileName;
                                }
                                let message = ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n");
                                return {
                                    file: filename,
                                    row: row,
                                    column: char,
                                    text: message,
                                    type: diagnostic.category == 1 ? "error" : "warning"
                                };
                            }));
                        }
                        else {
                            results.push({
                                file: filename,
                                row: 0,
                                column: 0,
                                text: "Success",
                                type: "success"
                            });
                        }
                    });
                    let duration = Date.now() - start;
                    let message = {
                        command: WorkerProcessTypes.DisplayFullCompileResults,
                        annotations: results,
                        compilerOptions: this.tsConfig.compilerOptions,
                        duration: duration
                    };
                    this.fs.setCommunicationPort(null);
                    port.postMessage(message);
                }
                monacoHandleProvideCompletionItems(port, eventData) {
                    let filename = this.resolvePartialFilename(eventData.uri);
                    let sourceFile = this.languageService.updateProjectFile(filename, eventData.source);
                    let completions = this.languageService.getCompletions(filename, eventData.positionOffset);
                    let message = {
                        command: WorkerProcessTypes.MonacoProvideCompletionItemsResponse,
                        completions: []
                    };
                    if (completions) {
                        message.completions = completions.entries.map((completion) => {
                            let completionItem = {
                                label: completion.name,
                                uri: eventData.uri,
                                sortText: completion.sortText,
                                completionKind: completion.kind,
                                kind: -1,
                                positionOffset: eventData.positionOffset
                            };
                            return completionItem;
                        });
                    }
                    port.postMessage(message);
                }
                monacoHandleResolveCompletionItem(port, data) {
                    let filename = this.resolvePartialFilename(data.item.uri);
                    const details = this.languageService.getCompletionEntryDetails(filename, data.item.positionOffset, data.item.label);
                    let message = {
                        command: WorkerProcessTypes.MonacoResolveCompletionItemResponse,
                        label: data.item.label,
                        kind: data.item.kind,
                        detail: "",
                        documentation: ""
                    };
                    if (details) {
                        message.label = details.name;
                        message.kind = data.item.kind;
                        message.detail = ts.displayPartsToString(details.displayParts);
                        message.documentation = ts.displayPartsToString(details.documentation);
                    }
                    port.postMessage(message);
                }
                monacoGetQuickInfo(port, eventData) {
                    let filename = this.resolvePartialFilename(eventData.uri);
                    let quickInfo = this.languageService.getQuickInfoAtPosition(filename, eventData.positionOffset);
                    let message = {
                        command: WorkerProcessTypes.MonacoGetQuickInfoResponse
                    };
                    if (quickInfo) {
                        message.contents = quickInfo.contents;
                        message.textSpan = quickInfo.range;
                        message.documentation = quickInfo.documentation;
                    }
                    port.postMessage(message);
                }
                monacoGetSignature(port, eventData) {
                    let filename = this.resolvePartialFilename(eventData.uri);
                    let sourceFile = this.languageService.updateProjectFile(filename, eventData.source);
                    let signatureInfo = this.languageService.getSignatureHelpItems(filename, eventData.positionOffset);
                    let message = {
                        command: WorkerProcessTypes.MonacoGetSignatureResponse,
                    };
                    if (signatureInfo) {
                        message.selectedItemIndex = signatureInfo.selectedItemIndex;
                        message.argumentIndex = signatureInfo.argumentIndex;
                        message.signatures = [];
                        signatureInfo.items.forEach(item => {
                            let signature = {
                                label: "",
                                documentation: null,
                                parameters: []
                            };
                            signature.label += ts.displayPartsToString(item.prefixDisplayParts);
                            item.parameters.forEach((p, i, a) => {
                                let label = ts.displayPartsToString(p.displayParts);
                                let parameter = {
                                    label: label,
                                    documentation: ts.displayPartsToString(p.documentation)
                                };
                                signature.label += label;
                                signature.parameters.push(parameter);
                                if (i < a.length - 1) {
                                    signature.label += ts.displayPartsToString(item.separatorDisplayParts);
                                }
                            });
                            signature.label += ts.displayPartsToString(item.suffixDisplayParts);
                            message.signatures.push(signature);
                        });
                    }
                    port.postMessage(message);
                }
                handleDelete(port, eventData) {
                    let filename = this.resolvePartialFilename(eventData.path);
                    this.languageService.deleteProjectFile(filename);
                }
                handleRename(port, eventData) {
                    let fromFn = this.resolvePartialFilename(eventData.path);
                    let toFn = fromFn.replace(eventData.path, eventData.newPath);
                    this.languageService.renameProjectFile(fromFn, toFn);
                }
                setPreferences(port, eventData) {
                    this.options.compileOnSave = eventData.preferences.compileOnSave;
                }
            };
            exports_1("default", TypescriptLanguageServiceWebWorker);
        }
    };
});
//# sourceMappingURL=TypescriptLanguageServiceWebWorker.js.map