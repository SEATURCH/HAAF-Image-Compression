LIBRARY IEEE;
LIBRARY WORK;
USE IEEE.STD_LOGIC_1164.ALL;
--use ieee.std_logic_arith.all;
use ieee.numeric_std.all;
use work.Types.all;

ENTITY ImageCompression IS
    port (
	 --clock : in std_logic; -- A 50MHz clock
--	 CLOCK_50 : in std_logic;
--	 KEY: in std_logic_vector(3 downto 0);
--	 RESULT_HORI_BLOCK : out CUblock;
--	 RESULT_VERT_BLOCK : out CUblock;
--	 RESULT_DC_BLOCK : out CUblock;
--	 RESULT_PLANAR_BLOCK : out CUblock;
--	 RESULT_HORI_RESIDUAL : out CUblockResidual;
--	 RESULT_VERT_RESIDUAL : out CUblockResidual;
--	 RESULT_DC_RESIDUAL : out CUblockResidual;
--	 RESULT_PLANAR_RESIDUAL : out CUblockResidual;
--		count : out integer;
--	 h_output : out horizontal;
--	 v_output : out vertical;
--	 DC_output : out integer;
	 
	 clk: in std_logic;
    reset_n: in std_logic;
    addr: in std_logic_vector(10 downto 0);
    rd_en: in std_logic;
    wr_en: in std_logic;
    readdata: out std_logic_vector(31 downto 0);
    writedata: in std_logic_vector(31 downto 0)
    );
end ImageCompression;

architecture behaviour of ImageCompression is

component my_ram_32bits IS
	PORT
	(
		address		: IN STD_LOGIC_VECTOR (5 DOWNTO 0);
		clock		: IN STD_LOGIC;
		data		: IN STD_LOGIC_VECTOR (31 DOWNTO 0);
		wren		: IN STD_LOGIC;
		q		: OUT STD_LOGIC_VECTOR (31 DOWNTO 0)
	);
	END component;

	constant reference_buffer_size : integer := 33;
	signal wr_en_ram0, wr_en_ram1, wr_en_ram2, wr_en_ram3, wr_en_ram4 : std_logic;
	signal wr_data_ram1, wr_data_ram2, wr_data_ram3, wr_data_ram4 : std_logic_vector(31 downto 0);
	signal hori_ram_data, vert_ram_data, DC_ram_data, planar_ram_data : std_logic_vector(31 downto 0);
	signal inputPicture_ram_data,test : std_logic_vector(31 downto 0);
	signal currentAddress : std_logic_vector(7 downto 0);
	signal done, go : std_logic := '1';
	signal led_signal: std_logic_vector(17 downto 0);
begin

