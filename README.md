# GAMS Studio

GAMS Studio is the official development environment of the General Algebraic Modeling System (GAMS). For further information about GAMS please visit the official [GAMS](https://www.gams.com) website, the [GAMS documentation](https://www.gams.com/latest/docs/) or the [GAMS Forum](https://forum.gams.com/), where Studio also has its own [FAQ section](https://forum.gams.com/c/faq/gams-studio-faqs/17).

# How to install

To install GAMS Studio on your system follow the steps for your platform. On Linux and Windows Studio is installed into the GAMS system directory (`<GAMS SysDir>`), which is the installation location of GAMS. Only on macOS Studio will be installed in the Application directory.

## Linux

1. Download the AppImage
2. Copy the AppImage to the `<GAMS SysDir>\studio`
3. Make it runnable via `chmod +x <GAMS SysDir>\studio\*.AppImage`
4. Run Studio via the shell or via your file manager

## macOS

1. Download the DMG for your macOS (Intel - x86 or Apple Silicon - ARM)
2. Double click on the dmg to open it. A new window should open and show a GAMS icon.
3. Move the Studio directory (via dragging and dropping the icon) to the Applications directory within your Finder window. Replace any existing old version, if a message is shown.
4. Go to the Application directory or go to the Launchpad and open the "GAMS Studio"

## Windows

1. Download the zip file and extract it
2. Copy the new directory (`studio`) to the `<GAMS SysDir>` to replace the previous Studio version or rename the `studio` directory before copying.
3. Run Studio via double clicking on the `studio.exe`

# How to build

## 1. Download and install Qt ##

The recommended way to get Qt is via its official [Qt online installer](https://www.qt.io/).
If you are interested in the Qt sources you can download them through the 
installer or build Qt from scratch by following the [Qt documentation](https://doc.qt.io/qt-6/build-sources.html).
Alternatively, you can get and build Qt from the official [Qt GitHub mirror](https://github.com/qt/qt5).

## 2. Download and install GAMS ##

GAMS Studio requires the GAMS low-level APIs. All those files are provided by the GAMS distribution packages, which are available for all major platforms. The installation package for your platform can be obtained from the [GAMS download page](https://www.gams.com/download/). After downloading the package please follow the latest GAMS [installation instructions](https://www.gams.com/latest/docs/UG_MAIN.html#UG_INSTALL).

**Note** By default GAMS will run in demo mode. Please check the [download page](https://www.gams.com/download/) for further details.

## 3. Get the GAMS Studio source code ##

Download the GAMS Studio sources from GitHub (via git or as zip archive). All information about the usage of this program can be found within the [GAMS Documentation](https://www.gams.com/latest/docs/T_STUDIO.html).

## 4. Building the GAMS Studio project ##

Start Qt Creator, open the project file `gams-studio.pro` and click `Build -> Run qmake`. This generates the file `gamsinclude.pri` that defines the default location of the GAMS installation location. The path within the `gamsinclude.pri` may have to be updated to the correct GAMS installation location on your system.

On Windows the file contains:
```
GAMS_DISTRIB=C:/GAMS/35
GAMS_DISTRIB_C_API=$$GAMS_DISTRIB/apifiles/C/api
GAMS_DISTRIB_CPP_API=$$GAMS_DISTRIB/apifiles/C++/api
```
On Unix it will look like:
```
GAMS_DISTRIB=$$(HOME)/gams/gams35.0_linux_x64_64_sfx
GAMS_DISTRIB_C_API=$$GAMS_DISTRIB/apifiles/C/api
GAMS_DISTRIB_CPP_API=$$GAMS_DISTRIB/apifiles/C++/api
```

Then perform a `Build All` operation (`Build -> Build All`) to build the project. Finally, the GAMS Studio can be executed by triggering `Run` button or pressing `Ctrl + R`.

# Contributing

Your contributions to the GAMS Studio project are highly appreciated! Depending on
your type of improvement you may want to [create an issue](https://help.github.com/en/articles/creating-an-issue)
or [fork GAMS Studio](https://guides.github.com/activities/forking/) and open a pull
request when your changes are ready.

Before you request a review of your changes please make sure that you used the latest [GAMS release](https://www.gams.com/download/) for development and that your code is following the [Qt Coding Style](https://wiki.qt.io/Qt_Coding_Style).

# Dependencies and Licenses

| Dependency | License | Description |
| ------ | ------ | ------ |
| [Qt 6](https://www.qt.io/) | [LGPL](https://doc.qt.io/qt-6/lgpl.html) | [Qt Licensing](https://doc.qt.io/qt-6/licensing.html). The Qt 6 everywhere package (source) can be downloaded from [GAMS](https://d37drm4t2jghv5.cloudfront.net/qt/qt-everywhere-src-6.9.1.tar.xz) or directly from https://www.qt.io/download, where installers are provided as well. |
| [dtoaLoc](extern/dtoaloc) | [License](extern/dtoaloc/README.md) | |
| [engineapi](extern/engineapi/) | [MIT](extern/engineapi/README.md) | Owned by GAMS and can be used in other projects. |
| [yaml-cpp](extern/yaml-cpp/) | [MIT](extern/yaml-cpp/LICENSE/) | The project can be found at https://github.com/jbeder/yaml-cpp |
| [JetBrains Mono](https://www.jetbrains.com/de-de/lp/mono/) | [SIL Open Font License 1.1](https://github.com/JetBrains/JetBrainsMono/blob/master/OFL.txt) | Monospace font for editor from https://www.jetbrains.com/de-de/lp/mono/ |
| [Source Code Pro](https://github.com/adobe-fonts/source-code-pro) | [SIL Open Font License 1.1](https://github.com/adobe-fonts/source-code-pro/blob/release/LICENSE.md) | Monospace font for editor from https://github.com/adobe-fonts/source-code-pro |
| [Fira Code](https://github.com/tonsky/FiraCode) | [SIL Open Font License 1.1](https://github.com/tonsky/FiraCode/blob/master/LICENSE) | Monospace font for editor from https://github.com/tonsky/FiraCode |
| [Cousine](https://font.download/font/cousine) | [apache 2.0 license](http://www.apache.org/licenses/LICENSE-2.0) | Monospace font for editor from https://font.download/font/cousine |
