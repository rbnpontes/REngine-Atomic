//
// Copyright (c) 2014-2016 THUNDERBEAST GAMES LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

class Preferences {

    private static Ctor = (() => {
        new Preferences();
    })();

    private fileSystem: EngineCore.FileSystem;

    private static instance: Preferences;
    private _prefs: PreferencesFormat;

    private _cachedProjectPreferences: Object = null;

    constructor() {
        this.fileSystem = EngineCore.getFileSystem();
        Preferences.instance = this;
    }

    registerRecentProject(path: string): void {
        var index = this._prefs.recentProjects.indexOf(path);
        if (index >= 0) {
            this._prefs.recentProjects.splice(index, 1);
        }
        this._prefs.recentProjects.unshift(path);
        this.updateRecentProjects(true);
    }

    updateRecentProjects(write: boolean = false): void {

        for (var i = 0; i < this._prefs.recentProjects.length; i++) {
            var path = this._prefs.recentProjects[i];
            if (!this.fileSystem.exists(path)) {
                this._prefs.recentProjects.splice(i, 1);
                write = true;
            }
        }
        if (write)
            this.write();
    }

    deleteRecentProjects(): void {
        this._prefs.recentProjects.length = 0;
        this.write();
    }

    addColorHistory(path: string): void {
        var index = this._prefs.colorHistory.indexOf(path);   // search array for entry
        if (index >= 0) {                                     // if its in there,
            this._prefs.colorHistory.splice(index, 1);        // REMOVE it.
        }
        this._prefs.colorHistory.unshift(path);               // now add it to beginning of array
        this.updateColorHistory(true);                        // update and write out
    }

    updateColorHistory(write: boolean = false): void {

        var len = this._prefs.colorHistory.length;                 // we only need indexes 0-7 now
        var over = 8;
        if ( len >= over )                                         // have MOaR than we need
            this._prefs.colorHistory.splice( over, len - over );   // remove the excess

        if (write)
            this.write();
    }

    getPreferencesFullPath(): string {
        var filePath = this.fileSystem.getAppPreferencesDir("AtomicEditor", "Preferences");
        filePath += "prefs.json";
        return filePath;
    }

    read(): void {
        var filePath = this.getPreferencesFullPath();
        var jsonFile;

        //check if file doesn't exist, create default json
        if (!this.fileSystem.fileExists(filePath)) {
            this.useDefaultConfig();
            this.write();
            return;
        }

        //Read file
        jsonFile = new EngineCore.File(filePath, EngineCore.FileMode.FILE_READ);

        var prefs = null;

        try {

            if (jsonFile.isOpen())
                prefs = <PreferencesFormat>JSON.parse(jsonFile.readText());

        } catch (e) {

            prefs = null;
        }

        if (prefs) {
            const defaultPrefs = new PreferencesFormat();
            const shouldWrite = defaultPrefs.applyMissingDefaults(prefs);
            this._prefs = prefs;
            if (shouldWrite) {
                this.write();
            }
        } else {
            console.log("Editor preference file missing or invalid, regenerating default configuration");
            this.useDefaultConfig();
            this.write();
        }

    }

    write(): boolean {
        var filePath = this.getPreferencesFullPath();
        var jsonFile = new EngineCore.File(filePath, EngineCore.FileMode.FILE_WRITE);
        if (!jsonFile.isOpen()) return false;
        jsonFile.writeString(JSON.stringify(this._prefs, null, 2));
    }

    saveEditorWindowData(windowData: WindowData) {
        this._prefs.editorWindow = windowData;
        this.write();
    }

    savePlayerWindowData(windowData: WindowData) {
        this._prefs.playerWindow = windowData;
        this.write();
    }

    saveEditorUiData(uiData: UserInterfaceData) {
        this._prefs.uiData = uiData;
        this.write();
    }

    toggleTheme() : void { // swap the themes
        var uiData = this.uiData;
        if ( this.uiData.defaultSkinPath == "resources/default_skin/" ) {
            this.uiData.defaultSkinPath = "resources/default_skin_light/";
            this.uiData.skinPath = "editor/skin_light/";
        }
        else {
            this.uiData.defaultSkinPath = "resources/default_skin/";
            this.uiData.skinPath = "editor/skin/";
        }
        var ui = EngineCore.ui; // install the new skins, live action
        ui.loadSkin(this.uiData.skinPath + "/skin.tb.txt", this.uiData.defaultSkinPath + "/skin.tb.txt");
        this.saveEditorUiData(this.uiData); // save preferences
    }

