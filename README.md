# What is QuadIron?

A Decentralized Storage with the Ethereum Blockchain

# Build from source

## Install the dependencies

- OpenSSL
- readline
- [libjson-rpc-cpp](https://github.com/cinemast/libjson-rpc-cpp)
- [NTTEC](https://github.com/vrancurel/nttec)

## Build

```sh
mkdir build
cd build
CXX=/usr/bin/clang++ cmake -G 'Unix Makefiles' ..
make
```

### Targets

The following targets are available:

- `doc`: build the documentation using Doxygen
- `format`: fix code source formatting
- `lint`: run the linter
- `fix-lint`: run the linter and try to apply the proposed fixes
- `quadiron`: build the QuadIron binary
- `quadiron_shared`: build the QuadIron shared library
- `quadiron_static`: build the QuadIron static library
- `quadiron_test`: build the test driver
- `check`: run the test suite

### Code coverage

By default the code coverage is not enabled.

To generate the code coverage reports:
1. set the option `ENABLE_COVERAGE` to `ON`.
2. recompile with `make`.
3. run the tests with `make check`.
4. extract coverage data with `make gcov`.
5. generate a report with `make lcov`.
6. open the report in `${CMAKE_BINARY_DIR}/lcov/html/all_targets`.

Note that, even though code coverage is supported by both Clang and GCC, result
with GCC seems more reliable (not surprising as we are using gcov).

# How to use it:

    $ ./quadiron -h
    usage: quadiron
       -b       n_bits
       -k       Kademlia K parameter
       -a       Kademlia alpha parameter
       -n       number of nodes
       -c       initial number of connections per node
       -N       number of files
       -S       random seed
    $ ./quadiron -n 100 -k 5
    initialize files
    checking files
    file adfa681fd77852ae who was referenced by 30a3d70a3d70a3d4 was not found
    1/5000 files wrongly stored
    quadiron>

In the example above, a replication factor of 5 is not sufficient to
guarantee a 100% hit on 100 nodes.

![Graphical Output of Simulator](graphviz.png )
