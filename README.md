# Build and Test P4 on Open Cloud Testbed
## Introduction 
This repository is to build and run P4 codes with or without FPGA hardware. It builds on P4, [Xilinx OpenNIC](https://github.com/Xilinx/open-nic), and makes use of network attached FPGAs.

There are four pieces of this repository:

- **Running P4 on BMV2**   For running P4 examples on the build machine on BMV2 with NO FPGA hardware: `OCT-P4-BMV2`.  These use the standard P4 tutorials  (https://github.com/p4lang/tutorials)  and follow the P4 model (bmv2). 

- **Building P4 on FPGA**  You can use `P4Framework` to build your FPGA bitstream and run on the AMD FPGAs. This is the core of this project. It provides a skeleton for building an FPGA-based NIC that instantiates a P4 function on the OpenNIC shell. It also provides three examples where you can directly try to build and depoly on our testbed. It can also be tested on the FABRIC cloud FPGAs [link](https://learn.fabric-testbed.net/article-categories/programmable-networking/).

- **Running P4 on FPGA**   There are two instrcutions available for testing after you generating the bitstream through our `P4Framework`. To test it on the OCT FPGAs, refer to `OCT-P4-FPGA`. To test it on FABRIC FPGAs, refer to `FABRIC-P4-FPGA`.

- **The Xiinx OpenNIC framework**  If you are not familiar with working with FPGAs and generating and downloading bitstreams, follow the instructions for deploying and testing Xilinx's OpenNIC `OpenNIC-Getting-Started`.  IMPORTANT:  These instructions are for deploying OpenNIC on FPGAs at Northeastern's Reconfigurable Computing Laboratory.  You will need your own FPGA hardware to run through this tutorial.  `OpenNIC-Getting-Started`

------

## Prerequisite 

If you are not familiar with the build workflow, follow the directions [here](https://github.com/OCT-FPGA/OCT-Tutorials/blob/master/cloudlab-setup/build-machines.md) to set up a build machine.

- Clone this repo by `git clone https://github.com/OCT-FPGA/P4OpenNIC_Public.git` on your build machine. 

For FPGA bitstream build:

- set the Xilinx Vivado installation path to 'VIVADO_ROOT'. On the build machine:  `export VIVADO_ROOT=/share/Xilinx/Vivado/2021.2`

## Prodcut licensing

To build OpenNIC, CMAC license is required. For deploying P4 codes onto FPGAs, certain licenses for VitisNetP4 are also required. All those licenses have already been setup.

To set the license path on the build machine: 

`export XILINXD_LICENSE_FILE=2100@xilinxlm`
