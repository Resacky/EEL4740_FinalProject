# EEL4740_FinalProject
 This is a streamlined version of my FPGA Final Project solution, The embedded system model I am using is the Zybo Z7-20 which is using VHDL and open block design for the implementation, the code is using C and it is programmed using the Vivado SDK.
 
 To initialize and start the project you will want to run the projects '.xpr' Vivado project solution file, which will launch Vivado and start the project solution.
 Once launched the open block design should be auto configured to read within the project solution, the 'lab4_zybo.xdc' constraint file and the led_ip custom IP.
 If you by any means need to generate a new wrapper for the project you can simply do so by running the synthesis, implementation and the bitstream.
 Afterwhich you can then click on file, then export, export hardware...
 
 Once the hardware is exported you can create a new application project and use the wrapper that you have exported to use the FPGA's configurations.
 The code provided within the SDK is already configured to use the current housed wrapper with all the peripherals configured.

The resources we used were:

https://digilent.com/reference/programmable-logic/zybo-z7/reference-manual

https://docs.xilinx.com/v/u/2017.1-English/ug904-vivado-implementation

https://forum.digilent.com/topic/9222-pmod-ssd/
