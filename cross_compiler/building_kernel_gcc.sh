mkdir build-gcc
cd build-gcc

gcc-13.2.0/configure \
  --target=i686-elf \
  --prefix=/mingw64/cross \
  --disable-nls \
  --enable-languages=c \
  --without-headers \
  --disable-hosted-libstdcxx \
  --disable-libssp \
  --disable-libmudflap \
  --disable-libgomp \
  --disable-libquadmath \
  --disable-libatomic

make all-gcc -j$(nproc)
make install-gcc