--	LED0 : leds
--		port map(
--			LEDR => led_signal);
			
	RAM0 : my_ram_32bits
		port map(
			address => addr(7 downto 2),
			clock => clk,
			data => writedata(31 downto 0),
			wren => wr_en_ram0,
			q => inputPicture_ram_data);
			
	RAM1 : my_ram_32bits
		port map(
			address => currentAddress(7 downto 2),
			clock => clk,
			data => wr_data_ram1,
			wren => wr_en_ram1,
			q => hori_ram_data);
			
	RAM2 : my_ram_32bits
		port map(
			address => currentAddress(7 downto 2),
			clock => clk,
			data => wr_data_ram2,
			wren => wr_en_ram2,
			q => vert_ram_data);
			
	RAM3 : my_ram_32bits
		port map(
			address => currentAddress(7 downto 2),
			clock => clk,
			data => wr_data_ram3,
			wren => wr_en_ram3,
			q => DC_ram_data);
			
	RAM4 : my_ram_32bits
		port map(
			address => currentAddress(7 downto 2),
			clock => clk,
			data => wr_data_ram4,
			wren => wr_en_ram4,
			q => planar_ram_data);
			
			
	--Main process for writing into input picture & reference buffers, and predicting values, writing into memory, and reading the predictions--
	process(clk, reset_n)
		variable ho: horizontal;
		variable ve: vertical;
		variable DC_average : integer;
		variable counter : integer range 0 to 64;
		variable j, tl: integer range 0 to 255;
		variable x,y : integer range 0 to 16;
		variable H, H_prime, V, V_prime, a, b: integer;
		--variable inputPicture, hori, vert, DC, planar: CUblock;
		--variable DC_residual, hori_residual, vert_residual, planar_residual: CUblockResidual;
		variable curr_state: states:= RESET;
		variable next_state: states;
		variable readThis : readThis;
		variable go_back_to_RW: std_logic;
		variable temp_currentAddress : integer;
	begin
		if(reset_n = '0') then
			next_state := RESET;
		elsif(rising_edge(clk)) then
			case curr_state is
				when RESET =>
					next_state := RW;
					done <= '0';
				when RW =>
					done <= '1';
					wr_en_ram1 <= '0';
					wr_en_ram2 <= '0';
					wr_en_ram3 <= '0';
					wr_en_ram4 <= '0';
					currentAddress <= addr(7 downto 0);
					if(to_integer(unsigned(addr)) = 1280 AND wr_en = '1') then --writedata has the topleft reference element
						tl := to_integer(unsigned(writedata(7 downto 0)));
					elsif(to_integer(unsigned(addr)) >= 1281 AND  to_integer(unsigned(addr)) <= 1296 AND wr_en = '1') then --writedata has the top elements
						ve(to_integer(unsigned(addr))-1281+3) := to_integer(unsigned(writedata(31 downto 24)));
						ve(to_integer(unsigned(addr))-1281+2) := to_integer(unsigned(writedata(23 downto 16)));
						ve(to_integer(unsigned(addr))-1281+1) := to_integer(unsigned(writedata(15 downto 8)));
						ve(to_integer(unsigned(addr))-1281) := to_integer(unsigned(writedata(7 downto 0)));
					elsif(to_integer(unsigned(addr)) >= 1297 AND to_integer(unsigned(addr)) <= 1312 AND wr_en = '1') then --writedata has the left elements
						ho(to_integer(unsigned(addr))-1297+3) := to_integer(unsigned(writedata(31 downto 24)));
						ho(to_integer(unsigned(addr))-1297+2) := to_integer(unsigned(writedata(23 downto 16)));
						ho(to_integer(unsigned(addr))-1297+1) := to_integer(unsigned(writedata(15 downto 8)));
						ho(to_integer(unsigned(addr))-1297) := to_integer(unsigned(writedata(7 downto 0)));
					elsif(to_integer(unsigned(addr)) = 1313 AND wr_en = '1') then --go bit sent, start the predictions
						go <= writedata(0);
						counter := 0;
						next_state := CHANGE_ADDR;
						end if;
				when CHANGE_ADDR => 
					--Changing the address being written to (increments of 4, since we write 32 bits at once)--
					currentAddress <= std_logic_vector(to_unsigned(4 * counter, currentAddress'length));
					done <= '0';
					next_state := WAIST_CYCLE;
				when WAIST_CYCLE =>  
					--Setup to write to RAM, as well as calculating values needed for prediction--
					wr_en_ram1 <= '1';
					wr_en_ram2 <= '1';
					wr_en_ram3 <= '1';
					wr_en_ram4 <= '1';
					
					--DC Variables--
					DC_average := tl;
					for i in 0 to 15 loop
						DC_average := DC_average + ho(i) + ve(i);
					end loop;
					DC_average := DC_average/reference_buffer_size;
					
					--Planar variables--
					H_prime := (ve(8)-ve(6)) + 2*(ve(9)-ve(5)) + 3*(ve(10)-ve(4)) + 4*(ve(11)-ve(3)) + 5*(ve(12)-ve(2)) + 6*(ve(13)-ve(1)) + 7*(ve(14)-ve(0)) + 8*(ve(15)-tl);
					V_prime := (ho(8)-ho(6)) + 2*(ho(9)-ho(5)) + 3*(ho(10)-ho(4)) + 4*(ho(11)-ho(3)) + 5*(ho(12)-ho(2)) + 6*(ho(13)-ho(1)) + 7*(ho(14)-ho(0)) + 8*(ho(15)-tl);
					H := (5*H_prime + 32) / 64;
					V := (5*V_prime + 32) / 64;
					a := 16*(ho(15)+ve(15)+1) - 7*(V+H);
					next_state := PREDICTION;
				when PREDICTION =>
					--HORIZONTAL PREDICTION--
					temp_currentAddress := to_integer(unsigned(currentAddress));
					x := temp_currentAddress mod 16;
					y := temp_currentAddress / 16;
					wr_data_ram1(7 downto 0)   <= std_logic_vector(to_unsigned(ho(y), 8));
					wr_data_ram1(15 downto 8)  <= std_logic_vector(to_unsigned(ho(y), 8));
					wr_data_ram1(23 downto 16) <= std_logic_vector(to_unsigned(ho(y), 8));
					wr_data_ram1(31 downto 24) <= std_logic_vector(to_unsigned(ho(y), 8));
--					hori(temp_currentAddress) := ho(y);
--					hori(temp_currentAddress+1) := ho(y);
--					hori(temp_currentAddress+2) := ho(y);
--					hori(temp_currentAddress+3) := ho(y);
						
					--VERTICAL PREDICTION--
					wr_data_ram2(7 downto 0) <= std_logic_vector(to_unsigned(ve  (x), 8));
					wr_data_ram2(15 downto 8) <= std_logic_vector(to_unsigned(ve (x+1), 8));
					wr_data_ram2(23 downto 16) <= std_logic_vector(to_unsigned(ve(x+2), 8));
					wr_data_ram2(31 downto 24) <= std_logic_vector(to_unsigned(ve(x+3), 8));
--					vert(temp_currentAddress) :=   ve(x);
--					vert(temp_currentAddress+1) := ve(x+1);
--					vert(temp_currentAddress+2) := ve(x+2);
--					vert(temp_currentAddress+3) := ve(x+3);
					
					
					--DC PREDICTION--
					wr_data_ram3(7 downto 0) <= std_logic_vector(to_unsigned(DC_average, 8));
					wr_data_ram3(15 downto 8) <= std_logic_vector(to_unsigned(DC_average, 8));
					wr_data_ram3(23 downto 16) <= std_logic_vector(to_unsigned(DC_average, 8));
					wr_data_ram3(31 downto 24) <= std_logic_vector(to_unsigned(DC_average, 8));
--					DC(temp_currentAddress) :=   DC_average;
--					DC(temp_currentAddress+1) := DC_average;
--					DC(temp_currentAddress+2) := DC_average;
--					DC(temp_currentAddress+3) := DC_average;
					
--					--PLANAR PREDICTION--
					b := a + V*y + H*x;
					b := b/32;
					if(b > 255) then
						b:= 255;
					elsif (b < 0) then
						b:= 0;
					end if;
					wr_data_ram4(7 downto 0) <= std_logic_vector(to_unsigned(b,8));
					--planar(x+(y*16)) := b;
					
					b := a + V*y + H*(x+1);
					b := b/32;
					if(b > 255) then
						b:= 255;
					elsif (b < 0) then
						b:= 0;
					end if;
					wr_data_ram4(15 downto 8) <= std_logic_vector(to_unsigned(b,8));
					--planar(x+(y*16)+1) := b;
					
					b := a + V*y + H*(x+2);
					b := b/32;
					if(b > 255) then
						b:= 255;
					elsif (b < 0) then
						b:= 0;
					end if;
					wr_data_ram4(23 downto 16) <= std_logic_vector(to_unsigned(b,8));
					--planar(x+(y*16)+2) := b;
					
					b := a + V*y + H*(x+3);
					b := b/32;
					if(b > 255) then
						b:= 255;
					elsif (b < 0) then
						b:= 0;
					end if;
					wr_data_ram4(31 downto 24) <= std_logic_vector(to_unsigned(b,8));
					--planar(x+(y*16)+3) := b;
					

					--Loop counter, we need to loop 64 times--
					--count <= counter;
					if (counter = 63) then
						done <= '1';
						next_state := RW;
					else
						counter := counter + 1;
						next_state := CHANGE_ADDR;
					end if;
			end case;
			curr_state := next_state;
			--RESULT_HORI_BLOCK <= hori;
			--RESULT_VERT_BLOCK <= vert;
			--RESULT_DC_BLOCK <= DC;
			--RESULT_PLANAR_BLOCK <= planar;
		end if;
  end process;
  
  process(rd_en, addr)
  begin
			if(to_integer(unsigned(addr)) >= 256 AND to_integer(unsigned(addr)) <= 511 AND rd_en = '1') then 				--trying to read hori pred
				readdata <= hori_ram_data; 	
			elsif(to_integer(unsigned(addr)) >= 512 AND to_integer(unsigned(addr)) <= 767 AND rd_en = '1') then 			--trying to read vert pred
				readdata <= vert_ram_data;
			elsif(to_integer(unsigned(addr)) >= 768 AND to_integer(unsigned(addr)) <= 1023 AND rd_en = '1') then 			--trying to read DC pred
				readdata <= DC_ram_data;
			elsif(to_integer(unsigned(addr)) >= 1024 AND to_integer(unsigned(addr)) <= 1279 AND rd_en = '1') then 		--trying to read planar pred
				readdata <= planar_ram_data;
			elsif(to_integer(unsigned(addr)) = 1314 AND rd_en = '1') then 																--trying to read 'done' bit
				readdata(0) <= done; --(0 => '1', others => '0');
			end if;
  end process;
  
  process(wr_en,addr)
  begin
		wr_en_ram0 <= '0';
		if(to_integer(unsigned(addr)) >= 0 AND to_integer(unsigned(addr)) <= 255 AND wr_en = '1') then --writing input picture
			wr_en_ram0 <= '1';
		end if;
  end process;
end behaviour;