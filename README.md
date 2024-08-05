
[![Build Status](https://github.com/rbnpontes/REngine-Atomic/actions/workflows/main.yml/badge.svg?event=push)](https://github.com/rbnpontes/REngine-Atomic/actions/workflows/main.yml)

#### [EARLY STAGE] Why REngine - Atomic ?

  

This project is a fork of Atomic Game Engine that is abandoned

This is a very cool project and can be my sucessor of my personal game engine called REngine.

**DISCLAIMER:** The project is in its early stages. If you try to build it yourself, you will face a lot of problems. However, if you find any kind of bug or problem, please open an issue so I can make a correction and improve the project.

### How to Build

You must have installed NodeJS 18+ and yarn, i recommend to use `nvm` to handle node versions

and install `yarn` through `npm`.


After that, you must run the above command at root of this project.

`yarn --cwd Build editor:build`

If you're under windows and needs Visual Studio Solution, you can run

`yarn --cwd Build editor:gen`

The generated solution is located at `THIS_REPO/Artifacts/Build/Windows/REngine.sln`

If you're under MacOS you do the same steps of Windows, but Xcode project will be at `THIS_REPO_PATH/Artifacts/Build/MacOS/REngine.xcodeproj`

### Features

- JavaScript/TypeScript Support (Will be change in the future #16)
- C# .NET Support (Will be change in the future #16)
- Android, iOS, WebGL (Not fully tested, and maybe need some improvements)
- Windows, MacOS and Linux Support
- Chromium WebView (It's unstable but can be usefull)
- Node Based Scene Graph
- Vulkan and D3D12 Support (Thanks to the Diligent Engine)
- Editor (Will be reworked #21)

### Credits

Huge thanks to the dedicated team behind Urho3D and Thunderbeast for crafting this incredible source code. It's truly disheartening to see it reach its conclusion, and to conclude in this manner. But your exceptional work will never be forgotten.