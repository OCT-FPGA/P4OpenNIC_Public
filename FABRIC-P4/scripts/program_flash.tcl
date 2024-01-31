# Directory variables
set script_path [file normalize [info script]]
set script_dir [file dirname $script_path]
set root_dir [file dirname $script_dir]

# Loading options
#   mcsfile_path Path to the mcs file 
#   board            Board name
array set options {
    -mcsfile_path "../build/au280/open_nic_shell/open_nic_shell.runs/impl_1/open_nic_shell.mcs"
    -probes_path    ""
    -board          au50
    -jtag_id        ""
}

# Expect arguments in the form of `-argument value`
for {set i 0} {$i < $argc} {incr i 2} {
    set arg [lindex $argv $i]
    set val [lindex $argv [expr $i+1]]
    if {[info exists options($arg)]} {
        set options($arg) $val
        puts "Set option $arg to $val"
    } else {
        puts "Skip unknown argument $arg and its value $val"
    }
}

# Settings based on defaults or passed in values
foreach {key value} [array get options] {
    set [string range $key 1 end] $value
}

#source ${script_dir}/board_settings/${board}.tcl

puts "Program file: $options(-mcsfile_path)"
puts "Probes file: $options(-probes_path)"
puts "Board: $options(-board)"
puts "jtag_id: $options(-jtag_id)"
#puts "HW device: $hw_device"

open_hw_manager
connect_hw_server -allow_non_jtag

open_hw_target [get_hw_targets "*/xilinx_tcf/Xilinx/$jtag_id"]
#open_hw_target [get_hw_targets -verbose [format "*/xilinx_tcf/Xilinx/%s" [lindex $argv 3]]]
set hw_device [current_hw_device]
refresh_hw_device -update_hw_probes false [lindex [get_hw_devices $hw_device] 0]
create_hw_cfgmem -hw_device [get_hw_devices $hw_device] [lindex [get_cfgmem_parts {mt25qu01g-spi-x1_x2_x4}] 0]
set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
refresh_hw_device [lindex [get_hw_devices $hw_device] 0]
set_property PROGRAM.ADDRESS_RANGE  {use_file} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
# Replace the following .mcs file with the one that you have.
#set_property PROGRAM.FILES [list "/home/zhhan/Workspace/new.mcs" ] [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.FILES ${options(-mcsfile_path)} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.PRM_FILE {} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.UNUSED_PIN_TERMINATION {pull-none} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]
startgroup 
create_hw_bitstream -hw_device [lindex [get_hw_devices $hw_device] 0] [get_property PROGRAM.HW_CFGMEM_BITFILE [ lindex [get_hw_devices $hw_device] 0]]; program_hw_devices [lindex [get_hw_devices $hw_device] 0]; refresh_hw_device [lindex [get_hw_devices $hw_device] 0];
program_hw_cfgmem -hw_cfgmem [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices $hw_device] 0]]

exit
