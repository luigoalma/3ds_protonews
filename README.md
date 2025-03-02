# 3ds_protonews

Open source stub of `news` system module for a prototype variant.\
This variant seems to use `000400000f003500` as the title ID.

## Building

Have devkitPro tools available, as well makerom and just run `make`.

## License

This code itself is under Unlicense. Read `LICENSE.txt`\
The folders `source/3ds` and `include/3ds` have source files taken from [ctrulib](https://github.com/smealum/ctrulib), with modifications for the purpose of this program.\
Copy of ctrulib's license in `LICENSE.ctrulib.txt`

## Modifications to ctrulib

Ctrulib changed to generate smaller code, slimmed down sources and headers for a quicker read, and not depend on std libraries.\
Changes to SRV to use a prototype version of SRV's IPCs.\
Assembly inlined SVCs on headers.\
Additional helpers on `ipc.h` to help check command buffer.
