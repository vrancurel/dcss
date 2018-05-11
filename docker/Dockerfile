FROM ubuntu:18.04
MAINTAINER Sylvain Laperche "sylvain.laperche@scality.com"

RUN apt-get update && apt-get -y --no-install-recommends install \
    ca-certificates \
    clang \
    clang-tidy \
    clang-format \
    cmake \
    doxygen \
    git \
    libargtable2-dev \
    libcurl4-openssl-dev \
    libhiredis-dev \
    libjsoncpp-dev \
    libmicrohttpd-dev \
    libreadline-dev \
    make

# Fix binary names.
RUN ln -s /usr/bin/clang-format-6.0 /usr/bin/clang-format &&\
    ln -s /usr/bin/clang-tidy-6.0   /usr/bin/clang-tidy

# Install libjson-rpc-cpp from the source (the version in the repo is too old).
RUN git clone git://github.com/cinemast/libjson-rpc-cpp.git &&\
    cd libjson-rpc-cpp && git checkout v1.0.0 &&\
    mkdir build && cd build &&\
    cmake -DCOMPILE_TESTS=No .. && make && make install &&\
    cd ../../ && rm -rf libjson-rpc-cpp

# Install NTTEC.
RUN git clone git://github.com/scality/nttec.git &&\
    mkdir nttec/build && cd nttec/build &&\
    cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release .. && make install

RUN ldconfig
