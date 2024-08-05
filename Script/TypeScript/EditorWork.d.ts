//
// Copyright (c) 2014-2015, THUNDERBEAST GAMES LLC All rights reserved
// LICENSE: Atomic Game Engine Editor and Tools EULA
// Please see LICENSE_ATOMIC_EDITOR_AND_TOOLS.md in repository root for
// license information: https://github.com/AtomicGameEngine/AtomicGameEngine
//

/// <reference path="../../Artifacts/Build/TypeScript/EngineCore.d.ts" />
/// <reference path="../../Artifacts/Build/TypeScript/EngineEditor.d.ts" />
/// <reference path="../../Artifacts/Build/TypeScript/ToolCore.d.ts" />
/// <reference path="../../Artifacts/Build/TypeScript/WebView.d.ts" />

declare module EngineEditor.Templates {
    // Commented out until the TSDoc gets updated to the latest version of TypeScript
    //export type TemplateType = "component" | "script";
    /**
     * New file defintion
     */
    export interface FileTemplateDefinition {
        /** name to display in the dropdown */
        name: string;
        /** description */
        desc: string;
        /** type of template */
        templateType: string;
        /** file extension */
        ext: string;
        /** file name/path of the source templage file to clone from.  Note, needs to be in the atomic cache */
        filename: string;
    }
}

declare module EngineEditor.Extensions {

    /**
     * Base interface for any editor services.
     */
    export interface EditorServiceExtension {
        /**
         * Unique name of this service
         * @type {string}
         */
        name: string;

        /**
         * Description of this service
         * @type {string}
         */
        description: string;

    }

    /**
     * Base Service Event Listener.  Attach descendents of these to an EditorServiceExtension
     * to hook service events
     */
    export interface ServiceEventListener extends EditorServiceExtension { }

    interface EventDispatcher {
        /**
         * Send a custom event.  This can be used by services to publish custom events
         * @param  {string} eventType
         * @param  {any} data
         */
        sendEvent(eventType: string, data: any);
        sendEvent<T extends EngineCore.EventMetaData>(eventType:string, data?:T);
        sendEvent<T extends EngineCore.EventCallbackMetaData>(eventCallbackMetaData:T);

        /**
         * Subscribe to an event and provide a callback.  This can be used by services to subscribe to custom events
         * @param  {string} eventType
         * @param  {any} callback
         */
        subscribeToEvent?(eventType: string, callback: (...params) => any);

        /**
         * Subscribe to an event with a pre-wrapped event object.  This can be used by services to subscribe to custom events
         * @param  {Atomic.EventMetaData} wrappedEvent
         */
        subscribeToEvent?(wrappedEvent: EngineCore.EventMetaData);
    }

    /**
     * Generic service locator of editor services that may be injected by either a plugin
     * or by the editor itself.
     */
    export interface ServiceLoader extends EventDispatcher {
        /**
         * Loads a service into a service registry
         * @param  {EditorService} service
         */
        loadService(service: EditorServiceExtension): void;
    }

    /**
     * Service registry interface for registering services
     */
    export interface ServicesProvider<T extends ServiceEventListener> {
        registeredServices: T[];

        /**
         * Adds a service to the registered services list for this type of service
         * @param  {T}      service the service to register
         */
        register(service: T);
        /**
         * Removes a service from the registered services list for this type of service
         * @param  {T}      service the service to unregister
         */
        unregister(service: T);
    }

    /**
     * Interface that describes a Resource Editor Factory that will build out the editor for the relevant resource type
     */
    export interface ResourceEditorBuilder {
        /**
         * Returns true if this builder can generate an editor for this resource type
         */
        canHandleResource(resourcePath: string) : boolean;
        /**
         * Generates a resource editor for the provided resource type
         * @param  resourceFrame 
         * @param  resourcePath
         * @param  tabContainer
         * @param  lineNumber
         */
        getEditor(resourceFrame: EngineCore.UIWidget, resourcePath: string, tabContainer: EngineCore.UITabContainer, lineNumber: number) : EngineEditor.ResourceEditor;
    }
}

declare module EngineEditor.Modal {
    export interface ExtensionWindow extends EngineCore.UIWindow {
        hide();
    }
}

