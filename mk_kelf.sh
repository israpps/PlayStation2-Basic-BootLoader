#!/bin/bash
rm -f bin/SYSTEM.XLF bin/XSYSTEM.XLF bin/HSYSTEM.XLF
echo compiling PS2 KELF
make clean kelf $* --silent
mv bin/PS2BBL_MC.KELF bin/SYSTEM.XLF
make clean $* --silent
echo compiling PSX KELF
make clean kelf PSX=1 $* --silent
mv bin/PSXBBL_MC.KELF bin/XSYSTEM.XLF
make clean PSX=1 $* --silent

echo compiling PS2 HDD KELF
make clean kelf HDD=1 $* --silent
mv bin/PS2BBL_HDD.KELF bin/HSYSTEM.XLF
make clean HDD=1 $* --silent

echo "done!"
