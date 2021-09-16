#!/bin/bash
# ------------------------------------------------------------------------------
# Usage
# ------------------------------------------------------------------------------
usage() {
	cat <<EOM
$0: [hs]
	-h display some help, you know, this
	-s shallow clone, useful for faster builds
	-p install prefix

-s doesn't work with git prior to 2.8 (e.g. xenial)
EOM
	exit 1
}
# ------------------------------------------------------------------------------
# Requirements to build...
# ------------------------------------------------------------------------------
check_req() {
	which cmake g++ make || {
		echo "Failed to find required build packages. Please install with: sudo apt-get install cmake make g++"
		exit 1
	}
}
# ------------------------------------------------------------------------------
# build...
# ------------------------------------------------------------------------------
main() {
	check_req
	mkdir -p build
	pushd build && \
		cmake ../ \
		-DBUILD_SYMBOLS=ON \
		-DBUILD_TLS=ON \
		-DCMAKE_INSTALL_PREFIX=${install_prefix} && \
		make -j${NPROC} && \
		umask 0022 && chmod -R a+rX . && \
		make package && \
		popd && \
	exit $?
}
install_prefix="/usr"
#parse options
while getopts ":hsp:" opts; do
	case "${opts}" in
		h)
			usage
			;;
		p)
			install_prefix="${OPTARG}"
			;;
		*)
			usage
			;;
	esac
done

main
