set root_dir [file normalize .]
set build_dir $root_dir/build/au280/open_nic_shell
set src_dir $root_dir/src
set plugin_dir $root_dir/plugin
set prj_name open_nic_shell.xpr
open_project $build_dir/$prj_name
update_compile_order -fileset sources_1
create_ip -name vitis_net_p4 -vendor xilinx.com -library ip -version 1.0 -module_name vitis_net_p4_0
set_property -dict [list CONFIG.USER_META_DATA_WIDTH {48} CONFIG.P4_FILE "$src_dir/forward.p4" CONFIG.JSON_TIMESTAMP {1653445372} CONFIG.S_AXI_ADDR_WIDTH {14} CONFIG.M_AXI_HBM_DATA_WIDTH {256} CONFIG.M_AXI_HBM_ADDR_WIDTH {33} CONFIG.M_AXI_HBM_ID_WIDTH {6} CONFIG.M_AXI_HBM_PROTOCOL {0} CONFIG.CAM_MEM_CLK_ENABLE {1} CONFIG.USER_METADATA_ENABLES {metadata.tuser_dst {input true output true} metadata.tuser_src {input true output true} metadata.tuser_size {input true output true}} CONFIG.CAM_TABLE_PARAMS {MyProcessing.forwardIPv4 {ram_style GLOBAL clock CAM mode STCAM opt GLOBAL} MyProcessing.forwardIPv6 {ram_style GLOBAL clock CAM mode TCAM opt GLOBAL}} CONFIG.USER_META_FORMAT {metadata.tuser_dst {length 16 start 0 end 15} metadata.tuser_src {length 16 start 16 end 31} metadata.tuser_size {length 16 start 32 end 47}}] [get_ips vitis_net_p4_0]
generate_target all [get_files $build_dir/open_nic_shell.srcs/sources_1/ip/vitis_net_p4_0/vitis_net_p4_0.xci]

update_compile_order -fileset sources_1
add_files -norecurse $plugin_dir/p2p/p2p_forward_p4_250mhz.sv
update_compile_order -fileset sources_1
set_property -dict [list CONFIG.M01_A00_BASE_ADDR {0x0000000000004000} CONFIG.M00_A00_ADDR_WIDTH {14}] [get_ips box_250mhz_axi_crossbar]
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
write_cfgmem  -format mcs -size 128 -interface $interface -loadbit "up $start_address $build_dir/open_nic_shell.runs/impl_1/open_nic_shell.bit" -file "$root_dir/forwarder.mcs"
close_project
