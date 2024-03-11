# Program FPGA Instructions

##  Install the Vivado_Lab tools (For FABRIC Nodes)
* Downlaod `Vivado 2023.1: Lab Edition - Linux` from AMD [webstie](https://www.xilinx.com/support/download.html) under `Vivado Lab Solutions - 2023.1` section. 
* Extract the download file:
  
    `tar -xf Xilinx_Vivado_Lab_Lin_2023.1_${VERSION}.tar.gz --directory /path/to/directory` 
* `cd Xilinx_Vivado_Lab_Lin_2023.1_${VERSION}`
* Run: 
  
  `sudo ./xsetup --agree 3rdPartyEULA,XilinxEULA --batch Install --edition "Vivado Lab Edition (Standalone)" --location /tools/Xilinx`

* Install necessary libs:
  `sudo apt install lsb` 

  `sudo ./installLibs.sh`

* Install JTAG drivers:
  
  `cd /tools/Xilinx/Vivado_Lab/2023.1/data/xicom/cable_drivers/lin64/install_script/install_drivers/`

  `sudo ./install_drivers`

* Reboot the slice to enable JTAG:
  
  `sudo reboot`


## Program FPGA PROM with mcs file

* `source /tools/Xilinx/Vivado_Lab/2023.1/settings64.sh`

* `cd scripts`

* Get the jtag\_id:

    `vivado_lab -mode batch -source get_jtag.tcl | grep -o 'Xilinx/[^[:space:]]*' | awk -F '/' '{print $2}'`

* Obtain the bdf:

  `lspci -d 10ee:`

* Run the following command to program the mcs file.

    `EXTENDED_DEVICE_BDF1=0000:${bdf} ./program_flash.sh mcs_file_name au280 ${jtag_id}`

    For example, if the `bdf` is `3b:00.0` and the `jtag_id` is `21770297401DA`, the command is: 

    `EXTENDED_DEVICE_BDF1=0000:3b:00.0 ./program_flash.sh open_nic_shell.mcs au280 21770297401DA`
## Checking the status of the bitstream.

As the P4Framework is based on [OpenNIC shell](https://github.com/Xilinx/open-nic), it provides a few registers that can be accessed through the PCIe Base Address Register (BAR). The detailed layout can be found in the opennic shell [manual](https://github.com/Xilinx/open-nic/blob/main/OpenNIC_manual.pdf). 
For example, as the manual indicates, the timestamp of the bitstream is located at 0x0000. Then you can use the [pcimem](https://github.com/billfarrow/pcimem) to access such address from the host machine. It will reflect the timestamp of the current loaded opennic shell design. In P4Framework, we auto-generate the timestamp based on the real time. The command of accessing such timestamp is:
    `sudo ./pcimem $pciBARAddr/resources2 0x0" 





