# pd-hvccgen
Creating a gen~-like environment for Pd, based on the Heavy compiler

Currently still in development, it works but the editor is still unfinished.

<img width="759" alt="Screenshot 2022-08-07 at 00 43 21" src="https://user-images.githubusercontent.com/44585538/183270437-3ad27d96-181d-4794-9abb-a170a1ec9e3b.png">

# Build instructions

Dependencies: Python3, C/C++ compiler

```
git clone --recursive https://github.com/timothyschoen/pd-hvccgen.git
mkdir build && cd build
cmake .. -DHVCC_PATH=PATH_TO_HVCC
cmake --build .

```
Replace PATH_TO_HVCC to the hvcc executable.

After running, the pd external should be in the ./binaries folder.
