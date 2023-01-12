#!/bin/bash
source $VIVADO_ROOT/settings64.sh
rm -rf gen
mkdir gen
p4c-vitisnet ../src/p4/calculator.p4 -o gen/calc.json
run-p4bm-vitisnet -j gen/calc.json -s src/cli_commands.txt
python ../../../utility/convertPcap.py src/calc_in.user gen/behav_calc_in.pcap
python ../../../utility/convertPcap.py src/calc_out.user gen/behav_calc_out.pcap
rm -rf src/calc_out.user
rm -rf src/calc_out.meta
