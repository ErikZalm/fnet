
# don't re-define FNET_STACK if it has been set elsewhere, e.g in Makefile
ifeq ($(FNET_STACK),)
	FNET_STACK = ../ext/fnet/fnet_stack
endif

FNETSRC += 	$(FNET_STACK)/cpu/fnet_cpu.c \
			$(FNET_STACK)/cpu/common/fnet_fec.c \
			$(FNET_STACK)/cpu/mcf/fnet_mcf.c \
			$(FNET_STACK)/cpu/mcf/fnet_mcf_cache.c \
			$(FNET_STACK)/cpu/mcf/fnet_mcf_eth.c \
			$(FNET_STACK)/cpu/mcf/fnet_mcf_flash.c \
			$(FNET_STACK)/cpu/mcf/fnet_mcf_isr_inst.c \
			$(FNET_STACK)/cpu/mcf/fnet_mcf_serial.c \
			$(FNET_STACK)/cpu/mcf/fnet_mcf_stdlib.c \
			$(FNET_STACK)/cpu/mcf/fnet_mcf_timer.c \
			$(FNET_STACK)/cpu/mk/fnet_mk.c \
			$(FNET_STACK)/cpu/mk/fnet_mk_cache.c \
			$(FNET_STACK)/cpu/mk/fnet_mk_eth.c \
			$(FNET_STACK)/cpu/mk/fnet_mk_flash.c \
			$(FNET_STACK)/cpu/mk/fnet_mk_isr.c \
			$(FNET_STACK)/cpu/mk/fnet_mk_isr_inst.c \
			$(FNET_STACK)/cpu/mk/fnet_mk_serial.c \
			$(FNET_STACK)/cpu/mk/fnet_mk_stdlib.c \
			$(FNET_STACK)/cpu/mk/fnet_mk_timer.c \
			$(FNET_STACK)/cpu/mpc/fnet_mpc.c \
			$(FNET_STACK)/cpu/mpc/fnet_mpc_cache.c \
			$(FNET_STACK)/cpu/mpc/fnet_mpc_eth.c \
			$(FNET_STACK)/cpu/mpc/fnet_mpc_isr.c \
			$(FNET_STACK)/cpu/mpc/fnet_mpc_isr_inst.c \
			$(FNET_STACK)/cpu/mpc/fnet_mpc_serial.c \
			$(FNET_STACK)/cpu/mpc/fnet_mpc_stdlib.c \
			$(FNET_STACK)/cpu/mpc/fnet_mpc_timer.c \
			$(FNET_STACK)/cpu/stm32/fnet_stm32_eth.c \
			$(FNET_STACK)/cpu/stm32/fnet_stm32_mac.c \
			$(FNET_STACK)/os/ChibiOS/fnet_chibios.c \
			$(FNET_STACK)/os/brtos/fnet_brtos.c \
			$(FNET_STACK)/os/freertos/fnet_freertos.c \
			$(FNET_STACK)/os/ucosIII/fnet_ucosIII.c \
			$(FNET_STACK)/services/dhcp/fnet_dhcp.c \
			$(FNET_STACK)/services/dns/fnet_dns.c \
			$(FNET_STACK)/services/flash/fnet_flash.c \
			$(FNET_STACK)/services/fs/fnet_fs.c \
			$(FNET_STACK)/services/fs/fnet_fs_rom.c \
			$(FNET_STACK)/services/fs/fnet_fs_root.c \
			$(FNET_STACK)/services/http/fnet_http.c \
			$(FNET_STACK)/services/http/fnet_http_auth.c \
			$(FNET_STACK)/services/http/fnet_http_cgi.c \
			$(FNET_STACK)/services/http/fnet_http_get.c \
			$(FNET_STACK)/services/http/fnet_http_post.c \
			$(FNET_STACK)/services/http/fnet_http_ssi.c \
			$(FNET_STACK)/services/ping/fnet_ping.c \
			$(FNET_STACK)/services/poll/fnet_poll.c \
			$(FNET_STACK)/services/serial/fnet_serial.c \
			$(FNET_STACK)/services/shell/fnet_shell.c \
			$(FNET_STACK)/services/telnet/fnet_telnet.c \
			$(FNET_STACK)/services/tftp/fnet_tftp_cln.c \
			$(FNET_STACK)/services/tftp/fnet_tftp_srv.c \
			$(FNET_STACK)/stack/fnet_arp.c \
			$(FNET_STACK)/stack/fnet_checksum.c \
			$(FNET_STACK)/stack/fnet_error.c \
			$(FNET_STACK)/stack/fnet_eth.c \
			$(FNET_STACK)/stack/fnet_icmp.c \
			$(FNET_STACK)/stack/fnet_icmp6.c \
			$(FNET_STACK)/stack/fnet_igmp.c \
			$(FNET_STACK)/stack/fnet_inet.c \
			$(FNET_STACK)/stack/fnet_ip.c \
			$(FNET_STACK)/stack/fnet_ip6.c \
			$(FNET_STACK)/stack/fnet_isr.c \
			$(FNET_STACK)/stack/fnet_loop.c \
			$(FNET_STACK)/stack/fnet_mempool.c \
			$(FNET_STACK)/stack/fnet_nd6.c \
			$(FNET_STACK)/stack/fnet_netbuf.c \
			$(FNET_STACK)/stack/fnet_netif.c \
			$(FNET_STACK)/stack/fnet_prot.c \
			$(FNET_STACK)/stack/fnet_raw.c \
			$(FNET_STACK)/stack/fnet_socket.c \
			$(FNET_STACK)/stack/fnet_stack.c \
			$(FNET_STACK)/stack/fnet_stdlib.c \
			$(FNET_STACK)/stack/fnet_tcp.c \
			$(FNET_STACK)/stack/fnet_timer.c \
			$(FNET_STACK)/stack/fnet_udp.c

 FNETINC += $(FNET_STACK) \
 			$(FNET_STACK)/compiler \
 			$(FNET_STACK)/cpu \
 			$(FNET_STACK)/cpu/common \
 			$(FNET_STACK)/cpu/mcf \
 			$(FNET_STACK)/cpu/mk \
 			$(FNET_STACK)/cpu/mpc \
 			$(FNET_STACK)/cpu/stm32 \
 			$(FNET_STACK)/os \
 			$(FNET_STACK)/os/ChibiOS \
 			$(FNET_STACK)/services \
 			$(FNET_STACK)/services/dhcp \
 			$(FNET_STACK)/services/dns \
 			$(FNET_STACK)/services/flash \
 			$(FNET_STACK)/services/fs \
 			$(FNET_STACK)/services/http \
 			$(FNET_STACK)/services/ping \
 			$(FNET_STACK)/services/poll \
 			$(FNET_STACK)/services/serial \
 			$(FNET_STACK)/services/shell \
 			$(FNET_STACK)/services/telnet \
 			$(FNET_STACK)/services/tftp \
 			$(FNET_STACK)/stack
 
