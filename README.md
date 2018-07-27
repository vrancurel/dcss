# DCSS

[![CircleCI][badgepub]](https://circleci.com/gh/vrancurel/dcss)

A Decentralized Storage with the Ethereum Blockchain

## Build from source

### Install the dependencies

- OpenSSL
- readline
- [libjson-rpc-cpp](https://github.com/cinemast/libjson-rpc-cpp)
- [QuadIron](https://github.com/scality/quadiron)

### Build

```sh
mkdir build
cd build
CXX=/usr/bin/clang++ cmake -G 'Unix Makefiles' ..
make
```

#### Targets

The following targets are available:

- `doc`: build the documentation using Doxygen
- `format`: fix code source formatting
- `lint`: run the linter
- `fix-lint`: run the linter and try to apply the proposed fixes
- `dcss`: build the DCSS binary
- `shared`: build the DCSS shared library
- `static`: build the DCSS static library
- `unit_tests`: build the unit tests
- `check`: run the test suite

#### Code coverage

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

## How to use it:

    $ ./dcss -h
    usage: dcss
       -l       path to the logging configuration
       -b       n_bits
       -k       Kademlia K parameter
       -a       Kademlia alpha parameter
       -n       number of nodes
       -c       initial number of connections per node
       -N       number of files
       -S       random seed
    $ ./dcss -n 100 -k 5
    initialize files
    checking files
    file adfa681fd77852ae who was referenced by 30a3d70a3d70a3d4 was not found
    1/5000 files wrongly stored
    dcss>

In the example above, a replication factor of 5 is not sufficient to
guarantee a 100% hit on 100 nodes.

![Graphical Output of Simulator](graphviz.png )

### Logging configuration

DCSS uses EasyLogging as a logging framework and the loggers can be
configured through a dedicated configuration file (which you can specify by
using the option `-l`, if not provided a default configuration will be applied).

DCSS has three loggers that can be configured independently:
- "simulator": logging for the simulated network.
- "ethereum": logging for the Ethereum transactions.
- "DHT": logging for the DHT layer.

For more details on how to configure the loggers, see
[the official documentation of EasyLogging](https://github.com/muflihun/easyloggingpp#configuration)

[badgepub]: https://circleci.com/gh/vrancurel/dcss.svg?style=shield&circle-token=:circle-token
