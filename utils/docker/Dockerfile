FROM ubuntu:20.04

# Set noninteractive mode to avoid tzdata prompts
ENV DEBIAN_FRONTEND=noninteractive

# Set a default timezone (e.g., "Etc/UTC")
RUN ln -fs /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    echo "Etc/UTC" > /etc/timezone

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    autoconf \
    libtool \
    pkg-config \
    sudo

# Set installation directory and PATH
ENV MY_INSTALL_DIR=$HOME/.local
ENV PATH="$MY_INSTALL_DIR/bin:$PATH"

# Create the installation directory
RUN mkdir -p $MY_INSTALL_DIR

# Clone and build gRPC
RUN git clone --recurse-submodules -b v1.62.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
WORKDIR /grpc/cmake/build
RUN cmake -DgRPC_INSTALL=ON \
          -DgRPC_BUILD_TESTS=OFF \
          -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
          ../..
RUN make -j$(nproc)
RUN make install

# Cleanup
WORKDIR /
RUN rm -rf /grpc

# Install thrift compiler and libraries
RUN apt-get install -y \
    thrift-compiler \
    libthrift-dev

# Install required packages for librcsc
RUN apt install -y build-essential libboost-all-dev autoconf automake libtool

# Install FUSE in the container
RUN apt-get update && apt-get install -y fuse libfuse2 libfuse3-3

# Allow non-root users to access FUSE (optional)
RUN chmod +x /usr/bin/fusermount

# Download linuxdeploy AppImage and extract it
RUN wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage \
    && chmod +x linuxdeploy-x86_64.AppImage \
    && ./linuxdeploy-x86_64.AppImage --appimage-extract \
    && mv squashfs-root /opt/linuxdeploy

# Set the working directory
WORKDIR /workspace/

# Default command
CMD ["/bin/bash"]