declare module EngineEditor.HostExtensions {

    /**
     * Generic service locator of editor services that may be injected by either a plugin
     * or by the editor itself.
     */
    export interface HostServiceLocator extends EngineEditor.Extensions.ServiceLoader {
        resourceServices: ResourceServicesProvider;
        projectServices: ProjectServicesProvider;
        sceneServices: SceneServicesProvider;
        uiServices: UIServicesProvider;
    }

    export interface HostEditorService extends EngineEditor.Extensions.EditorServiceExtension {
        /**
         * Called by the service locator at load time
         */
        initialize(serviceLocator: HostServiceLocator);
    }

    export interface ResourceServicesEventListener extends EngineEditor.Extensions.ServiceEventListener {
        /**
         * Called once a resource is saved
         */
        save?(ev: EngineEditor.EditorSaveResourceEvent);
        /**
         * Called when a resource is deleted
         */
        delete?(ev: EngineEditor.EditorDeleteResourceEvent);
        /**
         * Called when a resource is renamed
         */
        rename?(ev: EngineEditor.EditorRenameResourceNotificationEvent);
        /**
         * Called when a resource is about to be edited
         */
        edit?(ev: EngineEditor.EditorEditResourceEvent);
    }

    export interface ResourceServicesProvider extends EngineEditor.Extensions.ServicesProvider<ResourceServicesEventListener> {
        createMaterial(resourcePath: string, materialName: string, reportError: boolean): boolean;
    }

    export interface ProjectServicesEventListener extends EngineEditor.Extensions.ServiceEventListener {
        projectUnloaded?();
        projectLoaded?(ev: EngineEditor.EditorLoadProjectEvent);
        playerStarted?();
    }
    export interface ProjectServicesProvider extends EngineEditor.Extensions.ServicesProvider<ProjectServicesEventListener> {

        /**
         * Return a preference value or the provided default from the user settings file
         * @param  {string} extensionName name of the extension the preference lives under
         * @param  {string} preferenceName name of the preference to retrieve
         * @param  {number | boolean | string} defaultValue value to return if pref doesn't exist
         * @return {number|boolean|string}
         */
        getUserPreference(settingsGroup: string, preferenceName: string, defaultValue?: number): number;
        getUserPreference(settingsGroup: string, preferenceName: string, defaultValue?: string): string;
        getUserPreference(settingsGroup: string, preferenceName: string, defaultValue?: boolean): boolean;

        /**
         * Return a preference value or the provided default from the global user settings file
         * @param  {string} extensionName name of the section the preference lives under
         * @param  {string} preferenceName name of the preference to retrieve
         * @param  {number | boolean | string} defaultValue value to return if pref doesn't exist
         * @return {number|boolean|string}
         */
        getApplicationPreference(settingsGroup: string, preferenceName: string, defaultValue?: number): number;
        getApplicationPreference(settingsGroup: string, preferenceName: string, defaultValue?: string): string;
        getApplicationPreference(settingsGroup: string, preferenceName: string, defaultValue?: boolean): boolean;

        /**
         * Sets a user preference value in the project settings file
         * @param  {string} extensionName name of the extension the preference lives under
         * @param  {string} preferenceName name of the preference to set
         * @param  {number | boolean | string} value value to set
         */
        setUserPreference(extensionName: string, preferenceName: string, value: number | boolean | string);

        /**
         * Sets an editor preference value in the global editor settings file
         * @param  {string} groupName name of the section the preference lives under
         * @param  {string} preferenceName name of the preference to set
         * @param  {number | boolean | string} value value to set
         */
        setApplicationPreference(groupName: string, preferenceName: string, value: number | boolean | string);
    }

    export interface SceneServicesEventListener extends EngineEditor.Extensions.ServiceEventListener {
        activeSceneEditorChanged?(ev: EngineEditor.EditorActiveSceneEditorChangeEvent);
        editorSceneClosed?(ev: EngineEditor.EditorSceneClosedEvent);
    }
    export interface SceneServicesProvider extends EngineEditor.Extensions.ServicesProvider<SceneServicesEventListener> { }

