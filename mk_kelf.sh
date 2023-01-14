#!/bin/bash
echo compiling PS2 KELF
make clean kelf
mv bin/PS2BBL.KELF bin/SYSTEM.XLF
make clean
echo compiling PSX KELF
make clean kelf PSX=1
mv bin/PSXBBL.KELF bin/XSYSTEM.XLF

echo "done!"
