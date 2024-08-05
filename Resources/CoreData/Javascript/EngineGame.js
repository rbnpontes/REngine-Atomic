
EngineCore.editor = null;

function Game() {

    this.engine = EngineCore.getEngine();
    this.cache = EngineCore.getResourceCache();
    this.renderer = EngineCore.getRenderer();
    this.graphics = EngineCore.getGraphics();
    this.input = EngineCore.getInput();

    this.input.setMouseVisible(true);

    if (EngineCore.platform == "Android") {
        this.renderer.reuseShadowMaps = false;
        this.renderer.shadowQuality = EngineCore.SHADOWQUALITY_LOW_16BIT;
    }

    // root view
    this.uiView = new EngineCore.UIView();

}

Game.prototype.init = function (start, update) {

    this.update = update;

    // register global to get at quickly
    __js_atomicgame_update = update;

    if (typeof (start) === "function")
        start();

}

Game.prototype.getSpriteSheet2D = function (xmlFile) {

    return this.cache.getResource("SpriteSheet2D", xmlFile);

}

Game.prototype.getSpriteSheet = Game.prototype.getSpriteSheet2D;

Game.prototype.getSound = function (soundFile) {

    return this.cache.getResource("Sound", soundFile);

}

Game.prototype.getSprite2D = function (spriteFile) {

    return this.cache.getResource("Sprite2D", spriteFile);

}


Game.prototype.showDebugHud = function () {

    var uiStyle = this.cache.getResource("XMLFile", "UI/DefaultStyle.xml");
    var debugHud = this.engine.createDebugHud();
    debugHud.defaultStyle = uiStyle;
    debugHud.toggleAll();

}

Game.prototype.createScene2D = function () {

    var scene = new EngineCore.Scene();
    scene.createComponent("Octree");

    var cameraNode = scene.createChild("Camera");
    cameraNode.position = [0.0, 0.0, -10.0];

    var camera = cameraNode.createComponent("Camera");
    camera.orthographic = true;
    camera.orthoSize = this.graphics.height * EngineCore.PIXEL_SIZE;

    var viewport = null;

    if (EngineCore.editor) {
        viewport = EngineCore.editor.setView(scene, camera);
    } else {
        viewport = new EngineCore.Viewport(scene, camera);
        this.renderer.setViewport(0, viewport);
    }

    this.scene = scene;
    this.cameraNode = cameraNode;
    this.camera = camera;
    this.viewport = viewport;

    return scene;

}

Game.prototype.dumpMetrics = function () {
    const metrics = EngineCore.getVM().metrics;
    metrics.capture();
    print("--------------");
    print("Object Instances:");
    print("--------------");
    metrics.dump();
    print("--------------");
    print("Nodes:");
    print("--------------");
    metrics.dumpNodes();
    print("--------------");
    print("JS Components:");
    print("--------------");
    metrics.dumpJSComponents();
};

Game.prototype.createScene3D = function (filename) {

    var scene = new EngineCore.Scene();

    // FIXME: Node should take a string name in constructor
    var cameraNode = new EngineCore.Node();
    cameraNode.name = "Camera";
    cameraNode.position = [0.0, 0.0, -10.0];

    var camera = cameraNode.createComponent("Camera");

    this.cameraNode = cameraNode;
    this.camera = camera;

    if (typeof (filename) == "string")
        scene.loadXML(filename)
    else
        scene.createComponent("Octree");

    scene.addChild(cameraNode);

    var viewport = null;
    if (EngineCore.editor) {
        viewport = EngineCore.editor.setView(scene, camera);
    } else {
        viewport = new EngineCore.Viewport(scene, camera);
        this.renderer.setViewport(0, viewport);
    }

    this.scene = scene;
    this.viewport = viewport;

    return scene;

}


EngineCore.game = exports.game = new Game();
