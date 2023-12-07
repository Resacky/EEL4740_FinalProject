# EEL4740_FinalProject
This report presents a refined version of my final project for the FPGA course. The project employs the Zybo Z7-20 embedded system model, utilizing VHDL and an open block design approach for its development. The programming is carried out in C language, facilitated by the Vivado SDK environment.

To initiate the project, the Vivado project solution file (with a '.xpr' extension) should be executed. This action will activate Vivado and load the project. Once in the Vivado environment, the open block design is already preconfigured with the scope of the project, incorporating the 'lab4_zybo.xdc' constraint file and the custom IP for LED, named 'led_ip'. In cases where a new project wrapper needs to be generated, it can be accomplished by performing synthesis, implementation, and bitstream processes. Subsequently, by navigating to 'File', then 'Export', and selecting 'Export Hardware', the process is completed.

Following the hardware export, a new application project can be created, utilizing the newly exported wrapper to apply the FPGA configurations. The code available in the SDK is pre-configured to interact with the current wrapper, ensuring all peripherals are appropriately integrated.

The resources we used were:

https://digilent.com/reference/programmable-logic/zybo-z7/reference-manual

https://docs.xilinx.com/v/u/2017.1-English/ug904-vivado-implementation

https://forum.digilent.com/topic/9222-pmod-ssd/

Video link demo:

https://www.youtube.com/watch?v=Uuk8ZhgSsSQ&ab_channel=AnthonyMartin
