# CommandLineHyperTerminal

 Command Line Hyper Terminal is a simple tool that allows user
 to communicate with COM ports from the Windows CLI interface.

 Usage:

		clht.exe COMn[:speed] [string | /f data.cfg]

 Exemples:

        clht.exe COM3:115200

  open the COM3 port, set its speed 115200bps and read data 
  from it;

        clht.exe COM3 AT

  open the COM3 port and write "AT" string to it;

        clht.exe COM3 ""

  write to the port the empty string - just the Carriage 
  Return (CR) symbol;

		cht.exe COM3 /f data.cfg

  write to the port data from specified file line by line.
  every line to be writed to port must be started with '>' 
  symbol. any other lines will be ignored but if a line is 
  started with '#' symbol it will be printed to console as 
  a comment.

