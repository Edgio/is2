#!/bin/bash
# ----------------------------------------------------------------------------
# Copyright (C) 2018 Verizon.  All Rights Reserved.
# All Rights Reserved
#
#   Author: Reed Morrison
#   Date:   01/06/2018
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
# To build...
# ------------------------------------------------------------------------------
which cmake g++ make || {
    echo "Failed to find required build packages. Please install with:   sudo apt-get install cmake make g++"
    exit 1
}
# ------------------------------------------------------------------------------
# Build is2
# ------------------------------------------------------------------------------
set -o errexit
mkdir -p build && \
pushd build && \
    cmake ../ \
    -DFORTIFY=ON \
    -DBUILD_TESTS=ON \
    -DCMAKE_INSTALL_PREFIX=/usr \
    && \
    make -j$(nproc) && \
    make test && \
popd

