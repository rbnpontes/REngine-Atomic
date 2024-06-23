const { fetchCefBinaries, fetchCefResources } = require("./Tasks/CefTask");
/** @type {Boolean} */
const noclean = require('./BuildConfig')['noclean'];
namespace('cef', ()=> {
    namespace('download', ()=> {
        task('windows', async ()=> {
            await fetchCefBinaries('Windows', noclean);
        });
        task('linux', async ()=> {
            await fetchCefBinaries('Linux', noclean);
        });
        task('macosx-arm64', async ()=> {
            await fetchCefBinaries('MacOS_arm64', noclean);
        });
        task('macosx-x86-64', async ()=> {
            await fetchCefBinaries('MacOS_x86_64', noclean);
        });
        task('resources', async ()=> {
            await fetchCefResources(noclean);
        });
    });
});