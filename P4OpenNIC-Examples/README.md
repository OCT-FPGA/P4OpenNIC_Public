# P4 Examples on OCT
# BUILD
## Prerequisite
- Before your build below,  be sure to follow the prerequisite instructions in the P4 folder.  In particular:
- 
- set the variable `VIVADO_ROOT` to  point to the installation path for your Vivado. For example:

`export VIVADO_ROOT=/tools/Xilinx/Vivado/2021.2`

- Make sure you have the license for the VITISNETP4. This can be checked by running `vlm` or the command `$VIVADO_ROOT/bin/unwrapped/lnx64.o/lmutil lmdiag`.  

- you should have cloned this directory into P4 on your local instance.  

These instructions assume you are in the directory P4/P4OpenNIC-Examples/

## APP List

There are multiple examples in this folder. The `$(APP)` in the below instructions should be replaced by any name in the following list.
- forwarder
- calc
- adv\_calc

## Build bitstreams and driver

Use `cd Example.$(APP)`  cd to the example folder. The P4 logic for the example is located in `src/p4/$(APP).p4`.

Run `make` to build the hardware bitstream and corresponding software driver.

The build will take up to about 12 hours on NERC. After the build is done, the memory configuration file is `$(APP)/$(APP).mcs` folder and the driver for configuring the tables is `$(APP).drivers/install/driver`.  

-------------------------------------------------------------
## DEPLOYMENT on OCT.  

## Pack
After build finishes, if you need to deploy it on another machine, run `make pack` to pack up the necessary files.

Copy $(APP).tar.gz to your deployment machine. It will contain a mcs file as bitstream for configuring FPGA, an `install` folder as driver for loading tables, `util` folder with useful scripts and tools and other files for testing.

## Config the FPGA and host

Step 0: Vivado or Vivado\_Lab must be installed.  You do not need any license for Vivado\_Lab.

Before you flash the shell, you should disable PCIe fatal error reporting by running a script. This is done in order to avoid kernel panic, and a subsequent server reboot caused by the iDRAC. Download the script [here](https://alexforencich.com/wiki/en/pcie/disable-fatal). To disable PCIe error reporting, you should run

`sudo ./pcie_disable_fatal.sh 0000:3b:00.0`

Step 1: Set up the enviorments for Vivado by: `source /tools/Xilinx/Vitis/2021.2/settings64.sh` 

Step 2: 
User can use either JTAG programming or PCIe programming.

For PCIe programming (on OCT nodes): we utilize Xilinx xbflash2 tool to program the FPGA. 
`sudo xbflash2 program --spi --image <path to the mcs> -d 3b:00.0 --bar 2`

For JTAG programming: User can choose to use the `util/program_config_mem.tcl` tcl file to program their FPGA by running:

`vivado -mode batch -source <path to the tcl/program_config_mem.tcl> -tclargs <path to the mcs>`.

If the tcl script is not working properly on your own machine, you can use Vivado GUI for configuring the FPGA.

Step 3: Reboot the host machine to install the bitstream.

Next,  you need to build the OpenNIC kernel module and load it.

Step 1: Clone the OpenNIC driver onto your machine: `git clone https://github.com/Xilinx/open-nic-driver`.

Step 2: Run `make` in the open-nic-driver directory to compile the kernel `onic.ko`.

Step 3: Run `sudo insmod onic.ko RS_FEC_ENABLED=0` to insert the module.

Step 4: Run `dmesg` to verify the kernel module has been inserted.

Step 5: Run `sudo ifconfig enp59s0 192.168.1.10 netmask 255.255.255.0 up` to configure the network interface. You may wish to replace some parameters in the command to suit your own case. 

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
