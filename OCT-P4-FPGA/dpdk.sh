#!/bin/bash
mkdir dpdk
cd dpdk
sudo apt --assume-yes install build-essential
sudo apt --assume-yes install libnuma-dev
sudo apt --assume-yes install pkg-config
sudo apt --assume-yes install python3 python3-pip python3-setuptools
sudo apt --assume-yes install python3-wheel python3-pyelftools
sudo apt --assume-yes install ninja-build
sudo pip3 install meson
sudo apt install --assume-yes libpcap-dev
sudo apt install --assume-yes linux-headers-4.15.0-169-generic

git clone https://github.com/Xilinx/dma_ip_drivers.git
cd dma_ip_drivers
git checkout 7859957
cd ..
git clone https://github.com/Xilinx/open-nic-dpdk.git
cp open-nic-dpdk/*.patch dma_ip_drivers
cd dma_ip_drivers
git apply *.patch
cd ..
wget https://fast.dpdk.org/rel/dpdk-20.11.tar.xz
tar xvf dpdk-20.11.tar.xz
cd dpdk-20.11
cp -R ../dma_ip_drivers/QDMA/DPDK/drivers/net/qdma ./drivers/net
cp -R ../dma_ip_drivers/QDMA/DPDK/examples/qdma_testapp ./examples
# add qdma to the drivers/net/meson.build
sed -i "47i 'qdma'," drivers/net/meson.build
cd ..
wget https://git.dpdk.org/apps/pktgen-dpdk/snapshot/pktgen-dpdk-pktgen-21.03.1.tar.xz
tar xvf pktgen-dpdk-pktgen-21.03.1.tar.xz
cd dpdk-20.11
meson build
cd build
ninja
sudo ninja install
ls -l /usr/local/lib/x86_64-linux-gnu/librte_net_qdma.so
sudo ldconfig
ls -l ./app/test/dpdk-test
cd ../..
pwd
cd pktgen-dpdk-pktgen-21.03.1
make RTE_SDK=../dpdk-20.11 RTE_TARGET=build
cd ../dpdk-20.11/usertools
sed -i '62s/\[network_class, cavium_pkx, avp_vnic, ifpga_class\]/\[network_class, cavium_pkx, avp_vnic, ifpga_class, qdma\]/' dpdk-devbind.py
sed -i "38i qdma = {'Class': '02', 'Vendor': '10ee', 'Device': '903f,913f'," dpdk-devbind.py
sed -i "39i                'SVendor': None, 'SDevice': None}" dpdk-devbind.py
cp dpdk-devbind.py /usr/local/bin/.
