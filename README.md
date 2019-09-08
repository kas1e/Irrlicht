# Irrlicht 1.8.4 AmigaOS4 port

to build release version:

cd source/Irrlicht
make -f Makefile.amigaos4 NDEBUG=1

This will compile Irrlicht, create a static lib (libIrrlicht.a), and copy it
into the subdirectory lib/AmigaOS4. That's all.


to build debug version:

cd source/Irrlicht
make -f Makefile.amigaos4


For cross-compilation you may add -j4 at end to speed up compilation process.
