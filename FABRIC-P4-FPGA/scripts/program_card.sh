#!/bin/bash

# Tested with Supermicro server.

echo $#
if [[ $# -le 2 ]] || [[ -z EXTENDED_DEVICE_BDF1 ]] || [[ -z $XILINX_VIVADO ]]; then
    echo "Usage: EXTENDED_DEVICE_BDF1=<EXTENDED_DEVICE_BDF1> ./program_fpga.sh BITSTREAM_PATH BOARD jtag_id "
    echo "Please export EXTENDED_DEVICE_BDF1 and [EXTENDED_DEVICE_BDF2 (if needed for 2 port boards)]"
    echo "Example: EXTENDED_DEVICE_BDF1=0000:86:00.0 ./program_fpga.sh BITSTREAM_PATH BOARD jtag_id "
    echo "Please ensure vivado is loaded into system path."
    exit 1
fi

set -Eeuo pipefail
set -x

bridge_bdf=""
bitstream_path=$1
board=$2
jtag_id=$3

echo $EXTENDED_DEVICE_BDF1
echo $EXTENDED_DEVICE_BDF2

# Program fpga
vivado_lab -mode tcl -source /scripts/program_card.tcl \
    -tclargs -board $board \
    -bitstream_path $bitstream_path \
	-jtag_id 		$jtag_id


echo "program_fpga.sh completed"
echo "Warm reboot machine if the machine hasn't been warm reboooted after loading a open nic bitstream."
