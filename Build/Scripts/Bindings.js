const { bindingsBuildTypescript, bindingsGenerate, bindingsLintTypescript, bindingsCleanTypescript } = require("./Tasks/BindingsTask");

namespace('bindings', ()=> {
    task('gen', async ()=> await bindingsGenerate());
    task('build', async ()=> await bindingsBuildTypescript());
    task('lint', async ()=> await bindingsLintTypescript());
    task('clean', async ()=> await bindingsCleanTypescript());
    task('all', async ()=> {
        await bindingsCleanTypescript();
        await bindingsGenerate();
        await bindingsLintTypescript();
        await bindingsBuildTypescript();
    });
});