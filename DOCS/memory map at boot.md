## Here we have the memory map at boot##

0x0500-0x4500; Loader (16kb in size) ** Will probabbly be a lot more **
    0x4000-0x4199 Loaded shared library list
    0x4200-0x4500 Loaded shared library function list
    ** maybe we should let the compiler decide where he should put the shared library and shared function lists**
0x5000-0x8000 Memory manager : 5kb **DOESN'T HAVE MUCH HEADROOM** , 4 kb curretbly



0x7C00 : MBR
0x7D00 : Temporary sector for file searching and loading
0x8000 : Disk address packed structure for LBA reading int 13h

0x10000-0x20000 : stack

