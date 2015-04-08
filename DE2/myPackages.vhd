LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

Package Types is
	type states is (RESET, RW, CHANGE_ADDR, WAIST_CYCLE, PREDICTION, SKIP_ONE_CLOCK, READ_MEMORY);
	type readThis is (hori_s, vert_s, DC_s, planar_s, none_s);
	type CUblock is array (0 to 255) of integer range 0 to 255;
	type CUblockResidual is array (0 to 255) of integer range -255 to 255;
	type horizontal is array (0 to 15) of integer range 0 to 255;
	type vertical is array (0 to 15) of integer range 0 to 255;
	--type test is array(0 to 10) of bit;
End package Types;