    export interface UIServicesEventListener extends EngineEditor.Extensions.ServiceEventListener {
        menuItemClicked?(refid: string): boolean;
        projectContextItemClicked?(asset: ToolCore.Asset, refid: string): boolean;
        projectAssetClicked?(asset: ToolCore.Asset): boolean;
        hierarchyContextItemClicked?(node: EngineCore.Node, refid: string): boolean;

        /**
         * Handle messages that are submitted via Atomic.Query from within a web view editor.
         * @param message The message type that was submitted to be used to determine what the data contains if present
         * @param data any additional data that needs to be submitted with the message
         */
        handleWebMessage?(messageType: string, data?: any): void;
    }

    export interface UIServicesProvider extends EngineEditor.Extensions.ServicesProvider<UIServicesEventListener> {
        createPluginMenuItemSource(id: string, items: any): EngineCore.UIMenuItemSource;
        removePluginMenuItemSource(id: string);
        createHierarchyContextMenuItemSource(id: string, items: any): EngineCore.UIMenuItemSource;
        removeHierarchyContextMenuItemSource(id: string);
        createProjectContextMenuItemSource(id: string, items: any): EngineCore.UIMenuItemSource;
        removeProjectContextMenuItemSource(id: string);
        refreshHierarchyFrame();
        loadCustomInspector(customInspector: EngineCore.UIWidget);
        showModalWindow(windowText: string, uifilename: string, handleWidgetEventCB: (ev: EngineCore.UIWidgetEvent) => void): EngineEditor.Modal.ExtensionWindow;
        showNonModalWindow(windowText: string, uifilename: string, handleWidgetEventCB: (ev: EngineCore.UIWidgetEvent) => void): EngineEditor.Modal.ExtensionWindow;
        showModalError(windowText: string, message: string):EngineCore.UIMessageWindow;
        showResourceSelection(windowText: string, importerType: string, resourceType: string, callback: (retObject: any, args: any) => void, args?: any);

        /**
         * Returns the currently active resource editor or null
         * @return {EngineEditor.ResourceEditor}
         */
        getCurrentResourceEditor(): EngineEditor.ResourceEditor;

        
        /**
         * Will load a resource editor or navigate to an already loaded resource editor by path
         * @param path The path to the resource to load
         * @param lineNumber optional line number to navigate to
         * @return {EngineEditor.ResourceEditor}
         */
        loadResourceEditor(path: string, lineNumber?: number): EngineEditor.ResourceEditor;

        /**
         * Register a custom editor.  These editors will override editors in the standard editor list if
         * they both resolve the ```canHandleResource``` call.
         */
        registerCustomEditor(editorBuilder: EngineEditor.Extensions.ResourceEditorBuilder);

        /**
         * Will unregister a previously registered editor builder
         * @param  {EngineEditor.Extensions.ResourceEditorBuilder} editorBuilder
         */
        unregisterCustomEditor(editorBuilder: EngineEditor.Extensions.ResourceEditorBuilder);
    }
}

/**
 * Interfaces for client extensions
 */
declare module EngineEditor.ClientExtensions {

    export interface EditorFileEvent {
        filename: string;
        fileExt: string;
        editor: any;
    }

    export interface CodeLoadedEvent extends EditorFileEvent {
        code: string;
    }

    export interface CodeSavedEvent extends EditorFileEvent {
        code: string;
    }

    /**
     * Called once the resource has been deleted
     * @type {String}
     */
    export interface DeleteResourceEvent {

        // The full path to the resource to edit
        path: string;

    }

    /**
     * Called once the resource has been renamed
     * @type {String}
     */
    export interface RenameResourceEvent {

        /**
         * Original path of the resource
         * @type {string}
         */
        path: string;

        /**
         * New path of the resource
         * @type {string}
         */
        newPath: string;

        /**
         * New base name of the resource (no path or extension)
         * @type {string}
         */
        newName?: string;
    }

    /**
     * Generic service locator of editor services that may be injected by either a plugin
     * or by the editor itself.
     */
    export interface ClientServiceLocator extends EngineEditor.Extensions.ServiceLoader {
        /**
         * Exposed services
         * @type {WebViewServicesProvider}
         */
        clientServices: WebViewServicesProvider;
    }

