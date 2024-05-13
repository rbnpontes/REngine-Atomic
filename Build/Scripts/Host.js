var os = require('os');

if (os.platform() == "win32")
  module.exports = require("./HostWindows");
else if (os.platform() == "darwin")
  module.exports = require("./HostMac");
else if (os.platform() == "linux")
  module.exports = require("./HostLinux");
