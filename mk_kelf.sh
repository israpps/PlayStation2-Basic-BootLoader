#!/bin/bash
make banner

DATE=$(date "+%d-%m-%Y")
SHA8=$(git rev-parse --short HEAD)
TARGET="PS2BBL_KELF-[$DATE]-[$SHA8]"

rm -rf kelf
rm -f PS2BBL_KELF.7z
mkdir -p kelf/MX4SIO

echo -- PS2
make clean kelf $* --silent
mv bin/PS2BBL_MC.KELF kelf/SYSTEM.XLF
make clean $* --silent

echo -- PSX
make clean kelf PSX=1 $* --silent
mv bin/PSXBBL_MC.KELF kelf/XSYSTEM.XLF
make clean PSX=1 $* --silent

echo -- PS2 + HDD
make clean kelf HDD=1 $* --silent
mv bin/PS2BBL_HDD.KELF kelf/HSYSTEM.XLF
make clean HDD=1 $* --silent

echo -- PS2 + MX4SIO
make clean kelf MX4SIO=1 $* --silent
mv bin/PS2BBL_MC.KELF kelf/MX4SIO/SYSTEM.XLF
make clean $* --silent

echo -- PSX + MX4SIO
make clean kelf PSX=1 MX4SIO=1 $* --silent
mv bin/PSXBBL_MC.KELF kelf/MX4SIO/XSYSTEM.XLF
make clean PSX=1 $* --silent

cp LICENSE kelf/LICENSE.TXT
cp README.md kelf/README.md
mv kelf/ $TARGET
7z a -t7z PS2BBL_KELF.7z "$TARGET/*"
rm -rf "$TARGET/"
echo "done!"
