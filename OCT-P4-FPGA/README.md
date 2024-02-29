# P4 Examples on OCT
# BUILD
For build, refer to instructions under P4Framework

-------------------------------------------------------------
## DEPLOYMENT on OCT.  

## Request P4-nodes from Cloudlab
Request nodes from Cloudlab. For the P4Framework examples, you will need to request at least two nodes. One is used to deploy P4 apps. The other is to send the packets. Note: The second node is not necessarily an FPGA node. However, only FPGA node in OCT provides 100G link.

## Copy necessary files to the deployment machines
Copy the `$(APP).dist to the FPGA node for the deployment. Copy the test pcap files to the other node. We will send these pcap packets to the P4-FPGA node.


## Config the FPGA and host

Step 1: 
User can use either JTAG programming or PCIe programming. On OCT, we use PCIe programming for security issues.

For PCIe programming (on OCT nodes): we utilize Xilinx xbflash2 tool to program the FPGA. 
`sudo xbflash2 program --spi --image <path to the mcs> -d 3b:00.0 --bar 2`

Step 2: Cold reboot the host machine to install the bitstream. To do so on OCT, you need to go to the cloudlab homepage to execute power cycle.

Next,  you need to build the OpenNIC kernel module and load it.

Step 1: Clone the OpenNIC driver onto your machine: `git clone https://github.com/Xilinx/open-nic-driver`.

Step 1.5: Run `cd open-nic-driver` and `git checkout 2fa96685` (This is for Ubuntu 16.04. Remove it in the future)

Step 2: Run `make` in the open-nic-driver directory to compile the kernel `onic.ko`.

Step 3: Run `sudo insmod onic.ko RS_FEC_ENABLED=0` to insert the module.

Step 4: Run `dmesg` to verify the kernel module has been inserted.

Step 5: Run `sudo ifconfig enp59s0 192.168.1.10 netmask 255.255.255.0 up` to configure the network interface. You may wish to replace some parameters in the command to suit your own case. 

## (Option) Config DPDK-pktgen

This is for faster packet throughput. 

Step 1: run `./dpdk.sh`

Step 2: Refer to [Section 5](https://github.com/Xilinx/open-nic-dpdk) to configure the BIOS of the host. 

Step 3: Refer to [Section 8](https://github.com/Xilinx/open-nic-dpdk) to run pktgen. 


# Runtime 
## Testing

For testing you need two servers, one with the FPGA attached that you just programmed.  We refer to this one as server1.  The second server, which sends packets to server1, we refer to as server2.  

Once server1 is rebooted, set up a program to capture incoming packets on server1.  You can use wireshark or tcpdump.  For example,  `sudo tcpdump -i <ethernet-interface-name>`.


Use server2 to send packets to server1. You can use the command `tcpreplay` to send the provided pcap file frem server2 in `pcap/test1.pcap`: 

OCT FPGA nodes are equipped with a dual-port 40G Intel NIC. You can either use them to send the packets or setup the FPGA to send packets. 

To bring the Intel NIC up, you can run `sudo ifconfig enp134s0f0 192.168.1.20 netmask 255.255.255.0 up`. 

 `sudo tcpreplay --intf1=enp134s0f0 pcap/test1.pcap`
 

## Configuring P4 tables and parameters

Now the OpenNIC with P4 logic has been loaded to the FPGA. 

You can create/update/delete the tables of your P4 logic and use the driver that is in the install folder of the unpacked files you transferred to your local machine. 

IMPORTANT: Modify the C file `$(APP).main.c` in the folder to reconfigure tables. Make sure the variable `sysfile_path[]` for PCIe BAR2 path is correct for your FPGA.

run `lspci -t -vv | grep 903f` to find the pci bus address for the FPGA. After that, you will need to find system path for the base address register (BAR2). You can find this in `/sys/devices/pci<your own address>/resource2`.

After modifications, run `make clean && make` in the `install` folder to rebuild the executable. 

Run `sudo ./driver` to load the tables to the P4 logic on FPGA.

Note: The C file in the `install` folder may be overwritten if you rebuild the bitstream.  Save any changes before rebuilding the bitstream.  
