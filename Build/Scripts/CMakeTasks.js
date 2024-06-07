const os = require('os');

switch(os.platform()) {
    case 'win32':
        {
            require('./BuildWindows')
            require('./CMakeWindows');
        }
        break;
    case 'darwin':
        require('./BuildMac');
        break;
    case 'linux':
        require('./BuildLinux');
        break;
}

require('./CMakeAndroid');
require("./BuildCommon");
require("./BuildAndroid");
require("./BuildIOS");
require("./BuildWeb");
require("./BuildAtomicNET");
require("./BuildLint");
require("./BuildTasks");
