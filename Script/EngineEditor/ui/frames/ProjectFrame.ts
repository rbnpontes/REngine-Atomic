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

import ScriptWidget = require("ui/ScriptWidget");
import {default as AtomicEditor} from "editor/Editor";
import ProjectFrameMenu = require("./menus/ProjectFrameMenu");
import MenuItemSources = require("./menus/MenuItemSources");
import SearchBarFiltering = require("resources/SearchBarFiltering");
import EditorUI = require("ui/EditorUI");

class ProjectFrame extends ScriptWidget {

    folderList: EngineCore.UIListView;
    menu: ProjectFrameMenu;
    currentFolder: ToolCore.Asset;
    resourceFolder: ToolCore.Asset;
    assetGUIDToItemID = {};
    resourcesID: number = -1;
    assetReferencePath: string = null;
    currentReferencedButton: EngineCore.UIButton = null;
    containerScrollToHeight: number;
    containerScrollToHeightCounter: number;
    uiSearchBar: SearchBarFiltering.UISearchBar = new SearchBarFiltering.UISearchBar();
    search: boolean = false;
    searchEdit: EngineCore.UIEditField;

    constructor(parent: EngineCore.UIWidget) {

        super();

        this.menu = new ProjectFrameMenu();

        this.load("editor/ui/projectframe.tb.txt");

        this.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_TOP_BOTTOM;

        this.searchEdit = <EngineCore.UIEditField>this.getWidget("filter");

        var projectviewcontainer = parent.getWidget("projectviewcontainer");

        projectviewcontainer.addChild(this);

        var foldercontainer = this.getWidget("foldercontainer");

        var folderList = this.folderList = new EngineCore.UIListView();

        folderList.rootList.id = "folderList_";

        foldercontainer.addChild(folderList);

        // events
        this.subscribeToEvent(ToolCore.ProjectLoadedEvent((data) => this.handleProjectLoaded(data)));
        this.subscribeToEvent(EngineEditor.ProjectUnloadedNotificationEvent((data) => this.handleProjectUnloaded(data)));
        this.subscribeToEvent(EngineCore.DragEndedEvent((data: EngineCore.DragEndedEvent) => this.handleDragEnded(data)));

        this.subscribeToEvent(ToolCore.ResourceAddedEvent((ev: ToolCore.ResourceAddedEvent) => this.handleResourceAdded(ev)));
        this.subscribeToEvent(ToolCore.ResourceRemovedEvent((ev: ToolCore.ResourceRemovedEvent) => this.handleResourceRemoved(ev)));
        this.subscribeToEvent(ToolCore.AssetRenamedEvent((ev: ToolCore.AssetRenamedEvent) => this.handleAssetRenamed(ev)));
        this.subscribeToEvent(EngineEditor.InspectorProjectReferenceEvent((ev: EngineEditor.InspectorProjectReferenceEvent) => { this.handleInspectorProjectReferenceHighlight(ev.path); }));

        this.searchEdit.subscribeToEvent(this.searchEdit, EngineCore.UIWidgetEvent((data) => this.handleWidgetEvent(data)));

        folderList.subscribeToEvent(EngineCore.UIListViewSelectionChangedEvent((event: EngineCore.UIListViewSelectionChangedEvent) => this.handleFolderListSelectionChangedEvent(event)));

        // this.subscribeToEvent(EditorEvents.ResourceFolderCreated, (ev: EditorEvents.ResourceFolderCreatedEvent) => this.handleResourceFolderCreated(ev));

        // this uses FileWatcher which doesn't catch subfolder creation
        this.subscribeToEvent(EngineCore.FileChangedEvent((data) => {

            // console.log("File CHANGED! ", data.fileName);

        }));

        // Development support for project frame resizing, including hierarchy frame
        this.subscribeToEvent("DevelopmentUIEvent", (data) => {
            if (data.subEvent == "ScaleFrameWidth" && data.arg0 == "projectframe") {
                this.handleScaleWidth(data.arg1);
            }
        });

        this.subscribeToEvent("ImportAssetEvent", (data) => {
            if (data.file.length > 0) {  // imported an asset file 
                var fileSystem = EngineCore.getFileSystem();
                var srcFilename = data.file;
                var pathInfo = EngineCore.splitPath(srcFilename);
                var destFilename = EngineCore.addTrailingSlash(data.destination);
                destFilename += pathInfo.fileName + pathInfo.ext;
                if ( fileSystem.copy(srcFilename, destFilename) ) {
                    EditorUI.showEditorStatus ( "Copied Asset " + pathInfo.fileName + " into project as " + destFilename);
                    var db = ToolCore.getAssetDatabase();
                    db.scan(); // fix up the asset database on success
                }
                else {
                    EditorUI.showEditorStatus ( "Warning, could not copy Asset " + pathInfo.fileName + " from " + srcFilename);
                }
            }
        });

    }

