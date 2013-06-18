FNET TCP/IP Stack.

Please read the FNET Quick Start chapter. You can find it in the /fnet_doc/fnet_user_manual.chm file.

FNET project directory structure:
	fnet                      - FNET root directory.
	|-- fnet_stack              - FNET TCP/IP stack.
	|    |-- stack                - FNET TCP/IP stack platform-independent source code.
	|    |-- services             - FNET Services.
	|    |    |-- dns               - DNS client/resolver service.
	|    |    |-- dhcp              - DHCP client service.
	|    |    |-- flash             - Flash  Memory driver.
	|    |    |-- fs                - File System driver.
	|    |    |-- http              - HTTP Server service.
	|    |    |-- poll              - Polling Mechanism library.
	|    |    |-- shell             - Command Shell service.
	|    |    |-- serial            - Serial Input/Output driver.
	|    |    |-- telnet            - Telnet server service.
	|    |    |-- tftp              - TFTP server and client services.
	|    |-- cpu                  - FNET platform-specific source code.
	|    |    |-- common            - Common platform-specific source code.
	|    |    |-- mcf               - Freescale ColdFire-specific source code.
	|    |    |-- mk                - Freescale Kinetis-specific source code.
	|    |-- compiler             - Compiler-specific source code.
	|    |-- os                   - OS-specific source code.
	|-- fnet_doc                  - FNET documentation.
	|-- fnet_demos                - FNET demo projects.
	|    |-- common                 - Common source code used by demos.
	|    |    |-- fnet_application    - Demo application source code.
	|    |    |-- fnet_webpage        - Demo web page source code.
	|    |    |-- startup             - Platform-specific startup source code.
	|    |-- <cpu_name>           - Demos for a <cpu_name> platform.
	|         |-- boot              - FNET TFTP Bootloader.
	|         |-- benchmark         - TCP/UDP Throughput Benchmark application. 
	|         |-- shell             - "Shell" demo is a fully featured shell. It allows
	|         |                       view/change various parameters (e.g. IP address,
	|         |                       subnet mask), explore mounted file systems, run 
	|         |                       DHCP client, HTTP and Telnet server.
	|         |-- shell_boot        - Example application used for the FNET TFTP Bootloader
	|                                 demonstration. This demo has the same features as 
	|                                 the "Shell" demo, but it`s modified to be able to
	|                                 work with the FNET Bootloader.
	|-- fnet_tools                - FNET tools.


