#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
    echo "Checking out"
    # TODO: Add your kernel build steps here
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE mrproper
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
    make -j4 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE all
fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir -p rootfs
cd "${OUTDIR}/rootfs"
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log
cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE distclean
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE install CONFIG_PREFIX="$OUTDIR/rootfs"
echo "Library dependencies "
CROSS_COMPILER_PATH=$(${CROSS_COMPILE}gcc -print-sysroot)
PROG_INTERPERTER_LIB=$(${CROSS_COMPILE}readelf -a busybox | grep -oP "(?<=program interpreter: /lib/).*(?=])")

PROG_INTERPERTER_LIB_PATH=$(find ${CROSS_COMPILER_PATH} -iname "${PROG_INTERPERTER_LIB}")
cp ${PROG_INTERPERTER_LIB_PATH} ${OUTDIR}/rootfs/lib/

SHARED_LIBS=$(${CROSS_COMPILE}readelf -a busybox | grep -oP "Shared library:\K[^;]*" | tr -d "[]")

for LIB_NAME in $SHARED_LIBS; do
    echo $LIB_NAME
    LIB_PATH=$(find ${CROSS_COMPILER_PATH} -iname "${LIB_NAME}")
    cp ${LIB_PATH} ${OUTDIR}/rootfs/lib64
done
# TODO: Make device nodes
cd "${OUTDIR}/rootfs"
sudo mknod -m 666 dev/null c 1 5
sudo mknod -m 666 dev/console c 3 1

# TODO: Clean and build the writer utility
cd "${FINDER_APP_DIR}"
make clean
make CROSS_COMPILE=${CROSS_COMPILE}
# TODO: Copy the finder related scripts and executables to the /home directory
mkdir -p ${OUTDIR}/rootfs/home/conf
cp writer_arm finder.sh finder-test.sh autorun-qemu.sh ${OUTDIR}/rootfs/home
cp conf/username.txt conf/assignment.txt ${OUTDIR}/rootfs/home/conf
# TODO: Chown the root directory
sudo chown -R root:root "${OUTDIR}/rootfs"
# TODO: Create initramfs.cpio.gz
cd "${OUTDIR}/rootfs"
find . | cpio -H newc -o | gzip > "${OUTDIR}/initramfs.cpio.gz"
