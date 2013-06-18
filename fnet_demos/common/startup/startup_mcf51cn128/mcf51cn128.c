/* Based on CPU DB MCF51CN128_80, version 3.00.022 (RegistersPrg V2.30) */
/* DataSheet : MCF51CN128RM Rev. 4 January 2009 */

#include <mcf51cn128.h>

/*lint -save -esym(765, *) */


/* * * * *  8-BIT REGISTERS  * * * * * * * * * * * * * * * */
/* NVFTRIM - macro for reading non volatile register       Nonvolatile MCG Fine Trim; 0x000003FE */
/* Tip for register initialization in the user code:  const byte NVFTRIM_INIT @0x000003FE = <NVFTRIM_INITVAL>; */
/* NVMCGTRM - macro for reading non volatile register      Nonvolatile MCG Trim Register; 0x000003FF */
/* Tip for register initialization in the user code:  const byte NVMCGTRM_INIT @0x000003FF = <NVMCGTRM_INITVAL>; */
/* NVBACKKEY0 - macro for reading non volatile register    Backdoor Comparison Key 0; 0x00000400 */
/* Tip for register initialization in the user code:  const byte NVBACKKEY0_INIT @0x00000400 = <NVBACKKEY0_INITVAL>; */
/* NVBACKKEY1 - macro for reading non volatile register    Backdoor Comparison Key 1; 0x00000401 */
/* Tip for register initialization in the user code:  const byte NVBACKKEY1_INIT @0x00000401 = <NVBACKKEY1_INITVAL>; */
/* NVBACKKEY2 - macro for reading non volatile register    Backdoor Comparison Key 2; 0x00000402 */
/* Tip for register initialization in the user code:  const byte NVBACKKEY2_INIT @0x00000402 = <NVBACKKEY2_INITVAL>; */
/* NVBACKKEY3 - macro for reading non volatile register    Backdoor Comparison Key 3; 0x00000403 */
/* Tip for register initialization in the user code:  const byte NVBACKKEY3_INIT @0x00000403 = <NVBACKKEY3_INITVAL>; */
/* NVBACKKEY4 - macro for reading non volatile register    Backdoor Comparison Key 4; 0x00000404 */
/* Tip for register initialization in the user code:  const byte NVBACKKEY4_INIT @0x00000404 = <NVBACKKEY4_INITVAL>; */
/* NVBACKKEY5 - macro for reading non volatile register    Backdoor Comparison Key 5; 0x00000405 */
/* Tip for register initialization in the user code:  const byte NVBACKKEY5_INIT @0x00000405 = <NVBACKKEY5_INITVAL>; */
/* NVBACKKEY6 - macro for reading non volatile register    Backdoor Comparison Key 6; 0x00000406 */
/* Tip for register initialization in the user code:  const byte NVBACKKEY6_INIT @0x00000406 = <NVBACKKEY6_INITVAL>; */
/* NVBACKKEY7 - macro for reading non volatile register    Backdoor Comparison Key 7; 0x00000407 */
/* Tip for register initialization in the user code:  const byte NVBACKKEY7_INIT @0x00000407 = <NVBACKKEY7_INITVAL>; */
/* NVPROT - macro for reading non volatile register        Nonvolatile Flash Protection Register; 0x0000040D */
/* Tip for register initialization in the user code:  const byte NVPROT_INIT @0x0000040D = <NVPROT_INITVAL>; */
/* NVOPT - macro for reading non volatile register         Nonvolatile Flash Options Register; 0x0000040F */
/* Tip for register initialization in the user code:  const byte NVOPT_INIT @0x0000040F = <NVOPT_INITVAL>; */
volatile PTADSTR _PTAD;                                    /* Port A Data Register; 0xFFFF8000 */
volatile PTADDSTR _PTADD;                                  /* Port A Data Direction Register; 0xFFFF8001 */
volatile PTAPESTR _PTAPE;                                  /* Port A Pull Enable Register; 0xFFFF8008 */
volatile PTASESTR _PTASE;                                  /* Port A Slew Rate Enable Register; 0xFFFF8009 */
volatile PTADSSTR _PTADS;                                  /* Port A Drive Strength Selection Register; 0xFFFF800A */
volatile PTAIFESTR _PTAIFE;                                /* Port A Input Filter Enable Register; 0xFFFF800B */
volatile PTBDSTR _PTBD;                                    /* Port B Data Register; 0xFFFF8010 */
volatile PTBDDSTR _PTBDD;                                  /* Port B Data Direction Register; 0xFFFF8011 */
volatile PTBPESTR _PTBPE;                                  /* Port B Pull Enable Register; 0xFFFF8018 */
volatile PTBSESTR _PTBSE;                                  /* Port B Slew Rate Enable Register; 0xFFFF8019 */
volatile PTBDSSTR _PTBDS;                                  /* Port B Drive Strength Selection Register; 0xFFFF801A */
volatile PTBIFESTR _PTBIFE;                                /* Port B Input Filter Enable Register; 0xFFFF801B */
volatile PTCDSTR _PTCD;                                    /* Port C Data Register; 0xFFFF8020 */
volatile PTCDDSTR _PTCDD;                                  /* Port C Data Direction Register; 0xFFFF8021 */
volatile PTCPESTR _PTCPE;                                  /* Port C Pull Enable Register; 0xFFFF8028 */
volatile PTCSESTR _PTCSE;                                  /* Port C Slew Rate Enable Register; 0xFFFF8029 */
volatile PTCDSSTR _PTCDS;                                  /* Port C Drive Strength Selection Register; 0xFFFF802A */
volatile PTCIFESTR _PTCIFE;                                /* Port C Input Filter Enable Register; 0xFFFF802B */
volatile PTDDSTR _PTDD;                                    /* Port D Data Register; 0xFFFF8030 */
volatile PTDDDSTR _PTDDD;                                  /* Port D Data Direction Register; 0xFFFF8031 */
volatile PTDPESTR _PTDPE;                                  /* Port D Pull Enable Register; 0xFFFF8038 */
volatile PTDSESTR _PTDSE;                                  /* Port D Slew Rate Enable Register; 0xFFFF8039 */
volatile PTDDSSTR _PTDDS;                                  /* Port D Drive Strength Selection Register; 0xFFFF803A */
volatile PTDIFESTR _PTDIFE;                                /* Port D Input Filter Enable Register; 0xFFFF803B */
volatile PTEDSTR _PTED;                                    /* Port E Data Register; 0xFFFF8040 */
volatile PTEDDSTR _PTEDD;                                  /* Port E Data Direction Register; 0xFFFF8041 */
volatile PTEPESTR _PTEPE;                                  /* Port E Pull Enable Register; 0xFFFF8048 */
volatile PTESESTR _PTESE;                                  /* Port E Slew Rate Enable Register; 0xFFFF8049 */
volatile PTEDSSTR _PTEDS;                                  /* Port E Drive Strength Selection Register; 0xFFFF804A */
volatile PTEIFESTR _PTEIFE;                                /* Port E Input Filter Enable Register; 0xFFFF804B */
volatile KBI2SCSTR _KBI2SC;                                /* KBI2 Status and Control Register; 0xFFFF804C */
volatile KBI2PESTR _KBI2PE;                                /* KBI2 Pin Enable Register; 0xFFFF804D */
volatile KBI2ESSTR _KBI2ES;                                /* KBI2 Edge Select Register; 0xFFFF804E */
volatile PTFDSTR _PTFD;                                    /* Port F Data Register; 0xFFFF8050 */
volatile PTFDDSTR _PTFDD;                                  /* Port F Data Direction Register; 0xFFFF8051 */
volatile PTFPESTR _PTFPE;                                  /* Port F Pull Enable Register; 0xFFFF8058 */
volatile PTFSESTR _PTFSE;                                  /* Port F Slew Rate Enable Register; 0xFFFF8059 */
volatile PTFDSSTR _PTFDS;                                  /* Port F Drive Strength Selection Register; 0xFFFF805A */
volatile PTFIFESTR _PTFIFE;                                /* Port F Input Filter Enable Register; 0xFFFF805B */
volatile PTGDSTR _PTGD;                                    /* Port G Data Register; 0xFFFF8060 */
volatile PTGDDSTR _PTGDD;                                  /* Port G Data Direction Register; 0xFFFF8061 */
volatile PTGPESTR _PTGPE;                                  /* Port G Pull Enable Register; 0xFFFF8068 */
volatile PTGSESTR _PTGSE;                                  /* Port G Slew Rate Enable Register; 0xFFFF8069 */
volatile PTGDSSTR _PTGDS;                                  /* Port G Drive Strength Selection Register; 0xFFFF806A */
volatile PTGIFESTR _PTGIFE;                                /* Port G Input Filter Enable Register; 0xFFFF806B */
volatile KBI1SCSTR _KBI1SC;                                /* KBI1 Status and Control Register; 0xFFFF806C */
volatile KBI1PESTR _KBI1PE;                                /* KBI1 Pin Enable Register; 0xFFFF806D */
volatile KBI1ESSTR _KBI1ES;                                /* KBI1 Edge Select Register; 0xFFFF806E */
volatile PTHDSTR _PTHD;                                    /* Port H Data Register; 0xFFFF8070 */
volatile PTHDDSTR _PTHDD;                                  /* Port H Data Direction Register; 0xFFFF8071 */
volatile PTHPESTR _PTHPE;                                  /* Port H Pull Enable Register; 0xFFFF8078 */
volatile PTHSESTR _PTHSE;                                  /* Port H Slew Rate Enable Register; 0xFFFF8079 */
volatile PTHDSSTR _PTHDS;                                  /* Port H Drive Strength Selection Register; 0xFFFF807A */
volatile PTHIFESTR _PTHIFE;                                /* Port H Input Filter Enable Register; 0xFFFF807B */
volatile PTJDSTR _PTJD;                                    /* Port J Data Register; 0xFFFF8080 */
volatile PTJDDSTR _PTJDD;                                  /* Port J Data Direction Register; 0xFFFF8081 */
volatile PTJPESTR _PTJPE;                                  /* Port J Pull Enable Register; 0xFFFF8088 */
volatile PTJSESTR _PTJSE;                                  /* Port J Slew Rate Enable Register; 0xFFFF8089 */
volatile PTJDSSTR _PTJDS;                                  /* Port J Drive Strength Selection Register; 0xFFFF808A */
volatile PTJIFESTR _PTJIFE;                                /* Port J Input Filter Enable Register; 0xFFFF808B */
volatile PTAPF1STR _PTAPF1;                                /* Port A Routing Register 1; 0xFFFF80C0 */
volatile PTAPF2STR _PTAPF2;                                /* Port A Routing Register 2; 0xFFFF80C1 */
volatile PTBPF1STR _PTBPF1;                                /* Port B Routing Register 1; 0xFFFF80C2 */
volatile PTBPF2STR _PTBPF2;                                /* Port B Routing Register 2; 0xFFFF80C3 */
volatile PTCPF1STR _PTCPF1;                                /* Port C Routing Register 1; 0xFFFF80C4 */
volatile PTCPF2STR _PTCPF2;                                /* Port C Routing Register 2; 0xFFFF80C5 */
volatile PTDPF1STR _PTDPF1;                                /* Port D Routing Register 1; 0xFFFF80C6 */
volatile PTDPF2STR _PTDPF2;                                /* Port D Routing Register 2; 0xFFFF80C7 */
volatile PTEPF1STR _PTEPF1;                                /* Port E Routing Register 1; 0xFFFF80C8 */
volatile PTEPF2STR _PTEPF2;                                /* Port E Routing Register 2; 0xFFFF80C9 */
volatile PTFPF1STR _PTFPF1;                                /* Port F Routing Register 1; 0xFFFF80CA */
volatile PTFPF2STR _PTFPF2;                                /* Port F Routing Register 2; 0xFFFF80CB */
volatile PTGPF1STR _PTGPF1;                                /* Port G Routing Register 1; 0xFFFF80CC */
volatile PTGPF2STR _PTGPF2;                                /* Port G Routing Register 2; 0xFFFF80CD */
volatile PTHPF1STR _PTHPF1;                                /* Port H Routing Register 1; 0xFFFF80CE */
volatile PTHPF2STR _PTHPF2;                                /* Port H Routing Register 2; 0xFFFF80CF */
volatile PTJPF1STR _PTJPF1;                                /* Port J Routing Register 1; 0xFFFF80D0 */
volatile PTJPF2STR _PTJPF2;                                /* Port J Routing Register 2; 0xFFFF80D1 */
volatile IRQSCSTR _IRQSC;                                  /* Interrupt request status and control register; 0xFFFF80E0 */
volatile SRSSTR _SRS;                                      /* System Reset Status Register; 0xFFFF8100 */
volatile SOPT1STR _SOPT1;                                  /* System Options Register 1; 0xFFFF8101 */
volatile SOPT3STR _SOPT3;                                  /* SIM Options Register 3; 0xFFFF8103 */
volatile SCGC1STR _SCGC1;                                  /* System Clock Gating Control 1 Register; 0xFFFF8108 */
volatile SCGC2STR _SCGC2;                                  /* System Clock Gating Control 2 Register; 0xFFFF8109 */
volatile SCGC3STR _SCGC3;                                  /* System Clock Gating Control 3 Register; 0xFFFF810A */
volatile SCGC4STR _SCGC4;                                  /* System Clock Gating Control 4 Register; 0xFFFF810B */
volatile SIMIPSSTR _SIMIPS;                                /* SIM Internal Peripheral Select Register; 0xFFFF810C */
volatile SPMSC1STR _SPMSC1;                                /* System Power Management Status and Control 1 Register; 0xFFFF8120 */
volatile SPMSC2STR _SPMSC2;                                /* System Power Management Status and Control 2 Register; 0xFFFF8121 */
volatile SPMSC3STR _SPMSC3;                                /* System Power Management Status and Control 3 Register; 0xFFFF8123 */
volatile ADCSC1STR _ADCSC1;                                /* Status and Control Register 1; 0xFFFF8140 */
volatile ADCSC2STR _ADCSC2;                                /* Status and Control Register 2; 0xFFFF8141 */
volatile ADCCFGSTR _ADCCFG;                                /* Configuration Register; 0xFFFF8146 */
volatile SCI1C1STR _SCI1C1;                                /* SCI1 Control Register 1; 0xFFFF8162 */
volatile SCI1C2STR _SCI1C2;                                /* SCI1 Control Register 2; 0xFFFF8163 */
volatile SCI1S1STR _SCI1S1;                                /* SCI1 Status Register 1; 0xFFFF8164 */
volatile SCI1S2STR _SCI1S2;                                /* SCI1 Status Register 2; 0xFFFF8165 */
volatile SCI1C3STR _SCI1C3;                                /* SCI1 Control Register 3; 0xFFFF8166 */
volatile SCI1DSTR _SCI1D;                                  /* SCI1 Data Register; 0xFFFF8167 */
volatile SCI2C1STR _SCI2C1;                                /* SCI2 Control Register 1; 0xFFFF8182 */
volatile SCI2C2STR _SCI2C2;                                /* SCI2 Control Register 2; 0xFFFF8183 */
volatile SCI2S1STR _SCI2S1;                                /* SCI2 Status Register 1; 0xFFFF8184 */
volatile SCI2S2STR _SCI2S2;                                /* SCI2 Status Register 2; 0xFFFF8185 */
volatile SCI2C3STR _SCI2C3;                                /* SCI2 Control Register 3; 0xFFFF8186 */
volatile SCI2DSTR _SCI2D;                                  /* SCI2 Data Register; 0xFFFF8187 */
volatile SCI3C1STR _SCI3C1;                                /* SCI3 Control Register 1; 0xFFFF81A2 */
volatile SCI3C2STR _SCI3C2;                                /* SCI3 Control Register 2; 0xFFFF81A3 */
volatile SCI3S1STR _SCI3S1;                                /* SCI3 Status Register 1; 0xFFFF81A4 */
volatile SCI3S2STR _SCI3S2;                                /* SCI3 Status Register 2; 0xFFFF81A5 */
volatile SCI3C3STR _SCI3C3;                                /* SCI3 Control Register 3; 0xFFFF81A6 */
volatile SCI3DSTR _SCI3D;                                  /* SCI3 Data Register; 0xFFFF81A7 */
volatile SPI1C1STR _SPI1C1;                                /* SPI1 Control Register 1; 0xFFFF81C0 */
volatile SPI1C2STR _SPI1C2;                                /* SPI1 Control Register 2; 0xFFFF81C1 */
volatile SPI1BRSTR _SPI1BR;                                /* SPI1 Baud Rate Register; 0xFFFF81C2 */
volatile SPI1SSTR _SPI1S;                                  /* SPI1 Status Register; 0xFFFF81C3 */
volatile SPI1DSTR _SPI1D;                                  /* SPI1 Data Register; 0xFFFF81C5 */
volatile SPI2C1STR _SPI2C1;                                /* SPI2 Control Register 1; 0xFFFF81E0 */
volatile SPI2C2STR _SPI2C2;                                /* SPI2 Control Register 2; 0xFFFF81E1 */
volatile SPI2BRSTR _SPI2BR;                                /* SPI2 Baud Rate Register; 0xFFFF81E2 */
volatile SPI2SSTR _SPI2S;                                  /* SPI2 Status Register; 0xFFFF81E3 */
volatile SPI2DSTR _SPI2D;                                  /* SPI2 Data Register; 0xFFFF81E5 */
volatile IIC1A1STR _IIC1A1;                                /* IIC Address Register; 0xFFFF8200 */
volatile IIC1FSTR _IIC1F;                                  /* IIC Frequency Divider Register; 0xFFFF8201 */
volatile IIC1C1STR _IIC1C1;                                /* IIC Control Register 1; 0xFFFF8202 */
volatile IIC1SSTR _IIC1S;                                  /* IIC Status Register; 0xFFFF8203 */
volatile IIC1DSTR _IIC1D;                                  /* IIC Data I/O Register; 0xFFFF8204 */
volatile IIC1C2STR _IIC1C2;                                /* IIC Control Register 2; 0xFFFF8205 */
volatile IIC1SMBSTR _IIC1SMB;                              /* SMBus Control and Status Register; 0xFFFF8206 */
volatile IIC1A2STR _IIC1A2;                                /* IIC Address Register 2; 0xFFFF8207 */
volatile IIC1FLTSTR _IIC1FLT;                              /* IIC Filter register; 0xFFFF820A */
volatile IIC2A1STR _IIC2A1;                                /* IIC Address Register; 0xFFFF8220 */
volatile IIC2FSTR _IIC2F;                                  /* IIC Frequency Divider Register; 0xFFFF8221 */
volatile IIC2C1STR _IIC2C1;                                /* IIC Control Register 1; 0xFFFF8222 */
volatile IIC2SSTR _IIC2S;                                  /* IIC Status Register; 0xFFFF8223 */
volatile IIC2DSTR _IIC2D;                                  /* IIC Data I/O Register; 0xFFFF8224 */
volatile IIC2C2STR _IIC2C2;                                /* IIC Control Register 2; 0xFFFF8225 */
volatile IIC2SMBSTR _IIC2SMB;                              /* SMBus Control and Status Register; 0xFFFF8226 */
volatile IIC2A2STR _IIC2A2;                                /* IIC Address Register 2; 0xFFFF8227 */
volatile IIC2FLTSTR _IIC2FLT;                              /* IIC Filter register; 0xFFFF822A */
volatile MCGC1STR _MCGC1;                                  /* MCG Control Register 1; 0xFFFF8240 */
volatile MCGC2STR _MCGC2;                                  /* MCG Control Register 2; 0xFFFF8241 */
volatile MCGTRMSTR _MCGTRM;                                /* MCG Trim Register; 0xFFFF8242 */
volatile MCGSCSTR _MCGSC;                                  /* MCG Status and Control Register; 0xFFFF8243 */
volatile MCGC3STR _MCGC3;                                  /* MCG Control Register 3; 0xFFFF8244 */
volatile MCGC4STR _MCGC4;                                  /* MCG Control Register 4; 0xFFFF8245 */
volatile TPM1SCSTR _TPM1SC;                                /* TPM1 Status and Control Register; 0xFFFF8260 */
volatile TPM1C0SCSTR _TPM1C0SC;                            /* TPM1 Timer Channel 0 Status and Control Register; 0xFFFF8265 */
volatile TPM1C1SCSTR _TPM1C1SC;                            /* TPM1 Timer Channel 1 Status and Control Register; 0xFFFF8268 */
volatile TPM1C2SCSTR _TPM1C2SC;                            /* TPM1 Timer Channel 2 Status and Control Register; 0xFFFF826B */
volatile TPM2SCSTR _TPM2SC;                                /* TPM2 Status and Control Register; 0xFFFF8280 */
volatile TPM2C0SCSTR _TPM2C0SC;                            /* TPM2 Timer Channel 0 Status and Control Register; 0xFFFF8285 */
volatile TPM2C1SCSTR _TPM2C1SC;                            /* TPM2 Timer Channel 1 Status and Control Register; 0xFFFF8288 */
volatile TPM2C2SCSTR _TPM2C2SC;                            /* TPM2 Timer Channel 2 Status and Control Register; 0xFFFF828B */
volatile MTIM1SCSTR _MTIM1SC;                              /* MTIM Clock Configuration Register; 0xFFFF82A0 */
volatile MTIM1CLKSTR _MTIM1CLK;                            /* MTIM Clock Configuration Register; 0xFFFF82A1 */
volatile MTIM1CNTSTR _MTIM1CNT;                            /* MTIM Counter Register; 0xFFFF82A2 */
volatile MTIM1MODSTR _MTIM1MOD;                            /* MTIM Modulo Register; 0xFFFF82A3 */
volatile RTCSCSTR _RTCSC;                                  /* RTC Status and Control Register; 0xFFFF82C0 */
volatile RTCCNTSTR _RTCCNT;                                /* RTC Counter Register; 0xFFFF82C1 */
volatile RTCMODSTR _RTCMOD;                                /* RTC Modulo Register; 0xFFFF82C2 */
volatile FCDIVSTR _FCDIV;                                  /* FLASH Clock Divider Register; 0xFFFF82E0 */
volatile FOPTSTR _FOPT;                                    /* Flash Options Register; 0xFFFF82E1 */
volatile FCNFGSTR _FCNFG;                                  /* Flash Configuration Register; 0xFFFF82E3 */
volatile FPROTSTR _FPROT;                                  /* Flash Protection Register; 0xFFFF82E4 */
volatile FSTATSTR _FSTAT;                                  /* Flash Status Register; 0xFFFF82E5 */
volatile FCMDSTR _FCMD;                                    /* Flash Command Register; 0xFFFF82E6 */
volatile MTIM2SCSTR _MTIM2SC;                              /* MTIM Clock Configuration Register; 0xFFFF8300 */
volatile MTIM2CLKSTR _MTIM2CLK;                            /* MTIM Clock Configuration Register; 0xFFFF8301 */
volatile MTIM2CNTSTR _MTIM2CNT;                            /* MTIM Counter Register; 0xFFFF8302 */
volatile MTIM2MODSTR _MTIM2MOD;                            /* MTIM Modulo Register; 0xFFFF8303 */
volatile INTC_FRCSTR _INTC_FRC;                            /* INTC Force Interrupt Register; 0xFFFFFFD0 */
volatile INTC_PL6P7STR _INTC_PL6P7;                        /* INTC Programmable Level 6, Priority 7 Register; 0xFFFFFFD8 */
volatile INTC_PL6P6STR _INTC_PL6P6;                        /* INTC Programmable Level 6, Priority 6 Register; 0xFFFFFFD9 */
volatile INTC_WCRSTR _INTC_WCR;                            /* INTC Wake-up Control Register; 0xFFFFFFDB */
volatile INTC_SFRCSTR _INTC_SFRC;                          /* INTC Set Interrupt Force Register; 0xFFFFFFDE */
volatile INTC_CFRCSTR _INTC_CFRC;                          /* INTC Clear Interrupt Force Register; 0xFFFFFFDF */
volatile INTC_SWIACKSTR _INTC_SWIACK;                      /* INTC Software IACK Register; 0xFFFFFFE0 */
volatile INTC_LVL1IACKSTR _INTC_LVL1IACK;                  /* INTC Level 1 IACK Register; 0xFFFFFFE4 */
volatile INTC_LVL2IACKSTR _INTC_LVL2IACK;                  /* INTC Level 2 IACK Register; 0xFFFFFFE8 */
volatile INTC_LVL3IACKSTR _INTC_LVL3IACK;                  /* INTC Level 3 IACK Register; 0xFFFFFFEC */
volatile INTC_LVL4IACKSTR _INTC_LVL4IACK;                  /* INTC Level 4 IACK Register; 0xFFFFFFF0 */
volatile INTC_LVL5IACKSTR _INTC_LVL5IACK;                  /* INTC Level 5 IACK Register; 0xFFFFFFF4 */
volatile INTC_LVL6IACKSTR _INTC_LVL6IACK;                  /* INTC Level 6 IACK Register; 0xFFFFFFF8 */
volatile INTC_LVL7IACKSTR _INTC_LVL7IACK;                  /* INTC Level 7 IACK Register; 0xFFFFFFFC */


