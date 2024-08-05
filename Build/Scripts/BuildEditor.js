const { 
    editorBuild, 
    editorGenerate, 
    editorBuildFirstPhase, 
    editorBuildSecondPhase,
    editorCopyBinaries,
    editorPackage,
    editorCleanArtifacts
} = require("./Tasks/BuildEditorTask");

namespace('editor', ()=> {
    task('build', editorBuild);
    task('gen', editorGenerate);
    task('firstphase', editorBuildFirstPhase);
    task('secondphase', editorBuildSecondPhase);
    task('cpybins', editorCopyBinaries);
    task('pkg', editorPackage);
    task('clean', editorCleanArtifacts);
});