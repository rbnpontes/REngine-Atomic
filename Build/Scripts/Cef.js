const { 
    cefFetchBinaries,
    cefFetchResources,
    cefExtractBinaries,
    cefExtractResources,
    cefPrepare
} = require("./Tasks/CefTask");
/** @type {Boolean} */
const noclean = require('./BuildConfig')['noclean'];
namespace('cef', ()=> {
    [
        ['windows', 'Windows'],
        ['linux', 'Linux'],
        ['macos-arm64', 'MacOS_arm64'],
        ['macos-x86-64', 'MacOS_x86_64'],
    ].forEach(x => {
        const [task_name, platform] = x;

        namespace('prepare', ()=> {
            task(task_name, async ()=> {
                await cefPrepare(platform);
            });
        });

        namespace('download', ()=> {
            task(task_name, async ()=> {
                await cefFetchBinaries(platform, noclean);
            });
        });

        namespace('extract', ()=> {
            task(task_name, async ()=> {
                await cefExtractBinaries(platform);
            });
        });
    });

    namespace('download', ()=> {
        task('resources', async ()=> {
            await cefFetchResources(noclean);
        });
    });

    namespace('extract', ()=> {
        task('resources', async ()=> {
            await cefExtractResources();
        });
    });
});