    handleAssetRenamed(ev: ToolCore.AssetRenamedEvent) {

        var container: EngineCore.UILayout = <EngineCore.UILayout>this.getWidget("contentcontainer");

        for (var widget = container.firstChild; widget; widget = widget.next) {

            if (widget.id == ev.asset.guid) {

                if (widget["assetButton"]) {
                    widget["assetButton"].text = ev.asset.name + ev.asset.extension;
                    widget["assetButton"].dragObject = new EngineCore.UIDragObject(ev.asset, ev.asset.name);
                }

                break;
            }
        }

    }

    handleResourceRemoved(ev: ToolCore.ResourceRemovedEvent) {

        var folderList = this.folderList;
        folderList.deleteItemByID(ev.guid);

        var container: EngineCore.UILayout = <EngineCore.UILayout>this.getWidget("contentcontainer");

        for (var widget = container.firstChild; widget; widget = widget.next) {

            if (widget.id == ev.guid) {

                container.removeChild(widget);
                break;
            }

        }

    }

    handleResourceAdded(ev: ToolCore.ResourceAddedEvent) {

        var db = ToolCore.getAssetDatabase();
        var asset = db.getAssetByGUID(ev.guid);

        var parent = asset.parent;
        var folderList = this.folderList;

        // these can be out of order
        if (asset.isFolder()) {

            if (!parent) {

                var id = folderList.addRootItem(asset.name, "Folder.icon", asset.guid);
                this.resourcesID = id;
                this.assetGUIDToItemID[asset.guid] = id;
                this.resourceFolder = asset;

            } else {
                var parentItemID = this.assetGUIDToItemID[parent.guid];
                var id = folderList.addChildItem(parentItemID, asset.name, "Folder.icon", asset.guid);
                this.assetGUIDToItemID[asset.guid] = id;
            }

        } else if (parent == this.currentFolder) {

            var container: EngineCore.UILayout = <EngineCore.UILayout>this.getWidget("contentcontainer");
            container.addChild(this.createButtonLayout(asset));

        }

    }

    handleWidgetEvent(data: EngineCore.UIWidgetEvent): boolean {

        if (!ToolCore.toolSystem.project) return;

        if (data.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_KEY_UP) {

            // Activates search while user is typing in search widget
            if (data.target == this.searchEdit) {

                if (data.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_KEY_UP) {
                    this.search = true;
                    this.refreshContent(this.currentFolder);
                }
            }
        }

        if (data.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_RIGHT_POINTER_UP) {

            var id = data.target.id;
            var db = ToolCore.getAssetDatabase();
            var asset: ToolCore.Asset;

            if (id == "folderList_")
                asset = db.getAssetByGUID(this.folderList.hoverItemID);
            else
                asset = db.getAssetByGUID(id);

            if (asset) {

                this.menu.createAssetContextMenu(this, asset, data.x, data.y);

                return true;

            }

        }

        if (data.type == EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK) {

            var id = data.target.id;

            // cancel search - goes back to the last selected folder
            if (id == "cancel search") {
                if (!ToolCore.toolSystem.project) return;
                this.searchEdit.text = "";
                this.refreshContent(this.currentFolder);
            }

            if (this.menu.handlePopupMenu(data.target, data.refid))
                return true;

            // create
            if (id == "menu create") {
                if (!ToolCore.toolSystem.project) return;
                var src = MenuItemSources.getMenuItemSource("project create items");
                var menu = new EngineCore.UIMenuWindow(data.target, "create popup");
                menu.show(src);
                return true;

            }

            var db = ToolCore.getAssetDatabase();
            var fs = EngineCore.getFileSystem();

            if (data.target && data.target.id.length) {

                if (id == "folderList_") {

                    var list = <EngineCore.UISelectList>data.target;

                    var selectedId = list.selectedItemID;

                    // selectedId == 0 = root "Resources"

                    if (selectedId != "0") {

                        var asset = db.getAssetByGUID(selectedId);

                        if (asset.isFolder)
                            this.refreshContent(asset);
                    }

                    return true;

                }

                var asset = db.getAssetByGUID(id);

                if (asset) {

                    if (asset.isFolder()) {

                        this.folderList.selectItemByID(id);
                        this.refreshContent(asset);

                    } else {

                        this.sendEvent(EngineEditor.EditorEditResourceEventData({ "path": asset.path, lineNumber: 0 }));
                    }

                }

            }

            if (this.currentReferencedButton) {
                this.currentReferencedButton.setState(4, false);
                this.currentReferencedButton = null;
            }

        }

        return false;

    }

