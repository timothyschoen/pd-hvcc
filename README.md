# pd-hvccgen
Creating a gen~-like environment for Pd, based on the Heavy compiler

Currently still in development, it works but the editor is still unfinished.

<img width="821" alt="Screenshot 2022-08-10 at 17 50 23" src="https://user-images.githubusercontent.com/44585538/183952750-4eb06938-519d-46f0-9617-5cfe02cd468e.png">

# Build instructions

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

I'll add instructions for installing these here soon!


# Usage Instructions


- Create "hvcc~" object
- Click on object to open, or use right-click -> open
- Create a patch and hit compile!
