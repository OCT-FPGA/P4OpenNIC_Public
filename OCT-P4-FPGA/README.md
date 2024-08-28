# P4 Examples on OCT
# BUILD
For build, refer to instructions under P4Framework

-------------------------------------------------------------
## DEPLOYMENT on OCT.  

## Request P4-nodes from Cloudlab
Request nodes from Cloudlab. For the P4Framework examples, you will need to request at least two nodes. One is used to deploy P4 apps. The other is to send the packets. Note: The second node is not necessarily an FPGA node. However, only FPGA node in OCT provides 100G link.

## Copy necessary files to the deployment machines
Copy the `$(APP).dist` to the FPGA node for the deployment. Copy the test pcap files to the other node. We will send these pcap packets to the P4-FPGA node.


## Config the FPGA and host
### Load your bitstream
1. Make sure that your FPGA is in factory mode (golden image). 
    ```
    source /opt/xilinx/xrt/setup.sh
    xbmgmt examine
    ```
    It should show something like with the golden image `xilinx_u280_GOLDEN_8`: 
    ```
    Devices present
    BDF             :  Shell                 Platform UUID  Device ID  Device Ready*
    ----------------------------------------------------------------------------------
    [0000:3b:00.0]  :  xilinx_u280_GOLDEN_8  n/a            n/a        No
    ```
    
    If not, use the `config-fpga reset private_key.pem` to reset the FPGA to golden image.

    The `private_key.pem` file is the private key used to access the head node that performs the reset. You can generate the public-private key pair by running the following commands:

    ```
    openssl genpkey -algorithm RSA -out private_key.pem -pkeyopt rsa_keygen_bits:2048
    ```
    ```
    openssl rsa -pubout -in private_key.pem -out public_key.pem
    ``` 
    After generating the keypair, share your public key with us (email your public key to s.handagala@northeastern.edu). This will allow you to use your private key to run `config-fpga`.

   It may take a few minutes to finish. After finish, it will show something like: 
    ```
    ➜  zhhan@pc164 ~ config-fpga reset private_key.pem
    Trying to reset the FPGA...
    Successfully flashed the FPGA with golden image.
    Trying to reset the PCIe bus...
    PCIe device 3a:00.0 has been reset.
    ```
    Use the `xbmgmt examine` to double check again. 
    
1. User can use either JTAG programming or PCIe programming. On OCT, we use PCIe programming for security issues.

    For PCIe programming (on OCT nodes): we utilize Xilinx xbflash2 tool to program the FPGA. 
    `sudo xbflash2 program --spi --image <path to the mcs> -d 3b:00.0 --bar 2`

    The output is like: 
    ```
    ➜  zhhan@pc164 ~ sudo xbflash2 program --spi --image bits/opennic_0x02150250.mcs -d 3b:00.0 --bar 2
    Preparing to program flash on device: 3b:00.0
    Are you sure you wish to proceed? [Y/n]: Y
    Successfully opened /dev/xfpga/flash.m15104.0
    flashing via QSPI driver
    Bitstream guard installed on flash @0x1002000
    Extracting bitstream from MCS data:
    .......................................
    Extracted 40523332 bytes from bitstream @0x1002000
    Writing bitstream to flash 0:
    .......................................
    Bitstream guard removed from flash
    ****************************************************
    Cold reboot machine to load the new image on device.
    ****************************************************
    ```

1. Instead of cold reboot, we can boot the FPGA from configuration memory to install the bitstream. Run 
   ```
   config-fpga boot private_key.pem
   ```
   It should show:
   ```
   zhhan@pc164 ~ config-fpga boot private_key.pem
   Trying to boot the FPGA...
   Successfully pulled the FPGA configuration from flash.
   Trying to reset the PCIe bus...
   PCIe device 3a:00.0 has been reset.
   ```

### Configure hardware drivers
Next,  you need to build the OpenNIC kernel module and load it.

1. Copy the opennic-scripts folder to your local directory:

    ```
    cp -r /proj/octfpga-PG0/tools/deployment/opennic/opennic-scripts/ ~/.
    cd opennic-scripts/open-nic-driver/
    ```
1. Run `sudo make` in the open-nic-driver directory to compile the kernel `onic.ko`.

1. Run `sudo insmod onic.ko RS_FEC_ENABLED=0` to insert the module.

1. Run `dmesg` to verify the kernel module has been inserted.

1. Run `sudo ifconfig ens1f1 192.168.40.20 netmask 255.255.255.0 up` to configure the network interface. You may wish to replace some parameters in the command to suit your own case. 

1. (Optional) An extra 40G Intel NIC is provided as `ens7f0`. You may need to disable this NIC for FPGA-to-FPGA test.

## (Optional) Config DPDK-pktgen

This is for faster packet throughput. 

1. The DPDK is installed in the directory `/opt/dpdk`

1. Refer to [Section 5](https://github.com/Xilinx/open-nic-dpdk) to configure the BIOS of the host. 

1. Refer to [Section 8](https://github.com/Xilinx/open-nic-dpdk) to run pktgen. 

## Checking the status of the bitstream.

As the P4Framework is based on [OpenNIC shell](https://github.com/Xilinx/open-nic), it provides a few registers that can be accessed through the PCIe Base Address Register (BAR). 

The detailed layout can be found in the opennic shell [manual](https://github.com/Xilinx/open-nic/blob/main/OpenNIC_manual.pdf). 

For example, as the manual indicates, the timestamp of the bitstream is located at 0x0000. Then you can use the [pcimem](https://github.com/billfarrow/pcimem) to access such address from the host machine. It will reflect the timestamp of the current loaded opennic shell design. In P4Framework, we auto-generate the timestamp based on the real time. The command of accessing such timestamp is:

```
sudo ./pcimem $pciBARAddr/resources2 0x0" 
``` 

The pcimem is located at `opennic-scripts/pcimem`.




# Runtime 
## Testing

For testing you need two servers, one with the FPGA attached that you just programmed.  We refer to this one as server1.  The second server, which sends packets to server1, we refer to as server2.

Once server1 is rebooted, set up a program to capture incoming packets on server1.  You can use wireshark or tcpdump.  For example,  `sudo tcpdump -i <ethernet-interface-name>`.


Use server2 to send packets to server1. You can use the command `tcpreplay` to send the provided pcap file frem server2 in `pcap/test1.pcap`: 

OCT FPGA nodes are equipped with a dual-port 40G Intel NIC. You can either use them to send the packets or setup the FPGA to send packets. 

To bring the Intel NIC up, you can run 
```
sudo ifconfig enp134s0f0 192.168.1.20 netmask 255.255.255.0 up
```

Run Tcpreplay to replay the trace on the network

```
sudo tcpreplay --intf1=enp134s0f0 pcap/test1.pcap
```
 

## Configuring P4 tables and parameters

Now the OpenNIC with P4 logic has been loaded to the FPGA. 

You can create/update/delete the tables of your P4 logic and use the driver that is in the install folder of the unpacked files you transferred to your local machine. 

IMPORTANT: Modify the C file `$(APP).main.c` in the folder to reconfigure tables. Make sure the variable `sysfile_path[]` for PCIe BAR2 path is correct for your FPGA.

run `lspci -t -vv | grep 903f` to find the pci bus address for the FPGA. After that, you will need to find system path for the base address register (BAR2). You can find this in `/sys/devices/pci<your own address>/resource2`.

After modifications, run `make clean && make` in the `install` folder to rebuild the executable. 

Run `sudo ./driver` to load the tables to the P4 logic on FPGA.

Note: The C file in the `install` folder may be overwritten if you rebuild the bitstream.  Save any changes before rebuilding the bitstream.