    rescan(asset: ToolCore.Asset) {

        var db = ToolCore.getAssetDatabase();
        db.scan();

    }

    selectPath(path: string) {

        var db = ToolCore.getAssetDatabase();

        var asset = db.getAssetByPath(path);

        if (!asset)
            return;

        this.folderList.selectItemByID(asset.guid);

    }

    handleFolderListSelectionChangedEvent(event: EngineCore.UIListViewSelectionChangedEvent) {

        var selectedId = this.folderList.selectedItemID;

        if (selectedId != "0") {
            var db = ToolCore.getAssetDatabase();

            var asset = db.getAssetByGUID(selectedId);
            if (!asset)
                return;

            if (asset.isFolder)
                this.refreshContent(asset);
        }
    }

    handleDragEnded(data: EngineCore.DragEndedEvent) {

        var asset: ToolCore.Asset;

        if (data.target) {

            var container: EngineCore.UILayout = <EngineCore.UILayout>this.getWidget("contentcontainer");

            if (data.target.id == "contentcontainerscroll" || container.isAncestorOf(data.target)) {

                if (data.target["asset"])
                    asset = <ToolCore.Asset>data.target["asset"];

                if (!asset || !asset.isFolder)
                    asset = this.currentFolder;
            }

        }

        if (!asset) {

            // if the drop target is the folderList's root select widget
            var rootList = this.folderList.rootList;
            var hoverID = rootList.hoverItemID;

            if (hoverID == "")
                return;

            var db = ToolCore.getAssetDatabase();
            asset = db.getAssetByGUID(hoverID);

        }

        if (!asset || !asset.isFolder)
            return;

        var dragObject = data.dragObject;

        if (dragObject.object && dragObject.object.typeName == "Node") {

            var node = <EngineCore.Node>dragObject.object;

            var prefabComponent = <EngineCore.PrefabComponent>node.getComponent("PrefabComponent");

            if (prefabComponent) {

                prefabComponent.savePrefab();

            }
            else {
                var destFilename = EngineCore.addTrailingSlash(asset.path);
                destFilename += node.name + ".prefab";
                var file = new EngineCore.File(destFilename, EngineCore.FileMode.FILE_WRITE);
                node.saveXML(file);
                file.close();
            }

            this.rescan(asset);

            return;

        } else if (dragObject.object && dragObject.object.typeName == "Asset") {

            var dragAsset = <ToolCore.Asset>dragObject.object;

            // get the folder we dragged on
            var destPath = EngineCore.addTrailingSlash(asset.path);

            dragAsset.move(destPath + dragAsset.name + dragAsset.extension);

            this.refreshContent(this.currentFolder);

            return true;
        }

        // dropped some files?
        var filenames = dragObject.filenames;

        if (!filenames.length)
            return;

        var fileSystem = EngineCore.getFileSystem();


        for (var i in filenames) {

            var srcFilename = filenames[i];

            var pathInfo = EngineCore.splitPath(srcFilename);

            var destFilename = EngineCore.addTrailingSlash(asset.path);

            destFilename += pathInfo.fileName + pathInfo.ext;

            fileSystem.copy(srcFilename, destFilename);

        }

        this.rescan(asset);

    }

    handleScaleWidth(scale:number) {
        this.getWidget("projectframe").layoutMinWidth = 220 * scale;
    }

    handleProjectLoaded(data: ToolCore.ProjectLoadedEvent) {

        this.handleScaleWidth(parseFloat(AtomicEditor.instance.getApplicationPreference( "developmentUI", "projectFrameWidthScalar", "1")));

        this.folderList.rootList.value = 0;
        this.folderList.setExpanded(this.resourcesID, true);
        this.refreshContent(this.resourceFolder);

    }

    handleProjectUnloaded(data) {

        this.handleScaleWidth(1);

        this.folderList.deleteAllItems();
        this.resourceFolder = null;

        var container: EngineCore.UILayout = <EngineCore.UILayout>this.getWidget("contentcontainer");
        container.deleteAllChildren();

    }

