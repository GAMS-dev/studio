# studio

GAMS Studio is the official development environment of the General Algebraic Modeling System (GAMS). For further information about GAMS please visit [GAMS](https://www.gams.com) or the [GAMS documentation](https://www.gams.com/latest/docs/).

GAMS Studio uses [Qt](https://www.qt.io/) which is licensed under [LGPL](https://www.gnu.org/licenses/lgpl-3.0.en.html). 

# Contribute

Contributions to the GAMS Studio project are highly appreciated! For futher information please check our [CONTRIBUTING.md](CONTRIBUTING.md) and [Code of Cunduct](CODE_OF_CONDUCT.md).

# How to build

## 1. Download and install Qt ##

The recommended way to get Qt is via its official [Qt online installer](https://www.qt.io/). If you are interested in the Qt sources you can download them through the installer and build Qt from scratch by following the [Qt documentation](https://doc.qt.io/qt-5/build-sources.html). Alternatively, you can get and build Qt from the official [Qt GitHub mirror](https://github.com/qt/qt5). Please note the the current version of GAMS Studio requires Qt 5.10.0 or later.

## 2. Download and install GAMS ##

GAMS Studio requires the GAMS low-level APIs. All those files are provided by the GAMS distribution packages, which are available for all major platforms. The installation package for your platform can be obtained from the [GAMS download page](https://www.gams.com/download/). After downloading the package please follow the latest GAMS [installation instructions](https://www.gams.com/latest/docs/UG_MAIN.html#UG_INSTALL).

**Note** By default GAMS will run in demo mode. Please check the [download page](https://www.gams.com/download/) for further details.

## 3. Get the GAMS Studio source code ##

Download the GAMS Studio sources from GitHub (via git or as zip archive). All information about the usage of this program can be found within the [**GAMS Documentation TODO**](https://www.gams.com/latest/docs/API_MAIN.html).

## 4. Building the GAMS Studio project ##

Start Qt Creator, open the project file 'src/studio.pro' and click 'Build/Run qmake'. This generates the file '**gamsinclude.pri**' that defines the default location of the GAMS installation location. The path within the 'gamsinclude.pri'  may have to be updated to the correct GAMS installation location on your system.

On Windows the file contains:
```
GAMS_DISTRIB=C:/GAMS/win64/24.9
GAMS_DISTRIB_API=$$GAMS_DISTRIB/apifiles/C/api
```
On Unix it will look like:
```
GAMS_DISTRIB=$$(HOME)/gams/gams24.9_linux_x64_64_sfx
GAMS_DISTRIB_API=$$GAMS_DISTRIB/apifiles/C/api
```

Then perform a 'Build All' operation (Build->Build All) to build the project. Finally, the GAMS Studio can be executed by triggering Run button or pressing 'Ctrl+R'.
