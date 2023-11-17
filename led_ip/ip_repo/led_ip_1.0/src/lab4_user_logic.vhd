library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity lab4_user_logic is
	generic ( LED_WIDTH : integer :=8);
	port (
		S_AXI_ACLK : in std_logic;
		slv_reg_wren : in std_logic;
		S_AXI_AWADDR : in std_logic_vector(1 downto 0);
		S_AXI_WDATA : in std_logic_vector(31 downto 0);
		S_AXI_ARESETN : in std_logic;
		LED : out std_logic_vector(LED_WIDTH-1 downto 0)
		);
end lab4_user_logic;

architecture arch_imp of lab4_user_logic is
      
begin
	process (S_AXI_ACLK)
	begin
	   if S_AXI_ACLK'event and S_AXI_ACLK='1' then
		  if S_AXI_ARESETN='0' then
			 LED <= (others => '0');
		  elsif (slv_reg_wren ='1' and S_AXI_AWADDR = "00") then
			 LED <= S_AXI_WDATA(LED_WIDTH-1 downto 0);
		  end if;
	   end if;
	end process;
end arch_imp;
