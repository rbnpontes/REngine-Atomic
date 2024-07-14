System.register(["./ClientExtensionServices", "./languageExtensions/typescript/TypescriptLanguageExtension", "./languageExtensions/javascript/JavascriptLanguageExtension", "./languageExtensions/turbobadger/TurboBadgerLanguageExtension"], function (exports_1, context_1) {
    "use strict";
    var ClientExtensionServices, TypescriptLanguageExtension_1, JavascriptLanguageExtension_1, TurboBadgerLanguageExtension_1, ClientServiceLocatorType, serviceLocator;
    var __moduleName = context_1 && context_1.id;
    return {
        setters: [
            function (ClientExtensionServices_1) {
                ClientExtensionServices = ClientExtensionServices_1;
            },
            function (TypescriptLanguageExtension_1_1) {
                TypescriptLanguageExtension_1 = TypescriptLanguageExtension_1_1;
            },
            function (JavascriptLanguageExtension_1_1) {
                JavascriptLanguageExtension_1 = JavascriptLanguageExtension_1_1;
            },
            function (TurboBadgerLanguageExtension_1_1) {
                TurboBadgerLanguageExtension_1 = TurboBadgerLanguageExtension_1_1;
            }
        ],
        execute: function () {
            ClientServiceLocatorType = class ClientServiceLocatorType {
                constructor() {
                    this.eventDispatcher = new ClientExtensionServices.EventDispatcher();
                    this.clientServices = new ClientExtensionServices.WebViewServicesProvider();
                    this.clientServices.subscribeToEvents(this);
                }
                loadService(service) {
                    try {
                        service.initialize(this);
                    }
                    catch (e) {
                        alert(`Extension Error:\n Error detected in extension ${service.name}\n \n ${e.stack}`);
                    }
                }
                sendEvent(eventTypeOrWrapped, data) {
                    if (this.eventDispatcher) {
                        if (data) {
                            this.eventDispatcher.sendEvent(eventTypeOrWrapped, data);
                        }
                        else {
                            this.eventDispatcher.sendEvent(eventTypeOrWrapped);
                        }
                    }
                }
                subscribeToEvent(eventTypeOrWrapped, callback) {
                    if (this.eventDispatcher) {
                        if (callback) {
                            this.eventDispatcher.subscribeToEvent(eventTypeOrWrapped, callback);
                        }
                        else {
                            this.eventDispatcher.subscribeToEvent(eventTypeOrWrapped);
                        }
                    }
                }
            };
            exports_1("ClientServiceLocatorType", ClientServiceLocatorType);
            serviceLocator = new ClientServiceLocatorType();
            exports_1("default", serviceLocator);
            serviceLocator.loadService(new TypescriptLanguageExtension_1.default());
            serviceLocator.loadService(new JavascriptLanguageExtension_1.default());
            serviceLocator.loadService(new TurboBadgerLanguageExtension_1.default());
        }
    };
});
//# sourceMappingURL=ServiceLocator.js.map