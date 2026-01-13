mkdir build-gcc
cd build-gcc

../gcc-13.2.0/configure \
  --target=$TARGET \
  --prefix=$PREFIX \
  --disable-nls \
  --enable-languages=c \
  --without-headers

make all-gcc -j$(nproc)
make install-gcc
