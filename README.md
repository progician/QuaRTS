[![Build Status](https://travis-ci.org/progician/QuaRTS.svg?branch=master)](https://travis-ci.org/progician/QuaRTS)
# QuaRTS

A real-time strategy game, inspired by Blizzard's Starcraft series. It aims to be an abstract game, like chess, without background story or story specific details.

## Getting started

Follow these instructions to be able to build the software on your system for playing or development.

### Requirements

* C++17 capable compiler
* CMake
* Ninja
* Conan package manager (optional, but strongly recommended)

### Setting up the build environment

The simplest way to be get up and running quickly is to use a Python virtual environment, like so:

Use the system's python command to create a python virtual environment (assuming that there is a python3 interpreter on the computer - also strongly recommended)
```
$ python -m virtualenv -p python3 env
```

Activate the virtual environment
```
$ source env/bin/activate
```

Install the build system dependencies.
```
$ pip install cmake conan ninja
```


### Build and test the project

Once the build dependencies are in place and available, you can proceed to build the project, using the the following instructions:

Check out the sourc code in the QUARTS_ROOT directory
```
$ cd $QUARTS_ROOT
$ git clone https://github.com/progician/QuaRTS.git
```

Use `conan` to install the dependencies of the project and store the dependency paths in a generated cmake file in the build directory. Assuming the build directory is called `build`.
```
$ conan install -if build --build=missing .
```
This command will also build any dependencies that has no binaries available for the current platform.

Configure the project with CMake. Developers are advised to use either Debug or RelWithDebInfo build type, while end-users should build in Release mode.
```
$ cmake -Bbuild -GNinna -DCMAKE_BUILD_TYPE=RelWithDebInfo .
```

Build the software. This step can be repeated for incremental development.
```
$ ninja -C build
```
This command will trigger, by default, the building and running of the unit test suite of the project. End-users might not interested in testing the project, they can configure with `-DUSE_UNIT_TESTS=OFF` which will prevent it entirely.

However, unit tests should run realy quickly and so I do not recommend turning them off.