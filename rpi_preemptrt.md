# Compiling PreemptRT for RPi 4B

Cross-compiled on Ubuntu 18.04.

Install dependencies.

    sudo apt update
    sudo apt install build-essential bison byacc flex

Create build directories.

    mkdir ~/rpi-kernel
    cd ~/rpi-kernel
    mkdir rt-kernel

Clone the raspberry pi kernel repo (preempt rt branch) and tools repo.

    git clone https://github.com/raspberrypi/linux.git -b rpi-4.19.y-rt
    git clone https://github.com/raspberrypi/tools.git

Set environment variables.

    export ARCH=arm
    export CROSS_COMPILE=~/rpi-kernel/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-
    export INSTALL_MOD_PATH=~/rpi-kernel/rt-kernel
    export INSTALL_DTBS_PATH=~/rpi-kernel/rt-kernel

Make the configuration (e.g., for Pi4B, BCM2711).

    # For Pi4B
    export KERNEL=kernel7l
    cd ~/rpi-kernel/linux/
    make bcm2711_defconfig

    # For Pi 2/3B(+)
    #export KERNEL=kernel7
    #cd ~/rpi-kernel/linux/
    #make bcm2709_defconfig


Compile the kernel.

    make -j12 zImage
    make -j12 modules
    make -j12 dtbs
    make -j12 modules_install # make note of the kernel version (e.g., DEPMOD  4.19.71-rt24-v7l+)
    make -j12 dtbs_install

    mkdir $INSTALL_MOD_PATH/boot
    ./scripts/mkknlimg ./arch/arm/boot/zImage $INSTALL_MOD_PATH/boot/$KERNEL.img
    cd $INSTALL_MOD_PATH/boot
    mv $KERNEL.img kernel7_rt.img
 
Tar/gzip up the files and transfer them to the Pi:

    cd $INSTALL_MOD_PATH
    tar czf ../rt-kernel.tgz *
    cd ..
    scp rt-kernel.tgz pi@<pi_address>:/tmp

Now, on the Pi, extract and install the kernel:

    cd /tmp
    tar xzf rt-kernel.tgz
    cd boot
    sudo cp -rd * /boot/
    cd ../lib
    sudo cp -dr * /lib/
    cd ../overlays
    sudo cp -d * /boot/overlays
    cd ..
    sudo cp -d bcm* /boot/
    
Modify /boot/config.txt:

    # add
    kernel=kernel7_rt.img

Reboot the Pi, and verify the rt kernel is running:

    uname -r # should see the kernel version, e.g. 4.19.71-rt24-v7l+

