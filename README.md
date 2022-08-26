# P4 Examples on OCT
## Introduction 
This repository is to build and run P4 codes with or without FPGA hardware. It builds on P4, [Xilinx OpenNIC](https://github.com/Xilinx/open-nic), and makes use of network attached FPGAs.  

There are three pieces of this repository:

- Standard P4 tutorials.   For running P4 examples on the NERC with NO FPGA hardware: `OCT-P4-tutorials`.  These use the standard P4 tutorials  (https://github.com/p4lang/tutorials)  and follow the P4 model (bmv2).  

- The Xiinx OpenNIC framework.  If you are not familiar with working with FPGAs and generating and downloading bitstreams, follow the instructions for deploying and testing Xilinx's OpenNIC `OpenNIC-Getting-Started`.  IMPORTANT:  These instructions are for deploying OpenNIC on FPGAs at Northeastern's Reconfigurable Computing Laboratory.  You will need your own FPGA hardware to run through this tutorial.  `OpenNIC-Getting-Started`

- Instructions for building and deploying P4 logic with OpenNIC.  This has two parts.  For building a bitstream you can access the tools through the NERC.  P4 logic is compiled into the OpenNIC framework using the Xilinx VitisNetP4 compiler. This may take some time.  Once you have built a bitstream you need to transfer the files to deploy the example on a platform that has one or more FPGAs and supports programming of the bitstream through JTAG.  `P4OpenNIC-Examples` 

------

## Prerequisite 

If you are not familiar with NERC follow these directions to create an account and setup an instance from [here](https://docs.google.com/document/d/1_JZ1K0lDdCTKP6TePhMbEBIyySO4jYZbF9-yBIQO07A/edit).

- Clone this repo by `git clone https://github.com/OCT-FPGA/P4` on your NERC instance.  

For FPGA bitstream build:

- set the Xilinx Vivado installation path to 'VIVADO_ROOT'. On NERC:  `export VIVADO_ROOT=/tools/Xilinx/Vivado/2021.2`

## Prodcut licensing

To build OpenNIC, CMAC license is required. For deploying P4 codes onto FPGAs, certain licenses for VitisNetP4 are also required. All those licenses have already been setup on NERC.

To set the license path on NERC:  

`export XILINXD_LICENSE_FILE=2100@192.168.0.54`

You can run `vlm` to check that you have these licenses available. 

Important: if vlm is not found, run `source $VIVADO_ROOT/settings64.sh`, then try again.

