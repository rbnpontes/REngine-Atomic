const { bindingsBuildTypescript, bindingsGenerate, bindingsLintTypescript } = require("./Tasks/BindingsTask");

namespace('bindings', ()=> {
    task('gen', async ()=> await bindingsGenerate());
    task('build', async ()=> await bindingsBuildTypescript());
    task('lint', async ()=> await bindingsLintTypescript());
    task('all', async ()=> {
        await bindingsGenerate();
        await bindingsBuildTypescript();
    });
});