set root_dir [file normalize .]
set build_dir $root_dir/build/au280/open_nic_shell
set src_dir $root_dir/src
set plugin_dir $root_dir/plugin
set prj_name open_nic_shell.xpr
open_project $build_dir/$prj_name
update_compile_order -fileset sources_1
create_ip -name vitis_net_p4 -vendor xilinx.com -library ip -version 1.0 -module_name vitis_net_p4_0
set_property -dict [list CONFIG.P4_FILE "$src_dir/calculator.p4" CONFIG.JSON_TIMESTAMP {1661464797}] [get_ips vitis_net_p4_0]
generate_target all [get_files $build_dir/open_nic_shell.srcs/sources_1/ip/vitis_net_p4_0/vitis_net_p4_0.xci]

update_compile_order -fileset sources_1
add_files -norecurse $plugin_dir/p2p/p2p_calc_p4_250mhz.sv
update_compile_order -fileset sources_1
set_property -dict [list CONFIG.M01_A00_BASE_ADDR {0x0000000000002000} CONFIG.M00_A00_ADDR_WIDTH {13}] [get_ips box_250mhz_axi_crossbar]
set_property strategy Performance_Explore [get_runs impl_1]
reset_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs 16
wait_on_run impl_1
set impl_status [get_property STATUS [get_runs impl_1]]
set impl_progress [get_property PROGRESS [get_runs impl_1]]
if {$impl_status != "write_bitstream Complete!" || $impl_progress != "100%"} {
    puts "Synthesis failed."
    exit 1
}
set interface SPIx4
set start_address 0x01002000
write_cfgmem  -format mcs -size 128 -interface $interface -loadbit "up $start_address $build_dir/open_nic_shell.runs/impl_1/open_nic_shell.bit" -file "$root_dir/calc.mcs"
close_project
