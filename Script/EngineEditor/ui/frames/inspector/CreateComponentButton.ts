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

var audioCreateSource = new EngineCore.UIMenuItemSource();

audioCreateSource.addItem(new EngineCore.UIMenuItem("SoundListener", "SoundListener"));
audioCreateSource.addItem(new EngineCore.UIMenuItem("SoundSource", "SoundSource"));
audioCreateSource.addItem(new EngineCore.UIMenuItem("SoundSource3D", "SoundSource3D"));

var _2DCreateSource = new EngineCore.UIMenuItemSource();
_2DCreateSource.addItem(new EngineCore.UIMenuItem("PhysicsWorld2D", "PhysicsWorld2D"));
_2DCreateSource.addItem(new EngineCore.UIMenuItem("StaticSprite2D", "StaticSprite2D"));
_2DCreateSource.addItem(new EngineCore.UIMenuItem("AnimatedSprite2D", "AnimatedSprite2D"));
_2DCreateSource.addItem(new EngineCore.UIMenuItem("ParticleEmitter2D", "ParticleEmitter2D"));
_2DCreateSource.addItem(new EngineCore.UIMenuItem("PointLight2D", "PointLight2D"));
_2DCreateSource.addItem(new EngineCore.UIMenuItem("DirectionalLight2D", "DirectionalLight2D"));
_2DCreateSource.addItem(new EngineCore.UIMenuItem("RigidBody2D", "RigidBody2D"));
_2DCreateSource.addItem(new EngineCore.UIMenuItem("CollisionBox2D", "CollisionBox2D"));
_2DCreateSource.addItem(new EngineCore.UIMenuItem("CollisionCircle2D", "CollisionCircle2D"));
_2DCreateSource.addItem(new EngineCore.UIMenuItem("TileMap2D", "TileMap2D"));

var geometryCreateSource = new EngineCore.UIMenuItemSource();

geometryCreateSource.addItem(new EngineCore.UIMenuItem("StaticModel", "StaticModel"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("AnimatedModel", "AnimatedModel"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("AnimationController", "AnimationController"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("BillboardSet", "BillboardSet"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("CustomGeometry", "CustomGeometry"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("ParticleEmitter", "ParticleEmitter"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("RibbonTrail", "RibbonTrail"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("Skybox", "SkyBox"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("StaticModelGroup", "StaticModelGroup"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("Terrain", "Terrain"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("Text3D", "create component"));
geometryCreateSource.addItem(new EngineCore.UIMenuItem("Water", "Water"));

var logicCreateSource = new EngineCore.UIMenuItemSource();

logicCreateSource.addItem(new EngineCore.UIMenuItem("JSComponent", "JSComponent"));
logicCreateSource.addItem(new EngineCore.UIMenuItem("CSComponent", "CSComponent"));
logicCreateSource.addItem(new EngineCore.UIMenuItem("AnimationController", "AnimationController"));
logicCreateSource.addItem(new EngineCore.UIMenuItem("SplinePath", "SplinePath"));

var navigationCreateSource = new EngineCore.UIMenuItemSource();

navigationCreateSource.addItem(new EngineCore.UIMenuItem("CrowdAgent", "CrowdAgent"));
navigationCreateSource.addItem(new EngineCore.UIMenuItem("CrowdManager", "CrowdManager"));
navigationCreateSource.addItem(new EngineCore.UIMenuItem("NavArea", "NavArea"));
navigationCreateSource.addItem(new EngineCore.UIMenuItem("Navigable", "Navigable"));
navigationCreateSource.addItem(new EngineCore.UIMenuItem("NavigationMesh", "NavigationMesh"));
navigationCreateSource.addItem(new EngineCore.UIMenuItem("DynamicNavigationMesh", "DynamicNavigationMesh"));
navigationCreateSource.addItem(new EngineCore.UIMenuItem("Obstacle", "Obstacle"));
navigationCreateSource.addItem(new EngineCore.UIMenuItem("OffMeshConnection", "OffMeshConnection"));

var networkCreateSource = new EngineCore.UIMenuItemSource();

networkCreateSource.addItem(new EngineCore.UIMenuItem("Network Priority", "create component"));

var physicsCreateSource = new EngineCore.UIMenuItemSource();

physicsCreateSource.addItem(new EngineCore.UIMenuItem("CollisionShape", "CollisionShape"));
physicsCreateSource.addItem(new EngineCore.UIMenuItem("Constraint", "Constraint"));
physicsCreateSource.addItem(new EngineCore.UIMenuItem("RigidBody", "RigidBody"));

var sceneCreateSource = new EngineCore.UIMenuItemSource();

sceneCreateSource.addItem(new EngineCore.UIMenuItem("Camera", "Camera"));
sceneCreateSource.addItem(new EngineCore.UIMenuItem("Light", "Light"));
sceneCreateSource.addItem(new EngineCore.UIMenuItem("Zone", "Zone"));

var subsystemCreateSource = new EngineCore.UIMenuItemSource();

subsystemCreateSource.addItem(new EngineCore.UIMenuItem("DebugRenderer", "create component"));
subsystemCreateSource.addItem(new EngineCore.UIMenuItem("Octree", "create component"));
subsystemCreateSource.addItem(new EngineCore.UIMenuItem("PhysicsWorld", "create component"));

var editorCreateSource = new EngineCore.UIMenuItemSource();

editorCreateSource.addItem(new EngineCore.UIMenuItem("CubemapGenerator", "CubemapGenerator"));


var componentCreateSource = new EngineCore.UIMenuItemSource();

var sources = {
    Audio: audioCreateSource,
    "2D": _2DCreateSource,
    Geometry: geometryCreateSource,
    Logic: logicCreateSource,
    Navigation: navigationCreateSource,
    Network: networkCreateSource,
    Physics: physicsCreateSource,
    Scene: sceneCreateSource,
    SubSystem: subsystemCreateSource,
    Editor : editorCreateSource
};

for (var sub in sources) {

    var item = new EngineCore.UIMenuItem(sub);
    item.subSource = sources[sub];
    componentCreateSource.addItem(item);

}


class CreateComponentButton extends EngineCore.UIButton {

    constructor() {

        super();

        this.fd.id = "Vera";
        this.fd.size = 11;

        this.text = "Create Component";

        this.subscribeToEvent(EngineCore.UIWidgetEvent((data) => this.handleWidgetEvent(data)));

    }

    // note instance method
    onClick = () => {

        var menu = new EngineCore.UIMenuWindow(this, "create component popup");
        menu.fontDescription = this.fd;
        menu.show(componentCreateSource);
    };

    handleWidgetEvent(ev: EngineCore.UIWidgetEvent) {

        if (ev.type != EngineCore.UI_EVENT_TYPE.UI_EVENT_TYPE_CLICK)
            return;

        if (ev.target && ev.target.id == "create component popup") {

            this.sendEvent(EngineEditor.SelectionCreateComponentEventData({ componentTypeName : ev.refid}));

            return true;

        }

    }

    fd: EngineCore.UIFontDescription = new EngineCore.UIFontDescription();

}

export = CreateComponentButton;