    useDefaultConfig(): void {
        this._prefs = new PreferencesFormat();
    }

    get cachedProjectPreferences(): any {
        return this._cachedProjectPreferences;
    }

    get cachedApplicationPreferences(): PreferencesFormat {
        return this._prefs;
    }

    get editorWindow(): WindowData {
        return this._prefs.editorWindow;
    }

    get playerWindow(): WindowData {
        return this._prefs.playerWindow;
    }

    get recentProjects(): string[] {
        return this._prefs.recentProjects;
    }

    get colorHistory(): string[] {
        return this._prefs.colorHistory;
    }

    get uiData(): UserInterfaceData {
        return this._prefs.uiData;
    }

    get editorBuildData(): EditorBuildData {
        return this._prefs.editorBuildData;
    }

    get editorFeatures(): EditorFeatures {
        return this._prefs.editorFeatures;
    }

    static getInstance(): Preferences {
        return Preferences.instance;
    }

    /**
     * Load up the user preferences for the project
     */
    loadUserPrefs() {
        const prefsFileLoc = ToolCore.toolSystem.project.userPrefsFullPath;
        if (EngineCore.fileSystem.fileExists(prefsFileLoc)) {
            let prefsFile = new EngineCore.File(prefsFileLoc, EngineCore.FileMode.FILE_READ);
            try {
                let prefs = JSON.parse(prefsFile.readText());
                this._cachedProjectPreferences = prefs;
            } finally {
                prefsFile.close();
            }
        }
    }

    /**
     * Return a preference value or the provided default from the user settings file located in the project
     * @param  {string} settingsGroup name of the group these settings should fall under
     * @param  {string} preferenceName name of the preference to retrieve
     * @param  {number | boolean | string} defaultValue value to return if pref doesn't exist
     * @return {number|boolean|string}
     */
    getUserPreference(settingsGroup: string, preferenceName: string, defaultValue?: number): number;
    getUserPreference(settingsGroup: string, preferenceName: string, defaultValue?: string): string;
    getUserPreference(settingsGroup: string, preferenceName: string, defaultValue?: boolean): boolean;
    getUserPreference(settingsGroup: string, preferenceName: string, defaultValue?: any): any {

        // Cache the settings so we don't keep going out to the file
        if (this._cachedProjectPreferences == null) {
            this.loadUserPrefs();
        }

        if (this._cachedProjectPreferences && this._cachedProjectPreferences[settingsGroup]) {
            return this._cachedProjectPreferences[settingsGroup][preferenceName] || defaultValue;
        }

        // if all else fails
        return defaultValue;
    }

    /**
     * Sets a preference value in a preferences file that is provided
     * @param  {string} preferencesFilePath path to the prefs file to update
     * @param  {string} settingsGroup name of the group the preference lives under
     * @param  {string} preferenceName name of the preference to set
     * @param  {number | boolean | string} value value to set
     */
    setGenericPreference(preferencesFilePath: string, settingsGroup: string, preferenceName: string, value: number | boolean | string): Object {
        let prefs = {};

        if (EngineCore.fileSystem.fileExists(preferencesFilePath)) {
            let prefsFile = new EngineCore.File(preferencesFilePath, EngineCore.FileMode.FILE_READ);
            try {
                prefs = JSON.parse(prefsFile.readText());
            } finally {
                prefsFile.close();
            }
        }

        prefs[settingsGroup] = prefs[settingsGroup] || {};
        prefs[settingsGroup][preferenceName] = value;

        let saveFile = new EngineCore.File(preferencesFilePath, EngineCore.FileMode.FILE_WRITE);
        try {
            saveFile.writeString(JSON.stringify(prefs, null, "  "));
        } finally {
            saveFile.flush();
            saveFile.close();
        }

        // Cache the update
        return prefs;
    }

