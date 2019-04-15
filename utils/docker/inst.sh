#!/bin/bash

set -x

PASS=pmdkpass

cd
echo $PASS | sudo -E dnf install -y zlib-devel

# Install OpenSSL & OpenSSH
git clone https://github.com/openssl/openssl.git
git clone https://github.com/openssh/openssh-portable.git openssh

cd openssl
git checkout -b bOpenSSL_1_1_1b OpenSSL_1_1_1b
./config --prefix=/usr --openssldir=/usr/ssl
make -j12
echo $PASS | sudo -E make install

cd ../openssh
git checkout -b bV_7_9_P1 V_7_9_P1
autoreconf
./configure --prefix=/usr
make -j12
echo $PASS | sudo -E make install
