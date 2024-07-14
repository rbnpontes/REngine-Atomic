System.register([], function (exports_1, context_1) {
    "use strict";
    var Kind;
    var __moduleName = context_1 && context_1.id;
    return {
        setters: [],
        execute: function () {
            Kind = class Kind {
                static convertKind(kind) {
                    switch (kind) {
                        case Kind.primitiveType:
                        case Kind.keyword:
                            return monaco.languages.CompletionItemKind.Keyword;
                        case Kind.variable:
                        case Kind.localVariable:
                            return monaco.languages.CompletionItemKind.Variable;
                        case Kind.memberVariable:
                        case Kind.memberGetAccessor:
                        case Kind.memberSetAccessor:
                            return monaco.languages.CompletionItemKind.Field;
                        case Kind.function:
                        case Kind.memberFunction:
                        case Kind.constructSignature:
                        case Kind.callSignature:
                        case Kind.indexSignature:
                            return monaco.languages.CompletionItemKind.Function;
                        case Kind.enum:
                            return monaco.languages.CompletionItemKind.Enum;
                        case Kind.module:
                            return monaco.languages.CompletionItemKind.Module;
                        case Kind.class:
                            return monaco.languages.CompletionItemKind.Class;
                        case Kind.interface:
                            return monaco.languages.CompletionItemKind.Interface;
                        case Kind.warning:
                            return monaco.languages.CompletionItemKind.File;
                    }
                    return monaco.languages.CompletionItemKind.Property;
                }
            };
            Kind.unknown = "";
            Kind.keyword = "keyword";
            Kind.script = "script";
            Kind.module = "module";
            Kind.class = "class";
            Kind.interface = "interface";
            Kind.type = "type";
            Kind.enum = "enum";
            Kind.variable = "var";
            Kind.localVariable = "local var";
            Kind.function = "function";
            Kind.localFunction = "local function";
            Kind.memberFunction = "method";
            Kind.memberGetAccessor = "getter";
            Kind.memberSetAccessor = "setter";
            Kind.memberVariable = "property";
            Kind.constructorImplementation = "constructor";
            Kind.callSignature = "call";
            Kind.indexSignature = "index";
            Kind.constructSignature = "construct";
            Kind.parameter = "parameter";
            Kind.typeParameter = "type parameter";
            Kind.primitiveType = "primitive type";
            Kind.label = "label";
            Kind.alias = "alias";
            Kind.const = "const";
            Kind.let = "let";
            Kind.warning = "warning";
            exports_1("Kind", Kind);
        }
    };
});
//# sourceMappingURL=tsLanguageSupport.js.map