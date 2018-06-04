# Yet Another Storage
YAS is a header-only highly template library. At the moment the library supports unicode, 32/64 bit operation mode(in 32 bit size of PV is limited by 2**32 bytes but it implies less overhead) and heterogeneous devices. 

For more info please see project wiki.

## Test

```shell
# build test binaries
make

# run tests
make tests
```

The default test binaries will be built in release mode. You can make Debug test binaries as well:

```shell
make clean
make debug
make tests
```

## Project templates

You can find project templates for VS 2017 and gcc 7 in exmaples. To change unicode support, bitness and device please change the corresponding declarations in include/storage/settings.h.

## Dependences 

1. nonstd::expected (https://github.com/martinmoene/expected-lite) - header-only
2. gtest (https://github.com/google/googletest) for tests