/* * * * *  16-BIT REGISTERS  * * * * * * * * * * * * * * * */
volatile RGPIO_DIRSTR _RGPIO_DIR;                          /* RGPIO Data Direction Register; 0x00C00000 */
volatile RGPIO_DATASTR _RGPIO_DATA;                        /* RGPIO Data Register; 0x00C00002 */
volatile RGPIO_ENBSTR _RGPIO_ENB;                          /* RGPIO Pin Enable Register; 0x00C00004 */
volatile RGPIO_CLRSTR _RGPIO_CLR;                          /* RGPIO Clear Data Register; 0x00C00006 */
volatile RGPIO_SETSTR _RGPIO_SET;                          /* RGPIO Set Data Register; 0x00C0000A */
volatile RGPIO_TOGSTR _RGPIO_TOG;                          /* RGPIO Toggle Data Register; 0x00C0000E */
volatile SDIDSTR _SDID;                                    /* System Device Identification Register; 0xFFFF8106 */
volatile ADCRSTR _ADCR;                                    /* Data Result Register; 0xFFFF8142 */
volatile ADCCVSTR _ADCCV;                                  /* Compare Value Register; 0xFFFF8144 */
volatile SCI1BDSTR _SCI1BD;                                /* SCI1 Baud Rate Register; 0xFFFF8160 */
volatile SCI2BDSTR _SCI2BD;                                /* SCI2 Baud Rate Register; 0xFFFF8180 */
volatile SCI3BDSTR _SCI3BD;                                /* SCI3 Baud Rate Register; 0xFFFF81A0 */
volatile IIC1SLTSTR _IIC1SLT;                              /* IIC SCL Low Time Out register; 0xFFFF8208 */
volatile IIC2SLTSTR _IIC2SLT;                              /* IIC SCL Low Time Out register; 0xFFFF8228 */
volatile TPM1CNTSTR _TPM1CNT;                              /* TPM1 Timer Counter Register; 0xFFFF8261 */
volatile TPM1MODSTR _TPM1MOD;                              /* TPM1 Timer Counter Modulo Register; 0xFFFF8263 */
volatile TPM1C0VSTR _TPM1C0V;                              /* TPM1 Timer Channel 0 Value Register; 0xFFFF8266 */
volatile TPM1C1VSTR _TPM1C1V;                              /* TPM1 Timer Channel 1 Value Register; 0xFFFF8269 */
volatile TPM1C2VSTR _TPM1C2V;                              /* TPM1 Timer Channel 2 Value Register; 0xFFFF826C */
volatile TPM2CNTSTR _TPM2CNT;                              /* TPM2 Timer Counter Register; 0xFFFF8281 */
volatile TPM2MODSTR _TPM2MOD;                              /* TPM2 Timer Counter Modulo Register; 0xFFFF8283 */
volatile TPM2C0VSTR _TPM2C0V;                              /* TPM2 Timer Channel 0 Value Register; 0xFFFF8286 */
volatile TPM2C1VSTR _TPM2C1V;                              /* TPM2 Timer Channel 1 Value Register; 0xFFFF8289 */
volatile TPM2C2VSTR _TPM2C2V;                              /* TPM2 Timer Channel 2 Value Register; 0xFFFF828C */
volatile INTC_ORMRSTR _INTC_ORMR;                          /* INTC OR Mask Register; 0xFFFFFFCC */


