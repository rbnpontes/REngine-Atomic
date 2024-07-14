System.register(["./ClientExtensionEventNames", "../interop"], function (exports_1, context_1) {
    "use strict";
    var ClientExtensionEventNames_1, interop_1, EventDispatcher, ServicesProvider, WebViewServicesProvider;
    var __moduleName = context_1 && context_1.id;
    return {
        setters: [
            function (ClientExtensionEventNames_1_1) {
                ClientExtensionEventNames_1 = ClientExtensionEventNames_1_1;
            },
            function (interop_1_1) {
                interop_1 = interop_1_1;
            }
        ],
        execute: function () {
            EventDispatcher = class EventDispatcher {
                constructor() {
                    this.subscriptions = [];
                }
                sendEvent(eventTypeOrWrapped, data) {
                    let eventType;
                    let eventData;
                    if (typeof (eventTypeOrWrapped) == "string") {
                        eventType = eventTypeOrWrapped;
                        eventData = data;
                    }
                    else {
                        const metaData = eventTypeOrWrapped;
                        eventType = metaData._eventType;
                        eventData = metaData._callbackData;
                    }
                    this.subscriptions.forEach(sub => {
                        if (sub.eventName == eventType) {
                            sub.callback(eventData);
                        }
                    });
                }
                subscribeToEvent(eventTypeOrWrapped, callback) {
                    if (callback) {
                        this.subscriptions.push({
                            eventName: eventTypeOrWrapped,
                            callback: callback
                        });
                    }
                    else {
                        this.subscriptions.push({
                            eventName: eventTypeOrWrapped._eventType,
                            callback: eventTypeOrWrapped._callback
                        });
                    }
                }
            };
            exports_1("EventDispatcher", EventDispatcher);
            ServicesProvider = class ServicesProvider {
                constructor() {
                    this.registeredServices = [];
                }
                register(service) {
                    this.registeredServices.push(service);
                }
                unregister(service) {
                    var index = this.registeredServices.indexOf(service, 0);
                    if (index > -1) {
                        this.registeredServices.splice(index, 1);
                    }
                }
            };
            WebViewServicesProvider = class WebViewServicesProvider extends ServicesProvider {
                constructor() {
                    super(...arguments);
                    this.projectPreferences = {};
                    this.applicationPreferences = {};
                }
                setPreferences(projectPreferences, applicationPreferences) {
                    this.projectPreferences = projectPreferences || this.projectPreferences;
                    this.applicationPreferences = applicationPreferences || this.applicationPreferences;
                }
                subscribeToEvents(eventDispatcher) {
                    eventDispatcher.subscribeToEvent(ClientExtensionEventNames_1.default.CodeLoadedEvent, (ev) => this.codeLoaded(ev));
                    eventDispatcher.subscribeToEvent(ClientExtensionEventNames_1.default.ConfigureEditorEvent, (ev) => this.configureEditor(ev));
                    eventDispatcher.subscribeToEvent(ClientExtensionEventNames_1.default.ResourceRenamedEvent, (ev) => this.renameResource(ev));
                    eventDispatcher.subscribeToEvent(ClientExtensionEventNames_1.default.ResourceDeletedEvent, (ev) => this.deleteResource(ev));
                    eventDispatcher.subscribeToEvent(ClientExtensionEventNames_1.default.CodeSavedEvent, (ev) => this.saveCode(ev));
                    eventDispatcher.subscribeToEvent(ClientExtensionEventNames_1.default.PreferencesChangedEvent, (ev) => this.preferencesChanged(ev));
                    eventDispatcher.subscribeToEvent(ClientExtensionEventNames_1.default.FormatCodeEvent, (ev) => this.formatCode());
                }
                codeLoaded(ev) {
                    this.registeredServices.forEach((service) => {
                        try {
                            if (service.codeLoaded) {
                                service.codeLoaded(ev);
                            }
                        }
                        catch (e) {
                            alert(`Extension Error:\n Error detected in extension ${service.name}\n \n ${e.stack}`);
                        }
                    });
                }
                saveCode(ev) {
                    this.registeredServices.forEach((service) => {
                        try {
                            if (service.save) {
                                service.save(ev);
                            }
                        }
                        catch (e) {
                            alert(`Error detected in extension ${service.name}\n \n ${e.stack}`);
                        }
                    });
                }
                deleteResource(ev) {
                    this.registeredServices.forEach((service) => {
                        try {
                            if (service.delete) {
                                service.delete(ev);
                            }
                        }
                        catch (e) {
                            alert(`Error detected in extension ${service.name}\n \n ${e.stack}`);
                        }
                    });
                }
                renameResource(ev) {
                    this.registeredServices.forEach((service) => {
                        try {
                            if (service.rename) {
                                service.rename(ev);
                            }
                        }
                        catch (e) {
                            alert(`Error detected in extension ${service.name}\n \n ${e.stack}`);
                        }
                    });
                }
                formatCode() {
                    this.registeredServices.forEach((service) => {
                        try {
                            if (service.formatCode) {
                                service.formatCode();
                            }
                        }
                        catch (e) {
                            alert(`Error detected in extension ${service.name}\n \n ${e.stack}`);
                        }
                    });
                }
                configureEditor(ev) {
                    this.registeredServices.forEach((service) => {
                        try {
                            if (service.configureEditor) {
                                service.configureEditor(ev);
                            }
                        }
                        catch (e) {
                            alert(`Extension Error:\n Error detected in extension ${service.name}\n \n ${e.stack}`);
                        }
                    });
                }
                preferencesChanged(prefs) {
                    this.registeredServices.forEach((service) => {
                        try {
                            if (service.preferencesChanged) {
                                service.preferencesChanged(prefs);
                            }
                        }
                        catch (e) {
                            alert(`Extension Error:\n Error detected in extension ${service.name}\n \n ${e.stack}`);
                        }
                    });
                }
                getHostInterop() {
                    return interop_1.default.getInstance();
                }
                getUserPreference(settingsGroup, preferenceName, defaultValue) {
                    if (this.projectPreferences) {
                        let prefs = this.projectPreferences[settingsGroup];
                        if (prefs) {
                            return prefs[preferenceName] || defaultValue;
                        }
                    }
                    return defaultValue;
                }
                getApplicationPreference(settingsGroup, preferenceName, defaultValue) {
                    if (this.applicationPreferences) {
                        let prefs = this.applicationPreferences[settingsGroup];
                        if (prefs) {
                            return prefs[preferenceName] || defaultValue;
                        }
                    }
                    return defaultValue;
                }
            };
            exports_1("WebViewServicesProvider", WebViewServicesProvider);
        }
    };
});
//# sourceMappingURL=ClientExtensionServices.js.map