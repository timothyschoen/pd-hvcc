# pd-hvccgen
Creating a gen~-like environment for Pd, based on the Heavy compiler

Currently still in development, it works but the editor is still unfinished.

<img width="821" alt="Screenshot 2022-08-10 at 17 50 23" src="https://user-images.githubusercontent.com/44585538/183952750-4eb06938-519d-46f0-9617-5cfe02cd468e.png">

# Build instructions

**Important:**
- Please ensure that the git submodules are initialized and updated! You can use the `--recursive` option while cloning or `git submodule update --init --recursive` in the PlugData repository .
- On Linux, JUCE framework requires to install dependencies, please refer to [Linux Dependencies.md](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md) and use the full command.
- The CMake build system has been tested with *Unix Makefiles*, *XCode*

```
git clone --recursive https://github.com/timothyschoen/pd-hvccgen.git
mkdir build && cd build
cmake ..
make
make install
```

After running, the pd external will be installed to ~/Documents/Pd/externals

# Setup Instructions

If you receive a message in the console that python3 or a c++ compiler wasn't found, open the settings dialog and point to the python3 and c++ compiler you want to use.

On macOS (through homebrew):
```
command xcode-select --install
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew install python3

```
On Linux, use your package manager to install python3 and gcc (clang is also supported as a replacement for gcc).

# Usage Instructions
- Create "hvcc~" object
- Click on object to open, or use right-click -> open
- Create a patch and hit compile!
