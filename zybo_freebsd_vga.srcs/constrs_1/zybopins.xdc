# Buttons
set_property PACKAGE_PIN R18 [get_ports {BTN[0]}]
set_property PACKAGE_PIN P16 [get_ports {BTN[1]}]
set_property PACKAGE_PIN V16 [get_ports {BTN[2]}]
set_property PACKAGE_PIN Y16 [get_ports {BTN[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {BTN[*]}]

# LEDs
set_property PACKAGE_PIN M14 [get_ports {LD[0]}]
set_property PACKAGE_PIN M15 [get_ports {LD[1]}]
set_property PACKAGE_PIN G14 [get_ports {LD[2]}]
set_property PACKAGE_PIN D18 [get_ports {LD[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {LD[*]}]

# Switches
set_property PACKAGE_PIN G15 [get_ports {SW[0]}]
set_property PACKAGE_PIN P15 [get_ports {SW[1]}]
set_property PACKAGE_PIN W13 [get_ports {SW[2]}]
set_property PACKAGE_PIN T16 [get_ports {SW[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {SW[*]}]

# SVGA output
set_property PACKAGE_PIN M19 [get_ports {RED_O[0]}]
set_property PACKAGE_PIN L20 [get_ports {RED_O[1]}]
set_property PACKAGE_PIN J20 [get_ports {RED_O[2]}]
set_property PACKAGE_PIN G20 [get_ports {RED_O[3]}]
set_property PACKAGE_PIN F19 [get_ports {RED_O[4]}]
set_property IOSTANDARD LVCMOS33 [get_ports {RED_O[*]}]
set_property SLEW FAST [get_ports {RED_O[*]}]

set_property PACKAGE_PIN H18 [get_ports {GREEN_O[0]}]
set_property PACKAGE_PIN N20 [get_ports {GREEN_O[1]}]
set_property PACKAGE_PIN L19 [get_ports {GREEN_O[2]}]
set_property PACKAGE_PIN J19 [get_ports {GREEN_O[3]}]
set_property PACKAGE_PIN H20 [get_ports {GREEN_O[4]}]
set_property PACKAGE_PIN F20 [get_ports {GREEN_O[5]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GREEN_O[*]}]
set_property SLEW FAST [get_ports {GREEN_O[*]}]

set_property PACKAGE_PIN P20 [get_ports {BLUE_O[0]}]
set_property PACKAGE_PIN M20 [get_ports {BLUE_O[1]}]
set_property PACKAGE_PIN K19 [get_ports {BLUE_O[2]}]
set_property PACKAGE_PIN J18 [get_ports {BLUE_O[3]}]
set_property PACKAGE_PIN G19 [get_ports {BLUE_O[4]}]
set_property IOSTANDARD LVCMOS33 [get_ports {BLUE_O[*]}]
set_property SLEW FAST [get_ports {BLUE_O[*]}]

set_property PACKAGE_PIN R19 [get_ports VSYNC_O]
set_property PACKAGE_PIN P19 [get_ports HSYNC_O]
set_property IOSTANDARD LVCMOS33 [get_ports VSYNC_O]
set_property IOSTANDARD LVCMOS33 [get_ports HSYNC_O]
set_property SLEW FAST [get_ports VSYNC_O]
set_property SLEW FAST [get_ports HSYNC_O]

set_property -dict {PACKAGE_PIN N18 IOSTANDARD LVCMOS33} [get_ports IIC_scl_io]
set_property -dict {PACKAGE_PIN N17 IOSTANDARD LVCMOS33} [get_ports IIC_sda_io]
