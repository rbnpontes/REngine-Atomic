"use strict";
importScripts("../../../../../systemjs/system.js");
System.config({
    defaultJSExtensions: true,
    meta: {
        "../../../../modules/typescript.js": {
            format: "global",
            exports: "ts"
        }
    }
});
function firstTimeConnect(e) {
    System.import("./TypescriptLanguageServiceWebWorker").then((TypescriptLanguageServiceWebWorker) => {
        const worker = new TypescriptLanguageServiceWebWorker.default();
        self.removeEventListener("connect", firstTimeConnect);
        self.addEventListener("connect", worker.connect.bind(worker));
        worker.connect(e);
    });
}
self.addEventListener("connect", firstTimeConnect);
//# sourceMappingURL=workerLoader.js.map