System.register([], function (exports_1, context_1) {
    "use strict";
    var ClientExtensionEventNames;
    var __moduleName = context_1 && context_1.id;
    return {
        setters: [],
        execute: function () {
            ClientExtensionEventNames = class ClientExtensionEventNames {
            };
            ClientExtensionEventNames.CodeLoadedEvent = "CodeLoadedEvent";
            ClientExtensionEventNames.ConfigureEditorEvent = "ConfigureEditorEvent";
            ClientExtensionEventNames.CodeSavedEvent = "CodeSavedEvent";
            ClientExtensionEventNames.ResourceRenamedEvent = "ResourceRenamedEvent";
            ClientExtensionEventNames.ResourceDeletedEvent = "ResourceDeletedEvent";
            ClientExtensionEventNames.PreferencesChangedEvent = "PreferencesChangedEvent";
            ClientExtensionEventNames.FormatCodeEvent = "FormatCodeEvent";
            exports_1("default", ClientExtensionEventNames);
        }
    };
});
//# sourceMappingURL=ClientExtensionEventNames.js.map