/* * * * *  32-BIT REGISTERS  * * * * * * * * * * * * * * * */
volatile EIRSTR _EIR;                                      /* Ethernet Interrupt Event Register; 0xFFFFE004 */
volatile EIMRSTR _EIMR;                                    /* Interrupt Mask Register; 0xFFFFE008 */
volatile RDARSTR _RDAR;                                    /* Receive Descriptor Active Register; 0xFFFFE010 */
volatile TDARSTR _TDAR;                                    /* Transmit Descriptor Active Register; 0xFFFFE014 */
volatile ECRSTR _ECR;                                      /*  Ethernet Control Register; 0xFFFFE024 */
volatile MMFRSTR _MMFR;                                    /* MII Management Frame Register; 0xFFFFE040 */
volatile MSCRSTR _MSCR;                                    /* MII Speed Control Register; 0xFFFFE044 */
volatile RCRSTR _RCR;                                      /* Receive Control Register; 0xFFFFE084 */
volatile TCRSTR _TCR;                                      /* Transmit Control Register; 0xFFFFE0C4 */
volatile PALRSTR _PALR;                                    /* Physical Address Low Register; 0xFFFFE0E4 */
volatile PAURSTR _PAUR;                                    /* Physical Address High Register; 0xFFFFE0E8 */
volatile OPDSTR _OPD;                                      /* Opcode/Pause Duration Register; 0xFFFFE0EC */
volatile IAURSTR _IAUR;                                    /* Descriptor Individual Upper Address Register; 0xFFFFE118 */
volatile IALRSTR _IALR;                                    /* Descriptor Individual Lower Address Register; 0xFFFFE11C */
volatile GAURSTR _GAUR;                                    /* Descriptor Group Upper Address Register; 0xFFFFE120 */
volatile GALRSTR _GALR;                                    /* Descriptor Group Lower Address Register; 0xFFFFE124 */
volatile TFWRSTR _TFWR;                                    /* FIFO Transmit FIFO Watermark Register; 0xFFFFE144 */
volatile FRBRSTR _FRBR;                                    /* FIFO Receive Bound Register; 0xFFFFE14C */
volatile FRSRSTR _FRSR;                                    /* FIFO Receive Start Register; 0xFFFFE150 */
volatile ERDSRSTR _ERDSR;                                  /* Receive Descriptor Ring Start Register; 0xFFFFE180 */
volatile ETSDRSTR _ETSDR;                                  /* Transmit Buffer Descriptor Ring Start Register; 0xFFFFE184 */
volatile EMRBRSTR _EMRBR;                                  /* Receive Buffer Size Register; 0xFFFFE188 */
volatile CSAR0STR _CSAR0;                                  /* Chip Select Address Register 0; 0xFFFFE800 */
volatile CSMR0STR _CSMR0;                                  /* Chip Select Mask Register 0; 0xFFFFE804 */
volatile CSCR0STR _CSCR0;                                  /* Chip Select Control Register 0; 0xFFFFE808 */
volatile CSAR1STR _CSAR1;                                  /* Chip Select Address Register 1; 0xFFFFE80C */
volatile CSMR1STR _CSMR1;                                  /* Chip Select Mask Register 1; 0xFFFFE810 */
volatile CSCR1STR _CSCR1;                                  /* Chip Select Control Register 1; 0xFFFFE814 */

/*lint -restore */

/* EOF */
