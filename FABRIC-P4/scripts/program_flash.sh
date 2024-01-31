#!/bin/bash

# Tested with Supermicro server.

echo $#
if [[ $# -le 2 ]] || [[ -z EXTENDED_DEVICE_BDF1 ]]; then
    echo "Usage: EXTENDED_DEVICE_BDF1=<EXTENDED_DEVICE_BDF1> ./program_fpga.sh BITSTREAM_PATH BOARD jtag_id "
    echo "Please export EXTENDED_DEVICE_BDF1 and [EXTENDED_DEVICE_BDF2 (if needed for 2 port boards)]"
    echo "Please ensure vivado is loaded into system path."
    exit 1
fi

set -Eeuo pipefail
set -x

mcsfile_path=$1
board=$2
jtag_id=$3

echo "Make sure that the netdev iface for open nic shell is down before running this script!"
echo "Example: sudo ifconfig enp134s0f0 down."
echo "The interface may be up if a prior open nic shell design is loaded on the FPGA."

## Remove
echo 1 | sudo tee "/sys/bus/pci/devices/${EXTENDED_DEVICE_BDF1}/remove" > /dev/null
if [[ -n "${EXTENDED_DEVICE_BDF2:-}" ]] && [[ -e "/sys/bus/pci/devices/${EXTENDED_DEVICE_BDF2}" ]]; then
    echo 1 | sudo tee "/sys/bus/pci/devices/${EXTENDED_DEVICE_BDF2}/remove" > /dev/null
fi

onic_found=$((lsmod | grep onic) || echo "not found")
if [[ ${onic_found} == "not found" ]]; then
    echo "onic module not loaded"
else
    sudo rmmod onic.ko
fi

# Program fpga
vivado_lab -mode tcl -source ./program_flash.tcl \
    -tclargs -board $board \
    -mcsfile_path $mcsfile_path\
	-jtag_id 		$jtag_id

# Rescan
echo 1 | sudo tee "/sys/bus/pci/rescan" > /dev/null
sudo setpci -s $EXTENDED_DEVICE_BDF1 COMMAND=0x02
if [[ -n "${EXTENDED_DEVICE_BDF2:-}" ]]; then
    sudo setpci -s $EXTENDED_DEVICE_BDF2 COMMAND=0x02
fi

echo "program_flash.sh completed"
echo "Cold Reboot the machine if the BAR size is changed"
