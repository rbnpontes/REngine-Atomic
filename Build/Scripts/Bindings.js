const { bindingsBuildTypescript, bindingsGenerate } = require("./Tasks/BindingsTask");

namespace('bindings', ()=> {
    task('gen', async ()=> await bindingsGenerate());
    task('build', async ()=> await bindingsBuildTypescript());
    task('all', async ()=> {
        await bindingsGenerate();
        await bindingsBuildTypescript();
    });
});