    // Shows referenced file in projectframe
    handleInspectorProjectReferenceHighlight(path: string): void {
        this.assetReferencePath = path;
        var db = ToolCore.getAssetDatabase();
        var asset = db.getAssetByPath(this.resourceFolder.getPath() + "/" + path);

        this.folderList.selectAllItems(false);
        this.folderList.selectItemByID(asset.parent.guid, true);
        this.refreshContent(asset.parent);
        this.folderList.scrollToSelectedItem();
    }

    // Searches folders within folders recursively
    searchProjectFolder(folderPath: string, container: EngineCore.UILayout, searchText: string, db: ToolCore.AssetDatabase) {

        if (folderPath == "")
            return;

        var assets = db.getFolderAssets(folderPath);

        for (var i in assets) {

            var childAsset = assets[i];

            if (childAsset.isFolder()) {
                this.searchProjectFolder(childAsset.path, container, searchText, db);
            } else if (this.uiSearchBar.searchPopulate(searchText, childAsset.name + childAsset.extension)) {
                container.addChild(this.createButtonLayout(childAsset));
            }
        }
    }

    private refreshContent(folder: ToolCore.Asset) {

        if (this.currentFolder != folder) {

            this.sendEvent(EngineEditor.ContentFolderChangedEventData({ path: folder.path }));

        }

        this.currentFolder = folder;

        var db = ToolCore.getAssetDatabase();

        var container: EngineCore.UILayout = <EngineCore.UILayout>this.getWidget("contentcontainer");
        container.deleteAllChildren();

        if (this.currentFolder != null) {
            var assets = db.getFolderAssets(folder.path);

            this.containerScrollToHeightCounter = 0;

            if (this.searchEdit.text == "" || !this.search) {

                for (var i in assets) {
                    var asset = assets[i];
                    container.addChild(this.createButtonLayout(asset));
                    this.containerScrollToHeightCounter++;
                }
            } else if (this.search) {
                this.searchProjectFolder(this.resourceFolder.path, container, this.searchEdit.text, db);
            }
        }

        var containerScroll: EngineCore.UIScrollContainer = <EngineCore.UIScrollContainer>this.getWidget("contentcontainerscroll");
        containerScroll.scrollTo(0, this.containerScrollToHeight);
        this.search = false;
    }

    private createButtonLayout(asset: ToolCore.Asset): EngineCore.UILayout {

        var system = ToolCore.getToolSystem();
        var project = system.project;
        var fs = EngineCore.getFileSystem();

        var pathinfo = EngineCore.splitPath(asset.path);

        var bitmapID = "Folder.icon";

        if (fs.fileExists(asset.path)) {
            bitmapID = "FileBitmap";
        }

        if (pathinfo.ext == ".js") {
            if (project.isComponentsDirOrFile(asset.path)) {
                bitmapID = "ComponentBitmap";
            }
            else {
                bitmapID = "JavascriptBitmap";
            }
        }

        var blayout = new EngineCore.UILayout();
        blayout.id = asset.guid;

        blayout.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_LEFT;

        var spacer = new EngineCore.UIWidget();
        spacer.rect = [0, 0, 8, 8];
        blayout.addChild(spacer);

        var button = new EngineCore.UIButton();



        // setup the drag object
        button.dragObject = new EngineCore.UIDragObject(asset, asset.name);

        var lp = new EngineCore.UILayoutParams;
        var buttonHeight = lp.height = 20;

        //Get the path of the button and compare it to the asset's path to highlight
        var resourcePath = this.resourceFolder.getPath() + "/" + this.assetReferencePath;

        //Highlight Button UI
        if (resourcePath == asset.path) {

            button.setState(4, true);
            this.currentReferencedButton = button;
            this.containerScrollToHeight = this.containerScrollToHeightCounter * buttonHeight;

        }

        var fd = new EngineCore.UIFontDescription();
        fd.id = "Vera";
        fd.size = 11;

        button.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_LEFT;

        var image = new EngineCore.UISkinImage(bitmapID);
        image.rect = [0, 0, 12, 12];
        image.gravity = EngineCore.UI_GRAVITY.UI_GRAVITY_RIGHT;
        blayout.addChild(image);
        image["asset"] = asset;

        button.id = asset.guid;
        button.layoutParams = lp;
        button.fontDescription = fd;
        button.text = asset.name + asset.extension;
        button.skinBg = "TBButton.flat";
        button["asset"] = asset;
        blayout["assetButton"] = button;
        blayout.addChild(button);

        return blayout;
    }

}

export = ProjectFrame;
