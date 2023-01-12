#! /bin/bash

if [ -z ${VIVADO_ROOT} ]; then 
echo VIVADO_ROOT is unset! Please set the VIVADO_ROOT >&2;
exit;
fi
source $VIVADO_ROOT/settings64.sh
cd script
vivado -mode batch -source build.tcl -tclargs -board au280
cd ..
cp ../src/open-nic-shell-p4-plugin.patch .
git apply open-nic-shell-p4-plugin.patch
cp ../src/p4/forward.p4 src/.
cp ../src/hw/p2p_forward_p4_250mhz.sv plugin/p2p/.
cp ../script/update.tcl .
vivado -mode batch -source update.tcl
