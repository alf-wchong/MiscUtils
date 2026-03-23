# Use the latest Fedora base image
FROM fedora:latest

# Install MinGW-w64, development tools, and utilities
RUN dnf -y update && \
    dnf -y install \
    mingw64-gcc \
    mingw64-gcc-c++ \
    mingw64-binutils \
    mingw64-headers \
    make \
    cmake \
    sudo \
    vim \
    nano \
    tree \
    less \
    procps-ng \
    && dnf clean all

# Create the user 'fed' with a home directory
RUN useradd -m -s /bin/bash fed

# Configure passwordless sudo for 'fed'
RUN echo "fed ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Set the working directory
WORKDIR /home/fed

USER fed
ENTRYPOINT ["sleep", "infinity"]