    export interface ClientEditorService extends EngineEditor.Extensions.EditorServiceExtension {
        /**
         * Called by the service locator at load time
         */
        initialize(serviceLocator: ClientServiceLocator);
    }

    export interface PreferencesChangedEventData {
        applicationPreferences? : any;
        projectPreferences? : any;
    }

    export interface WebViewServiceEventListener extends EngineEditor.Extensions.EditorServiceExtension {
        configureEditor?(ev: EditorFileEvent);
        codeLoaded?(ev: CodeLoadedEvent);
        save?(ev: CodeSavedEvent);
        delete?(ev: DeleteResourceEvent);
        rename?(ev: RenameResourceEvent);
        projectUnloaded?();
        formatCode?();
        preferencesChanged?(preferences: PreferencesChangedEventData);
    }

    /**
     * Available methods exposed to client services
     */
    export interface WebViewServicesProvider extends EngineEditor.Extensions.ServicesProvider<WebViewServiceEventListener> {

        /**
         * Get a reference to the interop to talk to the host
         * @return {HostInterop}
         */
        getHostInterop(): HostInterop;

        /**
         * Return a preference value or the provided default from the user settings file
         * @param  {string} extensionName name of the extension the preference lives under
         * @param  {string} preferenceName name of the preference to retrieve
         * @param  {number | boolean | string} defaultValue value to return if pref doesn't exist
         * @return {number|boolean|string}
         */
        getUserPreference(settingsGroup: string, preferenceName: string, defaultValue?: number): number;
        getUserPreference(settingsGroup: string, preferenceName: string, defaultValue?: string): string;
        getUserPreference(settingsGroup: string, preferenceName: string, defaultValue?: boolean): boolean;

        /**
         * Return a preference value or the provided default from the application settings file
         * @param  {string} extensionName name of the extension the preference lives under
         * @param  {string} preferenceName name of the preference to retrieve
         * @param  {number | boolean | string} defaultValue value to return if pref doesn't exist
         * @return {number|boolean|string}
         */
        getApplicationPreference(settingsGroup: string, preferenceName: string, defaultValue?: number): number;
        getApplicationPreference(settingsGroup: string, preferenceName: string, defaultValue?: string): string;
        getApplicationPreference(settingsGroup: string, preferenceName: string, defaultValue?: boolean): boolean;
    }

    export interface AtomicErrorMessage {
        error_code: number;
        error_message: string;
    }

    /**
     * Interface for functions that are available from the client web view to call on the host
     */
    export interface HostInterop {

        /**
         * Called from the host to notify the client what file to load
         * @param  {string} codeUrl
         */
        loadCode(codeUrl: string);

        /**
         * Save the contents of the editor
         * @return {Promise}
         */
        saveCode(): PromiseLike<{}>;

        /**
         * Save the contents of a file as filename
         * @param  {string} filename
         * @param  {string} fileContents
         * @return {Promise}
         */
        saveFile(filename: string, fileContents: string): PromiseLike<{}>;

        /**
         * Queries the host for a particular resource and returns it in a promise
         * @param  {string} codeUrl
         * @return {Promise}
         */
        getResource(codeUrl: string): PromiseLike<{}>;

        /**
         * Returns a file resource from the resources directory
         * @param  {string} filename name and path of file under the project directory or a fully qualified file name
         * @return {Promise}
         */
        getFileResource(filename: string): PromiseLike<{}>;

        /**
         * Notify the host that the contents of the editor has changed
         */
        notifyEditorChange();

        /**
         * This adds a global routine to the window object so that it can be called from the host
         * @param  {string} routineName
         * @param  {(} callback
         */
        addCustomHostRoutine(routineName: string, callback: (...any) => void);
    }
}

declare module EngineEditor {
    /**
     * Valid editor shortcuts that can be called from menus
     */
    export type EditorShortcutType = "cut" | "copy" | "paste" | "undo" | "redo" | "close" | "frameselected" | "selectall";
}

declare const features : {
    dotnet : boolean;
    webview : boolean;
};