    /**
     * Sets a user preference value in the user settings file located in the project
     * @param  {string} settingsGroup name of the group the preference lives under
     * @param  {string} preferenceName name of the preference to set
     * @param  {number | boolean | string} value value to set
     */
    setUserPreference(settingsGroup: string, preferenceName: string, value: number | boolean | string) {

        const prefsFileLoc = ToolCore.toolSystem.project.userPrefsFullPath;
        const prefs = this.setGenericPreference(prefsFileLoc, settingsGroup, preferenceName, value);

        // Cache the update
        this._cachedProjectPreferences = prefs;
    }

    /**
     * Sets an editor preference value in the global user settings file
     * @param  {string} settingsGroup name of the group the preference lives under
     * @param  {string} preferenceName name of the preference to set
     * @param  {number | boolean | string} value value to set
     */
    setApplicationPreference(settingsGroup: string, preferenceName: string, value: number | boolean | string) {

        const prefsFileLoc = this.getPreferencesFullPath();
        const prefs = this.setGenericPreference(prefsFileLoc, settingsGroup, preferenceName, value);

        // Cache the update
        this._prefs = prefs as PreferencesFormat;
    }

    /**
     * Return a preference value or the provided default from the global user settings file located in the project
     * @param  {string} settingsGroup name of the group these settings should fall under
     * @param  {string} preferenceName name of the preference to retrieve
     * @param  {number | boolean | string} defaultValue value to return if pref doesn't exist
     * @return {number|boolean|string}
     */
    getApplicationPreference(settingsGroup: string, preferenceName: string, defaultValue?: number): number;
    getApplicationPreference(settingsGroup: string, preferenceName: string, defaultValue?: string): string;
    getApplicationPreference(settingsGroup: string, preferenceName: string, defaultValue?: boolean): boolean;
    getApplicationPreference(settingsGroup: string, preferenceName: string, defaultValue?: any): any {

        // Cache the settings so we don't keep going out to the file
        if (this._prefs == null) {
            this.read();
        }

        if (this._prefs && this._prefs[settingsGroup]) {
            return this._prefs[settingsGroup][preferenceName] || defaultValue;
        }

        // if all else fails
        return defaultValue;
    }

    /**
     * Sets a group of user preference values in the user settings file located in the project.  Elements in the
     * group will merge in with existing group preferences.  Use this method if setting a bunch of settings
     * at once.
     * @param  {string} settingsGroup name of the group the preference lives under
     * @param  {string} groupPreferenceValues an object literal containing all of the preferences for the group.
     */
    setUserPreferenceGroup(settingsGroup: string, groupPreferenceValues: Object) {

        const prefsFileLoc = ToolCore.toolSystem.project.userPrefsFullPath;
        let prefs = {};

        if (EngineCore.fileSystem.fileExists(prefsFileLoc)) {
            let prefsFile = new EngineCore.File(prefsFileLoc, EngineCore.FileMode.FILE_READ);
            try {
                prefs = JSON.parse(prefsFile.readText());
            } finally {
                prefsFile.close();
            }
        }

        prefs[settingsGroup] = prefs[settingsGroup] || {};
        for (let preferenceName in groupPreferenceValues) {
            prefs[settingsGroup][preferenceName] = groupPreferenceValues[preferenceName];
        }

        let saveFile = new EngineCore.File(prefsFileLoc, EngineCore.FileMode.FILE_WRITE);
        try {
            saveFile.writeString(JSON.stringify(prefs, null, "  "));
        } finally {
            saveFile.flush();
            saveFile.close();
        }

        // Cache the update
        this._cachedProjectPreferences = prefs;
    }
}

interface WindowData {
    x: number;
    y: number;
    width: number;
    height: number;
    monitor: number;
    maximized: boolean;
}

interface MonacoEditorSettings {
    theme: string;
    fontSize: number;
    fontFamily: string;
    showInvisibles: boolean;
    useSoftTabs: boolean;
    tabSize: number;
}

interface UserInterfaceData {
    skinPath: string;
    defaultSkinPath: string;
    fontFile: string;
    fontName: string;
    fontSize: number;
}

interface EditorBuildData {
    lastEditorBuildSHA: string;
}

interface EditorFeatures {
    closePlayerLog: boolean;
    defaultPath: string;
    defaultLanguage: string;
    screenshotPath: string;
    screenshotFormat: string;
}

