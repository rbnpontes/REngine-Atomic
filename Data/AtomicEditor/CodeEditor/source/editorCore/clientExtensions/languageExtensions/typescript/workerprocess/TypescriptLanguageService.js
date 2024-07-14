System.register(["../../../../modules/typescript"], function (exports_1, context_1) {
    "use strict";
    var ts, TypescriptLanguageService;
    var __moduleName = context_1 && context_1.id;
    return {
        setters: [
            function (ts_1) {
                ts = ts_1;
            }
        ],
        execute: function () {
            TypescriptLanguageService = class TypescriptLanguageService {
                constructor(fs, tsconfig) {
                    this.fs = null;
                    this.languageService = null;
                    this.documentRegistry = null;
                    this.compilerOptions = null;
                    this.name = "TypescriptLanguageService";
                    this.projectFiles = [];
                    this.versionMap = {};
                    this.fs = fs;
                    this.setTsConfig(tsconfig);
                    this.documentRegistry = ts.createDocumentRegistry();
                    this.createLanguageService(this.documentRegistry);
                }
                createLanguageService(documentRegistry) {
                    const servicesHost = {
                        getScriptFileNames: () => this.projectFiles,
                        getScriptVersion: (fileName) => this.versionMap[fileName] && this.versionMap[fileName].version.toString(),
                        getScriptSnapshot: (filename) => {
                            const scriptVersion = this.versionMap[filename];
                            if (scriptVersion) {
                                if (scriptVersion.snapshot) {
                                    return scriptVersion.snapshot;
                                }
                                else {
                                    let sourceFile = this.documentRegistry.acquireDocument(filename, this.compilerOptions, ts.ScriptSnapshot.fromString(""), scriptVersion.version.toString());
                                    return ts.ScriptSnapshot.fromString(sourceFile.text);
                                }
                            }
                        },
                        getCurrentDirectory: () => this.fs.getCurrentDirectory(),
                        getCompilationSettings: () => this.compilerOptions,
                        getDefaultLibFileName: (options) => undefined
                    };
                    this.languageService = ts.createLanguageService(servicesHost, documentRegistry);
                }
                setTsConfig(tsConfig) {
                    const stubParseConfigHost = {
                        readDirectory(rootDir, extension, exclude) {
                            return null;
                        },
                        fileExists(fileName) {
                            return false;
                        },
                        readFile(fileName) {
                            return "";
                        }
                    };
                    let cmdLine = ts.parseJsonConfigFileContent(tsConfig, stubParseConfigHost, ".");
                    this.compilerOptions = cmdLine.options;
                    this.compilerOptions.outDir = "DUMMY_OUT_DIR";
                }
                releaseProjectFile(filename) {
                    if (this.versionMap[filename]) {
                        const fileIndex = this.projectFiles.indexOf(filename);
                        if (fileIndex > -1) {
                            this.projectFiles.splice(fileIndex, 1);
                        }
                        delete this.versionMap[filename];
                        this.documentRegistry.releaseDocument(filename, this.compilerOptions);
                    }
                }
                addProjectFile(filename, fileContents) {
                    if (this.projectFiles.indexOf(filename) == -1) {
                        console.log("Added project file: " + filename);
                        if (fileContents == null) {
                            this.versionMap[filename] = {
                                version: 0,
                                snapshot: ts.ScriptSnapshot.fromString(this.fs.getFile(filename))
                            };
                        }
                        else {
                            this.versionMap[filename] = {
                                version: 0,
                                snapshot: ts.ScriptSnapshot.fromString(fileContents)
                            };
                        }
                        this.projectFiles.push(filename);
                        return this.documentRegistry.acquireDocument(filename, this.compilerOptions, this.versionMap[filename].snapshot, "0");
                    }
                    else {
                        return null;
                    }
                }
                updateProjectFile(filename, fileContents) {
                    if (this.projectFiles.indexOf(filename) == -1) {
                        return this.addProjectFile(filename, fileContents);
                    }
                    else {
                        console.log("Updating file: " + filename);
                        this.versionMap[filename].version++;
                        this.versionMap[filename].snapshot = ts.ScriptSnapshot.fromString(fileContents);
                        return this.documentRegistry.updateDocument(filename, this.compilerOptions, this.versionMap[filename].snapshot, this.versionMap[filename].version.toString());
                    }
                }
                getSourceFile(filename) {
                    return this.documentRegistry.acquireDocument(filename, this.compilerOptions, this.versionMap[filename].snapshot, this.versionMap[filename].version.toString());
                }
                updateProjectFileVersionNumber(filename) {
                    this.versionMap[filename].version++;
                    return this.documentRegistry.updateDocument(filename, this.compilerOptions, this.versionMap[filename].snapshot, this.versionMap[filename].version.toString());
                }
                getProjectFiles() {
                    return this.projectFiles;
                }
                getDiagnostics(filename) {
                    let allDiagnostics = this.languageService.getSyntacticDiagnostics(filename);
                    if (filename.endsWith(".ts")) {
                        allDiagnostics = allDiagnostics.concat(this.languageService.getSemanticDiagnostics(filename));
                    }
                    return allDiagnostics.map(diagnostic => {
                        return {
                            message: ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n"),
                            type: diagnostic.category == 1 ? "error" : "warning",
                            start: diagnostic.start,
                            length: diagnostic.length,
                            code: diagnostic.code,
                            source: filename
                        };
                    });
                }
                transpile(fileNames) {
                    fileNames.forEach((fileName) => {
                        console.log(`${this.name}:  Transpiling ${fileName}`);
                        let script = this.fs.getFile(fileName);
                        let diagnostics = [];
                        let result = ts.transpile(script, this.compilerOptions, fileName, diagnostics);
                        if (diagnostics.length) {
                            this.logErrors(diagnostics);
                        }
                        if (diagnostics.length == 0) {
                            this.fs.writeFile(fileName.replace(".ts", ".js"), result);
                        }
                    });
                }
                getPositionOfLineAndCharacter(file, line, character) {
                    return ts.getPositionOfLineAndCharacter(file, line, character);
                }
                getCompletions(filename, pos) {
                    return this.languageService.getCompletionsAtPosition(filename, pos);
                }
                getCompletionEntryDetails(filename, pos, entryname) {
                    return this.languageService.getCompletionEntryDetails(filename, pos, entryname);
                }
                getQuickInfoAtPosition(filename, pos) {
                    let results = this.languageService.getQuickInfoAtPosition(filename, pos);
                    if (results) {
                        return {
                            contents: ts.displayPartsToString(results.displayParts),
                            range: results.textSpan,
                            documentation: ts.displayPartsToString(results.documentation)
                        };
                    }
                    else {
                        return null;
                    }
                }
                getSignatureHelpItems(filename, pos) {
                    let results = this.languageService.getSignatureHelpItems(filename, pos);
                    if (results) {
                        return {
                            selectedItemIndex: results.selectedItemIndex,
                            argumentIndex: results.argumentIndex,
                            items: results.items
                        };
                    }
                    else {
                        return null;
                    }
                }
                compile(files, progress) {
                    let start = new Date().getTime();
                    files.forEach((file) => {
                        this.addProjectFile(file);
                    });
                    this.projectFiles
                        .filter(f => f.endsWith(".js"))
                        .forEach(f => {
                        if (this.projectFiles.indexOf(f.replace(/\.js$/, ".ts")) > -1) {
                            this.releaseProjectFile(f);
                        }
                    });
                    let errors = [];
                    if (files.length == 0) {
                        files = this.projectFiles;
                    }
                    files.forEach(filename => {
                        let currentErrors = this.compileFile(filename);
                        errors = errors.concat(currentErrors);
                        if (progress) {
                            progress(filename, currentErrors);
                        }
                    });
                    console.log(`${this.name}: Compiling complete after ${new Date().getTime() - start} ms`);
                    return errors;
                }
                deleteProjectFile(filepath) {
                    if (this.versionMap[filepath]) {
                        delete this.versionMap[filepath];
                    }
                    let idx = this.projectFiles.indexOf(filepath);
                    if (idx > -1) {
                        console.log(`delete project file from ${filepath}`);
                        this.projectFiles.splice(idx, 1);
                    }
                }
                renameProjectFile(filepath, newpath) {
                    let oldFile = this.versionMap[filepath];
                    if (oldFile) {
                        this.releaseProjectFile(filepath);
                        oldFile.version++;
                        this.versionMap[newpath] = oldFile;
                        this.projectFiles.push(newpath);
                        this.documentRegistry.acquireDocument(newpath, this.compilerOptions, oldFile.snapshot, oldFile.version.toString());
                    }
                }
                reset() {
                    this.projectFiles = [];
                    this.versionMap = {};
                }
                compileFile(filename) {
                    console.log(`${this.name}: Compiling version ${this.versionMap[filename].version} of ${filename}`);
                    try {
                        return this.emitFile(filename);
                    }
                    catch (err) {
                        console.log(`${this.name}: problem encountered compiling ${filename}: ${err}`);
                        return [];
                    }
                }
                emitFile(filename) {
                    let allDiagnostics = [];
                    if (filename.endsWith(".ts")) {
                        let output = this.languageService.getEmitOutput(filename);
                        if (output.emitSkipped) {
                            console.log(`${this.name}: Failure Emitting ${filename}`);
                            allDiagnostics = this.languageService.getCompilerOptionsDiagnostics()
                                .concat(this.languageService.getSyntacticDiagnostics(filename))
                                .concat(this.languageService.getSemanticDiagnostics(filename));
                        }
                        else {
                            output.outputFiles.forEach(o => {
                                this.fs.writeFile(filename.replace(/\.ts$/, ".js"), o.text);
                            });
                        }
                    }
                    return allDiagnostics;
                }
                logErrors(diagnostics) {
                    let msg = [];
                    diagnostics.forEach(diagnostic => {
                        let message = ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n");
                        if (diagnostic.file) {
                            let d = diagnostic.file.getLineAndCharacterOfPosition(diagnostic.start);
                            let line = d.line;
                            let character = d.character;
                            msg.push(`${this.name}:  Error ${diagnostic.file.fileName} (${line + 1},${character + 1}): ${message}`);
                        }
                        else {
                            msg.push(`${this.name}  Error: ${message}`);
                        }
                    });
                    console.log(`TypeScript Errors:\n${msg.join("\n")}`);
                    throw new Error(`TypeScript Errors:\n${msg.join("\n")}`);
                }
            };
            exports_1("TypescriptLanguageService", TypescriptLanguageService);
        }
    };
});
//# sourceMappingURL=TypescriptLanguageService.js.map