# studio

GAMS Studio is the official development environment of the General Algebraic Modeling System (GAMS). For further information about GAMS please visit [GAMS](https://www.gams.com) or the [GAMS documentation](https://www.gams.com/latest/docs/).

GAMS Studio uses [Qt](https://www.qt.io/) which is licensed under [LGPL](https://www.gnu.org/licenses/lgpl-3.0.en.html). 

# How to build

## 1. Download and install Qt ##

The recommended way to get Qt is via its official [Qt online installer](https://www.qt.io/).
If you are interested in the Qt sources you can download them through the 
installer or build Qt from scratch by following the [Qt documentation](https://doc.qt.io/qt-5/build-sources.html).
Alternatively, you can get and build Qt from the official [Qt GitHub mirror](https://github.com/qt/qt5).
Please check the [CONTRIBUTING.md](CONTRIBUTING.md) for the GAMS Studio C++ and Qt requirements.

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

# Dependencies and Licenses

| Dependency | License | Description |
| ------ | ------ | ------ |
| [Qt 6](https://www.qt.io/) | [LGPL](https://doc.qt.io/qt-6/lgpl.html) | [Qt Licensing](https://doc.qt.io/qt-6/licensing.html). The Qt 6 everywhere package (source) can be downloaded from [GAMS](https://d37drm4t2jghv5.cloudfront.net/qt/qt-everywhere-src-6.4.2.tar.xz) or directly from https://www.qt.io/download, where installers are provided as well. |
| [dtoaLoc](extern/dtoaloc) | [License](extern/dtoaloc/README.md) | |
| [engineapi](extern/engineapi/) | [MIT](extern/engineapi/README.md) | Owned by GAMS and can be used in other projects. |
| [yaml-cpp](extern/yaml-cpp/) | [MIT](extern/yaml-cpp/LICENSE/) | The project can be found at https://github.com/jbeder/yaml-cpp |