interface DevelopmentUI {
    projectFrameWidthScalar: number;
}

class PreferencesFormat {

    constructor() {

        this.setDefault();
    }

    setDefault() {

        this.recentProjects = [];

        this.colorHistory = [ "#000000", "#ffffff", "#00ff00", "#0000ff", "#ff0000", "#ff00ff", "#ffff00", "#668866" ];

        this.editorWindow = {
            x: 0,
            y: 0,
            width: 0,
            height: 0,
            monitor: 0,
            maximized: true
        };

        this.playerWindow = {
            x: 0,
            y: 0,
            width: 0,
            height: 0,
            monitor: 0,
            maximized: false
        };

        this.codeEditor = {
            theme: "vs-dark",
            fontSize: 12,
            fontFamily: "",
            showInvisibles: false,
            useSoftTabs: true,
            tabSize: 4
        };

        this.uiData = {
            skinPath: "editor/skin/",
            defaultSkinPath: "resources/default_skin/",
            fontFile: "resources/vera.ttf",
            fontName: "Vera",
            fontSize: 12
        };

        this.editorBuildData = {
            lastEditorBuildSHA: "Unversioned Build"
        };

        var fileSystem = EngineCore.getFileSystem();
        var userDocuments = fileSystem.userDocumentsDir;
        if (EngineCore.platform == "MacOSX") userDocuments += "Documents/";
        userDocuments += "AtomicProjects";

        this.editorFeatures = {
            closePlayerLog: true,
            defaultPath: userDocuments,
            defaultLanguage: "JavaScript",
            screenshotPath: userDocuments,
            screenshotFormat: "png"
        };

        this.developmentUI = {
            projectFrameWidthScalar: 1
        };

    }

    /**
     * Run through a provided prefs block and verify that all the sections are present.  If any
     * are missing, add the defaults in
     * @param  {PreferencesFormat} prefs
     * @return boolean returns true if any missing defaults were updated
     */
    applyMissingDefaults(prefs: PreferencesFormat) {
        let updatedMissingDefaults = false;
        if (!prefs.recentProjects) {
            prefs.recentProjects = this.recentProjects;
            updatedMissingDefaults = true;
        }

        if (!prefs.colorHistory) {
            prefs.colorHistory = this.colorHistory;
            updatedMissingDefaults = true;
        }

        if (!prefs.editorWindow) {
            prefs.editorWindow = this.editorWindow;
            updatedMissingDefaults = true;
        }

        if (!prefs.playerWindow) {
            prefs.playerWindow = this.playerWindow;
            updatedMissingDefaults = true;
        }

        if (!prefs.codeEditor) {
            prefs.codeEditor = this.codeEditor;
            updatedMissingDefaults = true;
        }

        if (!prefs.uiData) {
            prefs.uiData = this.uiData;
            updatedMissingDefaults = true;
        }

        if (!prefs.editorBuildData) {
            prefs.editorBuildData = this.editorBuildData;
            updatedMissingDefaults = true;
        }

        if (!prefs.editorFeatures) {
            prefs.editorFeatures = this.editorFeatures;
            updatedMissingDefaults = true;
        }

        if (!prefs.editorFeatures.defaultPath) {
            prefs.editorFeatures.defaultPath = this.editorFeatures.defaultPath;
            updatedMissingDefaults = true;
        }

        if (!prefs.editorFeatures.screenshotPath) {
            prefs.editorFeatures.screenshotPath = this.editorFeatures.screenshotPath;
            updatedMissingDefaults = true;
        }

        if (!prefs.editorFeatures.screenshotFormat) {
            prefs.editorFeatures.screenshotFormat = this.editorFeatures.screenshotFormat;
            updatedMissingDefaults = true;
        }

        if (!prefs.developmentUI) {
            prefs.developmentUI = this.developmentUI;
            updatedMissingDefaults = true;
        }

        return updatedMissingDefaults;
    }

    recentProjects: string[];
    editorWindow: WindowData;
    playerWindow: WindowData;
    codeEditor: MonacoEditorSettings;
    uiData: UserInterfaceData;
    editorBuildData: EditorBuildData;
    colorHistory: string[];
    editorFeatures: EditorFeatures;
    developmentUI: DevelopmentUI;
}

export = Preferences;
