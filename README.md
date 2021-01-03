# Rajce album downloader - Ultimate++ application

Rajce album downloader is the Ultimate++ GUI application to retrieve images/videos from the [www.rajce.net](http://www.rajce.net) (Czech social network and photo album). It supports downloading from password protected albums of the user.

[Ultimate++](http://www.ultimatepp.org/) is a C++ cross-platform rapid application development framework focused on programmers productivity. It includes a set of libraries (GUI, SQL, etc.), and an integrated development environment.

# AppVeyor build status

[AppVeyor](https://www.appveyor.com) is used to build Windows binaries and deploy them to the GitHub releases directory.

|branch  | status|
|---     |---    |
|master  |[![Build status](https://ci.appveyor.com/api/projects/status/github/CoolmanCZ/rajce?svg=true)](https://ci.appveyor.com/project/CoolmanCZ/rajce)|

*The Windows executable is currently not signed, which will show a warning when you run the .exe. Press 'more info' -> 'run anyway' to skip the warning.*

# Installation

## Windows
Simply download and unzip appropriate package for your windows version from the link below and then execute Rajce binary.

## Linux
The linux binary is not available. You have to build the binary manually and this requires some development tools installed in your linux distribution.

### Linux Build
Execute following commands to build linux binary

```bash
git clone https://github.com/CoolmanCZ/rajce.git
cd rajce
git submodule update --init --recursive
./GenerateCMakeFiles.sh
mkdir -p build
cd build
cmake .. && make
```

After successfull build the binary is stored in the ```build/bin``` directory

*Note: For clang build replace ```cmake .. && make``` with ```cmake -DCMAKE_TOOLCHAIN_FILE=../upp_cmake/utils/toolchain-clang.cmake .. && make```*

# Download

Download latest windows binaries and source code from [GitHub releases](https://github.com/CoolmanCZ/rajce/releases/).

# Changelog

* 1.5.12 - Version handling update, submodules update
* 1.5.11 - Fix of version information
* 1.5.10 - Submodules update, small code corrections
* 1.5.9 - Fix of #12 - album authorization is not working, submodules update
* 1.5.8 - Fix of #11 - video files download
* 1.5.7 - Submodules update, small code corrections
* 1.5.6 - Submodule path changed, small code corrections
* 1.5.5 - U++ framework code update, widget sorting, small code corrections
* 1.5.4 - Fix sha256 hash verification when downloading a new version of the application
* 1.5.3 - Show revision changes in the check and download application window
* 1.5.2 - Sha256 hash verification option when downloading a new version of the application
* 1.5.1 - Check and download a new application version directly from the GitHub
* 1.5.0 - Store multiple album URLs, with corresponding album username and authorization flag, to the configuration file
* 1.4.2 - Fix of the #10 - remove dots from the end of the album name in Windows OS
* 1.4.1 - Fix of the #9 - clear download list when new URL is set, new request and connection timeout options
* 1.4.0 - Fix of the #8 - new url parsing of video files
* 1.3.1 - Fix of the #6 - downloaded video files do not have full name
* 1.3.0 - New option - download video files
* 1.2.4 - Store settings to the configuration file, new option - download new files only
* 1.2.3 - Hide the http proxy settings if not used
* 1.2.2 - New option - http/https download protocol
* 1.2.1 - Correction of the runtime translation
* 1.2 - New option - append album user name to download directory
* 1.1 - Correction of the url video name, runtime language switch
* 1.0 - Initial version

# Screenshots

![img1](https://github.com/CoolmanCZ/rajce/raw/master/images/img1.png "screenshot 1")
![img2](https://github.com/CoolmanCZ/rajce/raw/master/images/img2.png "screenshot 2")

