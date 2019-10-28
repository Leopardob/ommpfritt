#!/usr/bin/env bash

#env: global:
PYBIND11_VERSION="2.2.4"

# before install
sudo apt-add-repository -y ppa:ubuntu-toolchain-r/test                           # gcc-8
sudo apt-add-repository -y ppa:beineri/opt-qt-5.12.3-xenial                      # qt-5.12
sudo apt-get update -qq
sudo apt-get -y upgrade
sudo apt-get install -y zlib1g-dev libssl-dev libffi-dev
sudo apt-get install -y git   # not required in travis

wget https://www.python.org/ftp/python/3.7.0/Python-3.7.0.tar.xz
tar xf Python-3.7.0.tar.xz

wget https://github.com/Kitware/CMake/releases/download/v3.14.5/cmake-3.14.5-Linux-x86_64.tar.gz
tar xf cmake-3.14.5-Linux-x86_64.tar.gz
cmake_3_14=$(pwd)/cmake-3.14.5-Linux-x86_64/bin/cmake

wget https://github.com/pybind/pybind11/archive/v${PYBIND11_VERSION}.tar.gz
tar xf v${PYBIND11_VERSION}.tar.gz
pybind11_DIR=pybind11-${PYBIND11_VERSION}

# install:
pushd Python-3.7.0
./configure
sudo make altinstall
popd

sudo python3.7 -m pip install pytest

mkdir -p $pybind11_DIR/build
pushd $pybind11_DIR/build
$cmake_3_14 ..
sudo make install
popd

sudo apt-get -y remove qt5-qmake qtbase5-dev qtbase5-dev-tools
sudo apt-get -y install libgl-dev python3-dev libpoppler-qt5-dev g++-8
sudo apt-get -y install qt512tools qt512translations qt512svg qt512base qt512imageformats
sudo apt-get -y install libkf5itemmodels-dev
export PATH=/opt/qt512/bin:$PATH

# script:
git clone https://github.com/pasbi/ommpfritt.git; cd ommpfritt  # not required in travis
mkdir build
cd build
qmake --version
$cmake_3_14 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++-8 -DQT_QM_PATH=/opt/qt512/translations/ ..
make -j 2

# after success:
touch success
