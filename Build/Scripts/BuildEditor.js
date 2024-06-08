const { 
    editorBuild, 
    editorGenerate, 
    editorBuildFirstPhase, 
    editorBuildSecondPhase,
    editorCopyBinaries
} = require("./Tasks/BuildEditorTask");

namespace('editor', ()=> {
    task('build', editorBuild);
    task('gen', editorGenerate);
    task('firstphase', editorBuildFirstPhase);
    task('secondphase', editorBuildSecondPhase);
    task('cpybins', editorCopyBinaries);
});