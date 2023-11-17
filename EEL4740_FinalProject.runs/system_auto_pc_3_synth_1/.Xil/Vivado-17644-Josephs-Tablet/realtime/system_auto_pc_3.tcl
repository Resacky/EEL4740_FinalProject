# 
# Synthesis run script generated by Vivado
# 

namespace eval rt {
    variable rc
}
set rt::rc [catch {
  uplevel #0 {
    set ::env(BUILTIN_SYNTH) true
    source $::env(HRT_TCL_PATH)/rtSynthPrep.tcl
    rt::HARTNDb_resetJobStats
    rt::HARTNDb_resetSystemStats
    rt::HARTNDb_startSystemStats
    rt::HARTNDb_startJobStats
    set rt::cmdEcho 0
    rt::set_parameter writeXmsg true
    rt::set_parameter enableParallelHelperSpawn true
    set ::env(RT_TMP) "C:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.runs/system_auto_pc_3_synth_1/.Xil/Vivado-17644-Josephs-Tablet/realtime/tmp"
    if { [ info exists ::env(RT_TMP) ] } {
      file delete -force $::env(RT_TMP)
      file mkdir $::env(RT_TMP)
    }

    rt::delete_design

    set rt::partid xc7z020clg400-1
    source $::env(HRT_TCL_PATH)/rtSynthParallelPrep.tcl

    set rt::multiChipSynthesisFlow false
    source $::env(SYNTH_COMMON)/common_vhdl.tcl
    set rt::defaultWorkLibName xil_defaultlib

    set rt::useElabCache false
    if {$rt::useElabCache == false} {
      rt::read_verilog -sv -include c:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.srcs/sources_1/bd/system/ipshared/ec67/hdl {
      C:/Xilinx/Vivado/2017.4/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv
      C:/Xilinx/Vivado/2017.4/data/ip/xpm/xpm_fifo/hdl/xpm_fifo.sv
      C:/Xilinx/Vivado/2017.4/data/ip/xpm/xpm_memory/hdl/xpm_memory.sv
    }
      rt::read_verilog -include c:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.srcs/sources_1/bd/system/ipshared/ec67/hdl {
      c:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.srcs/sources_1/bd/system/ipshared/b752/hdl/generic_baseblocks_v2_1_vl_rfs.v
      c:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.srcs/sources_1/bd/system/ipshared/9909/hdl/axi_data_fifo_v2_1_vl_rfs.v
      c:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.srcs/sources_1/bd/system/ipshared/ec67/hdl/axi_infrastructure_v1_1_vl_rfs.v
      c:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.srcs/sources_1/bd/system/ipshared/3ed1/hdl/axi_register_slice_v2_1_vl_rfs.v
      c:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.srcs/sources_1/bd/system/ipshared/ff69/hdl/axi_protocol_converter_v2_1_vl_rfs.v
      c:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.srcs/sources_1/bd/system/ip/system_auto_pc_3/synth/system_auto_pc_3.v
    }
      rt::read_vhdl -lib blk_mem_gen_v8_4_1 c:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.srcs/sources_1/bd/system/ipshared/67d8/hdl/blk_mem_gen_v8_4_vhsyn_rfs.vhd
      rt::read_vhdl -lib fifo_generator_v13_2_1 c:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.srcs/sources_1/bd/system/ipshared/5c35/hdl/fifo_generator_v13_2_vhsyn_rfs.vhd
      rt::read_vhdl -lib xpm C:/Xilinx/Vivado/2017.4/data/ip/xpm/xpm_VCOMP.vhd
      rt::filesetChecksum
    }
    rt::set_parameter usePostFindUniquification false
    set rt::top system_auto_pc_3
    set rt::reportTiming false
    rt::set_parameter elaborateOnly true
    rt::set_parameter elaborateRtl true
    rt::set_parameter eliminateRedundantBitOperator false
    rt::set_parameter writeBlackboxInterface true
    rt::set_parameter merge_flipflops true
    rt::set_parameter srlDepthThreshold 3
    rt::set_parameter rstSrlDepthThreshold 4
# MODE: 
    rt::set_parameter webTalkPath {}
    rt::set_parameter enableSplitFlowPath "C:/Users/ruiz-/EEL4740_FinalProject/EEL4740_FinalProject.runs/system_auto_pc_3_synth_1/.Xil/Vivado-17644-Josephs-Tablet/"
    set ok_to_delete_rt_tmp true 
    if { [rt::get_parameter parallelDebug] } { 
       set ok_to_delete_rt_tmp false 
    } 
    if {$rt::useElabCache == false} {
        set oldMIITMVal [rt::get_parameter maxInputIncreaseToMerge]; rt::set_parameter maxInputIncreaseToMerge 1000
        set oldCDPCRL [rt::get_parameter createDfgPartConstrRecurLimit]; rt::set_parameter createDfgPartConstrRecurLimit 1
      rt::run_rtlelab -module $rt::top
        rt::set_parameter maxInputIncreaseToMerge $oldMIITMVal
        rt::set_parameter createDfgPartConstrRecurLimit $oldCDPCRL
    }

    set rt::flowresult [ source $::env(SYNTH_COMMON)/flow.tcl ]
    rt::HARTNDb_stopJobStats
    if { $rt::flowresult == 1 } { return -code error }


    if { [ info exists ::env(RT_TMP) ] } {
      if { [info exists ok_to_delete_rt_tmp] && $ok_to_delete_rt_tmp } { 
        file delete -force $::env(RT_TMP)
      }
    }

    source $::env(HRT_TCL_PATH)/rtSynthCleanup.tcl
  } ; #end uplevel
} rt::result]

if { $rt::rc } {
  $rt::db resetHdlParse
  set hsKey [rt::get_parameter helper_shm_key] 
  if { $hsKey != "" && [info exists ::env(BUILTIN_SYNTH)] && [rt::get_parameter enableParallelHelperSpawn] && [info exists rt::doParallel] && $rt::doParallel} { 
     $rt::db killSynthHelper $hsKey
  } 
  source $::env(HRT_TCL_PATH)/rtSynthCleanup.tcl
  return -code "error" $rt::result
}