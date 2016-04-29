/*
 * @brief LWIP FreeRTOS TCP Echo example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "lwip/init.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/timers.h"
#include "netif/etharp.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

#include "board.h"
#include "arch\lpc18xx_43xx_emac.h"
#include "arch\lpc_arch.h"
#include "arch\sys_arch.h"
#include "lpc_phy.h"/* For the PHY monitor support */
#include "tcpecho.h"

/** @defgroup EXAMPLE_LWIP_TCPECHO_FREERTOS_18XX43XX LWIP TCP Echo with FreeRTOS
 * @ingroup EXAMPLES_LWIP_18XX43XX
 * Welcome to the LWIP TCP Echo example using the NET API for RTOS based
 * operation. This example shows how to use the NET API with the LWIP contrib
 * TCP Echo (threaded) example using the 18xx/43xx LWIP MAC and PHY drivers.
 * The example shows how to handle PHY link monitoring and indicate to LWIP that
 * a ethernet cable is plugged in.
 *
 * To use the example, Simply connect an ethernet cable to the board. The board
 * will acquire an IP address via DHCP and you can ping the board at it's IP
 * address. You can monitor network traffice to the board using a tool such as
 * wireshark at the boards MAC address.
 *
 * Build procedure:
 * <a href="http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-build-procedures/lpc18xx/43xx-lpco">LPCOpen 18xx/43xx build instructions</a>
 *
 * Submit bug reports for LPCOpen code <a href="http://www.lpcware.com/content/bugtrackerproject/lpcopen">here.</a>
 * @{
 */

/* When building the example to run in FLASH, the number of available
   pbufs and memory size (in lwipopts.h) and the number of descriptors
   (in lpc_18xx43xx_emac_config.h) can be increased due to more available
   IRAM. */

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* NETIF data */
static struct netif lpc_netif;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	/* LED0 is used for the link status, on = PHY cable detected */
	Board_Init();
	Board_LED_Init();

	/* Initial LED state is off to show an unconnected cable state */
	Board_LED_Set(0, false);
}

/* Callback for TCPIP thread to indicate TCPIP init is done */
static void tcpip_init_done_signal(void *arg)
{
	/* Tell main thread TCP/IP init is done */
	*(s32_t *) arg = 1;
}

/* LWIP kickoff and PHY link monitor thread */
static portTASK_FUNCTION(vSetupIFTask, pvParameters) {
	ip_addr_t ipaddr, netmask, gw;
	volatile s32_t tcpipdone = 0;
	uint32_t physts;
	static int prt_ip = 0;

	/* Wait until the TCP/IP thread is finished before
	   continuing or wierd things may happen */
	LWIP_DEBUGF(LWIP_DBG_ON, ("Waiting for TCPIP thread to initialize...\n"));
	tcpip_init(tcpip_init_done_signal, (void *) &tcpipdone);
	while (!tcpipdone) {
		msDelay(1);
	}

	LWIP_DEBUGF(LWIP_DBG_ON, ("Starting LWIP TCP echo server...\n"));

	/* Static IP assignment */
#if LWIP_DHCP
	IP4_ADDR(&gw, 0, 0, 0, 0);
	IP4_ADDR(&ipaddr, 0, 0, 0, 0);
	IP4_ADDR(&netmask, 0, 0, 0, 0);
#else
	IP4_ADDR(&gw, 0, 0, 0, 0);
	IP4_ADDR(&ipaddr, 126, 126, 126, 155);
	IP4_ADDR(&netmask, 255, 0, 0, 0);
//	IP4_ADDR(&gw, 10, 1, 10, 1);
//	IP4_ADDR(&ipaddr, 10, 1, 10, 234);
//	IP4_ADDR(&netmask, 255, 255, 255, 0);
#endif

	/* Add netif interface for lpc17xx_8x */
	if (!netif_add(&lpc_netif, &ipaddr, &netmask, &gw, NULL, lpc_enetif_init,
				   tcpip_input)) {
		LWIP_ASSERT("Net interface failed to initialize\r\n", 0);
	}
	netif_set_default(&lpc_netif);
	netif_set_up(&lpc_netif);

	/* Enable MAC interrupts only after LWIP is ready */
	NVIC_SetPriority(ETHERNET_IRQn, config_ETHERNET_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(ETHERNET_IRQn);

#if LWIP_DHCP
	dhcp_start(&lpc_netif);
#endif

	/* Initialize and start application */
	tcpecho_init();

	/* This loop monitors the PHY link and will handle cable events
	   via the PHY driver. */
	while (1) {
		/* Call the PHY status update state machine once in a while
		   to keep the link status up-to-date */
		physts = lpcPHYStsPoll();

		/* Only check for connection state when the PHY status has changed */
		if (physts & PHY_LINK_CHANGED) {
			if (physts & PHY_LINK_CONNECTED) {
				Board_LED_Set(0, true);

				/* Set interface speed and duplex */
				if (physts & PHY_LINK_SPEED100) {
					Chip_ENET_Set_Speed(1);
					NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 100000000);
				}
				else {
					Chip_ENET_Set_Speed(0);
					NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 10000000);
				}
				if (physts & PHY_LINK_FULLDUPLX) {
					Chip_ENET_Set_Duplex(true);
				}
				else {
					Chip_ENET_Set_Duplex(false);
				}

				tcpip_callback_with_block((tcpip_callback_fn) netif_set_link_up,
										  (void *) &lpc_netif, 1);
			}
			else {
				Board_LED_Set(0, false);
				tcpip_callback_with_block((tcpip_callback_fn) netif_set_link_down,
										  (void *) &lpc_netif, 1);
			}

			DEBUGOUT("Link connect status: %d\r\n", ((physts & PHY_LINK_CONNECTED) != 0));

			/* Delay for link detection (250mS) */
			vTaskDelay(configTICK_RATE_HZ / 4);
		}

		/* Print IP address info */
		if (!prt_ip) {
			if (lpc_netif.ip_addr.addr) {
				static char tmp_buff[16];
				DEBUGOUT("IP_ADDR    : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.ip_addr, tmp_buff, 16));
				DEBUGOUT("NET_MASK   : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.netmask, tmp_buff, 16));
				DEBUGOUT("GATEWAY_IP : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.gw, tmp_buff, 16));
				prt_ip = 1;
			}
		}
	}
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	MilliSecond delay function based on FreeRTOS
 * @param	ms	: Number of milliSeconds to delay
 * @return	Nothing
 * Needed for some functions, do not use prior to FreeRTOS running
 */
void msDelay(uint32_t ms)
{
	vTaskDelay((configTICK_RATE_HZ * ms) / 1000);
}

/* task list */
int vSetupPollTask (void);
int vLcdTask (void);
int vLcdTaskNew (void);
int vRTCTask (void);

#define TASK_PRIO_LCD             (tskIDLE_PRIORITY + 0UL)

xSemaphoreHandle xSemaDataAvail, xSemaGUIend;
IP_RTC_TIME_T FullTime, dataInizio, dataFine, scadenza;
#define maxR 5
#define maxC 100
uint8_t l_tabellaMedicine[maxR][maxC] = {
	{
	0x9b,0x83,0x1e,0x9f, /*TAG - 9B831E9F [0-3]*/
	0x54,0x41,0x43,0x48,0x49,0x50,0x49,0x52,0x49,0x4e,
	0x41,0x20,0x31,0x30,0x30,0x30,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20, /*NOME - TACHIPIRINA 1000 [4-38]*/
	0x1f,0x0c,0x07,0xe0, /*SCADENZA - 31/12/2016 [39-42]*/
	0x01,0x04,0x07,0xe0, /*DATA INIZIO ASSUNZIONE - 01/04/2016 [43-46]*/
	0x1e,0x04,0x07,0xe0, /*DATA FINE ASSUNZIONE - 30/04/2016 [47-50]*/
	0x09,0x00,0x00, /*ORA ASSUNZIONE - 9:00:00 [51-53]*/
	0x7f,// - 01111111  -  0x40, /*RIPETIZIONE - 01000000 [54]*/
	0x04, /*DOSE - 00000100 [55]*/
	0x00,0x00,0x00,0x00, /*CONTATORI GIORNI E ORE - 0|0|0|0 [56-59]*/
	0x00,0x00, /*PRESA e IMPORTANZA - 0|0 [60-61]*/
	0x04,0x00, //0x14,0x00, /*NUM DOSI RIMASTE - INT+DECIM 20|0 [62-63]*/
	0x05, /*SOGLIA AVVISO - 5 [64]*/
	0x00, /*PIU' VOLTE AL GIORNO - 0 [65]*/
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20 /*NOTE - empty [66-99]*/
	},
	{
	0x6b,0xec,0x1d,0x9f, /*TAG - 6BEC1D9F [0-3]*/
	0x4e,0x65,0x6f,0x20,0x42,0x4f,0x52,0x4f,0x43,0x49,
	0x4c,0x4c,0x49,0x4e,0x41,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20, /*NOME - Neo BOROCILLINA [4-38]*/
	0x1e,0x0c,0x07,0xe0, /*SCADENZA - 30/12/2016 [39-42]*/
	0x05,0x04,0x07,0xe0, /*DATA INIZIO ASSUNZIONE - 05/04/2016 [43-46]*/
	0x1e,0x04,0x07,0xe0, /*DATA FINE ASSUNZIONE - 30/04/2016 [47-50]*/
	0x09,0x02,0x00, /*ORA ASSUNZIONE - 9:02:00 [51-53]*/
	0x50, /*RIPETIZIONE - 01010000 [54]*/
	0x04, /*DOSE - 00000100 [55]*/
	0x00,0x00,0x00,0x00, /*CONTATORI GIORNI E ORE - 0|0|0|0 [56-59]*/
	0x00,0x00, /*PRESA e IMPORTANZA - 0|0 [60-61]*/
	0x14,0x00, /*NUM DOSI RIMASTE - INT+DECIM 20|0 [62-63]*/
	0x05, /*SOGLIA AVVISO - 5 [64]*/
	0x00, /*PIU' VOLTE AL GIORNO - 0 [65]*/
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20 /*NOTE - empty [66-99]*/
	},
	{
	0xab,0xec,0x1d,0x9f, /*TAG - ABEC1D9F [0-3]*/
	0x50,0x52,0x4f,0x56,0x41,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20, /*NOME - PROVA [4-38]*/
	0x1e,0x0c,0x07,0xe0, /*SCADENZA - 30/12/2016 [39-42]*/
	0x05,0x04,0x07,0xe0, /*DATA INIZIO ASSUNZIONE - 05/04/2016 [43-46]*/
	0x1e,0x04,0x07,0xe0, /*DATA FINE ASSUNZIONE - 30/04/2016 [47-50]*/
	0x08,0x00,0x00, /*ORA ASSUNZIONE - 8:00:00 [51-53]*/
	0x00, /*RIPETIZIONE - 00000000 [54]*/
	0x04, /*DOSE - 00000100 [55]*/
	0x08,0x08,0x00,0x00, /*CONTATORI ORE E GIORNI - 8|8|0|0 [56-59]*/
	0x00,0x00, /*PRESA e IMPORTANZA - 0|0 [60-61]*/
	0x14,0x00, /*NUM DOSI RIMASTE - INT+DECIM 20|0 [62-63]*/
	0x05, /*SOGLIA AVVISO - 5 [64]*/
	0x01, /*PIU' VOLTE AL GIORNO - 1 [65]*/
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20 /*NOTE - empty [66-99]*/
	},
	{
	0xbb,0x4a,0x1d,0x9f, /*TAG - BB4A1D9F [0-3]*/
	0x4d,0x45,0x44,0x49,0x43,0x49,0x4e,0x41,0x20,0x50,
	0x45,0x52,0x53,0x4f,0x4e,0x41,0x4c,0x49,0x5a,0x5a,
	0x41,0x54,0x41,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20, /*NOME - MEDICINA PERSONALIZZATA [4-38]*/
	0x1e,0x0c,0x07,0xe0, /*SCADENZA - 30/12/2016 [39-42]*/
	0x05,0x04,0x07,0xe0, /*DATA INIZIO ASSUNZIONE - 05/04/2016 [43-46]*/
	0x1e,0x04,0x07,0xe0, /*DATA FINE ASSUNZIONE - 30/04/2016 [47-50]*/
	0x06,0x00,0x00, /*ORA ASSUNZIONE - 6:00:00 [51-53]*/
	0x00, /*RIPETIZIONE - 00000000 [54]*/
	0x04, /*DOSE - 00000100 [55]*/
	0x00,0x00,0x00,0x00, /*CONTATORI ORE E GIORNI - 0|0|0|0 [56-59]*/
	0x00,0x00, /*PRESA e IMPORTANZA - 0|0 [60-61]*/
	0x14,0x00, /*NUM DOSI RIMASTE - INT+DECIM 20|0 [62-63]*/
	0x05, /*SOGLIA AVVISO - 5 [64]*/
	0x05, /*PIU' VOLTE AL GIORNO - 5 [65]*/
	0x08,0x00,0x00,/*ora 2 personalizzata - 8:00:00 [66-68]*/
	0x0a,0x00,0x00,/*ora 3 personalizzata - 10:00:00 [69-71]*/
	0x14,0x00,0x00,/*ora 4 personalizzata - 20:00:00 [72-74]*/
	0x15,0x00,0x00,/*ora 5 personalizzata - 21:00:00 [75-77]*/
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20 /*NOTE - empty [78-99]*/
	},
	{
	0xfb,0x4a,0x1d,0x9f, /*TAG - FB4A1D9F [0-3]*/
	0x50,0x52,0x4f,0x56,0x41,0x20,0x4f,0x47,0x4e,0x49,
	0x20,0x32,0x20,0x47,0x49,0x4f,0x52,0x4e,0x49,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20, /*NOME - PROVA OGNI 2 GIORNI [4-38]*/
	0x1e,0x0c,0x07,0xe0, /*SCADENZA - 30/12/2016 [39-42]*/
	0x05,0x04,0x07,0xe0, /*DATA INIZIO ASSUNZIONE - 05/04/2016 [43-46]*/
	0x1e,0x04,0x07,0xe0, /*DATA FINE ASSUNZIONE - 30/04/2016 [47-50]*/
	0x10,0x00,0x00, /*ORA ASSUNZIONE - 10:00:00 [51-53]*/
	0x00, /*RIPETIZIONE - 00000000 [54]*/
	0x04, /*DOSE - 00000100 [55]*/
	0x00,0x00,0x01,0x02, /*CONTATORI ORE E GIORNI - 0|0|1|2 [56-59]*/
	0x00,0x00, /*PRESA e IMPORTANZA - 0|0 [60-61]*/
	0x14,0x00, /*NUM DOSI RIMASTE - INT+DECIM 20|0 [62-63]*/
	0x05, /*SOGLIA AVVISO - 5 [64]*/
	0x00, /*PIU' VOLTE AL GIORNO - 0 [65]*/
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20 /*NOTE - empty [66-99]*/
	}
};
typedef struct {
	uint8_t tag[4];
	char nome[35];
	int secA;
	int minA;
	int oraA;
	int scaduta;
	int avvisare;
	int importanza;
	int presa;
	struct medicinale *next;
} medicinale;
medicinale medic; //creo medicinale
medicinale *ptrMedic;

medicinale* inizializzaListaMed(){
	return NULL;
}
int inEsaurimento(uint16_t r, uint8_t s){
	uint16_t tmp = s<<2;
	if (r>tmp) return 0;
	else return 1;
}
medicinale* inserisciInTestaListaMed(medicinale* l, uint8_t m[100]){
	medicinale *temp;
	temp=malloc(sizeof(medicinale));

	//ESTRAI DATA INIZIO E DATA FINE
	dataInizio.time[RTC_TIMETYPE_DAYOFMONTH] = m[43];
	dataInizio.time[RTC_TIMETYPE_MONTH] = m[44];
	dataInizio.time[RTC_TIMETYPE_YEAR] = (m[45]<<8) | m[46];

	dataFine.time[RTC_TIMETYPE_DAYOFMONTH] = m[47];
	dataFine.time[RTC_TIMETYPE_MONTH] = m[48];
	dataFine.time[RTC_TIMETYPE_YEAR] = (m[49]<<8) | m[50];

/*
	uint8_t cO = m[56], cOT = m[57], cG = m[58], cGT = m[59];
	if (cGT!=0){
		if (cG!=0) {
			cG--;
		} else {//da prendere e settare cG=cGT;
			cG--;
		}
	}else {
		if (cOT!=0){
			if (cOT<24) {
				//PIU VOLTE IN UN GIORNO! DA PENSARE
			} else {
				if (cO<24) {
					//da prendere e settare cO=cOT;
				} else {
					cO-=24;
				}
			}
		}else {
*/
			uint8_t dowFT, dowM = m[54];
			dowFT = FullTime.time[RTC_TIMETYPE_DAYOFWEEK];
			dowM <<= dowFT;
			dowM >>= 7;

			/*
			 * Se la data di oggi è compresa tra il giorno di inizio assunzione e quella di fine assunzione
			 * e se il giorno della settimana coincide, allora creo l'avviso legato al medicinale
			 */
			if ((dowM == 1) && (dataDopoData(FullTime,dataInizio)) && (dataPrimaData(FullTime,dataFine))) {

			//	estraiTagENome(m1,medic);
				uint8_t c = 0;
				for (c=0;c<4;c++){
					temp->tag[c] = m[c];
				}
				for (c=0;c<35;c++){
					temp->nome[c] = m[c+4];
				}

			//	estraiOra(m1,medic);
				uint8_t pvag = m[65];
				if (pvag==0x00){
					//Assumere una volta al giorno: ho una sola ora per giorno.
					uint8_t hh=m[51], mm=m[52], ss=m[53];
					temp->oraA = hh;
					temp->minA = mm;
					temp->secA = ss;
				}else{
					//Ho più assunzioni al giorno della medicina
					//DA DECIDERE COME FARE
				}

				//ESTRAI DATA DI SCADENZA
				scadenza.time[RTC_TIMETYPE_DAYOFMONTH] = m[39];
				scadenza.time[RTC_TIMETYPE_MONTH] = m[40];
				scadenza.time[RTC_TIMETYPE_YEAR] = (m[41]<<8) | m[42];
				temp->scaduta = dataDopoData(FullTime,scadenza);

				uint8_t /*dose = m[55],*/ rimasteInt = m[62], rimasteFraz = m[63], sogliaAvvisoEsaurimento = m[64];
				uint16_t rimaste = (rimasteInt << 2) | rimasteFraz; //0b11

				temp->avvisare = inEsaurimento(rimaste, sogliaAvvisoEsaurimento);
				temp->presa = 0;
//				int aaa = 1;
				temp->next = l;
			} else {
				return l;
/*
			}
		}
*/
	}

//	//temp->info = buid;
//	strcpy(temp->info,buid);
//	temp->prox = l;
	return temp;
}
medicinale* inserisciInTestaListaMedOra(medicinale* l, uint8_t m[100], IP_RTC_TIME_T ora){
	medicinale *temp;
	temp=malloc(sizeof(medicinale));

	//ESTRAI DATA INIZIO E DATA FINE
	dataInizio.time[RTC_TIMETYPE_DAYOFMONTH] = m[43];
	dataInizio.time[RTC_TIMETYPE_MONTH] = m[44];
	dataInizio.time[RTC_TIMETYPE_YEAR] = (m[45]<<8) | m[46];

	dataFine.time[RTC_TIMETYPE_DAYOFMONTH] = m[47];
	dataFine.time[RTC_TIMETYPE_MONTH] = m[48];
	dataFine.time[RTC_TIMETYPE_YEAR] = (m[49]<<8) | m[50];

	if ((dataDopoData(FullTime,dataInizio)) && (dataPrimaData(FullTime,dataFine))) {

	//	estraiTagENome(m1,medic);
		uint8_t c = 0;
		for (c=0;c<4;c++){
			temp->tag[c] = m[c];
		}
		for (c=0;c<35;c++){
			temp->nome[c] = m[c+4];
		}

		temp->oraA = ora.time[RTC_TIMETYPE_HOUR];
		temp->minA = ora.time[RTC_TIMETYPE_MINUTE];
		temp->secA = ora.time[RTC_TIMETYPE_SECOND];

		//ESTRAI DATA DI SCADENZA
		scadenza.time[RTC_TIMETYPE_DAYOFMONTH] = m[39];
		scadenza.time[RTC_TIMETYPE_MONTH] = m[40];
		scadenza.time[RTC_TIMETYPE_YEAR] = (m[41]<<8) | m[42];
		temp->scaduta = dataDopoData(FullTime,scadenza);

		uint8_t rimasteInt = m[62], rimasteFraz = m[63], sogliaAvvisoEsaurimento = m[64];
		uint16_t rimaste = (rimasteInt << 2) | rimasteFraz; //0b11

		temp->avvisare = inEsaurimento(rimaste, sogliaAvvisoEsaurimento);
		temp->presa = 0;
//		int aaa = 1;
		temp->next = l;
	} else {
		return l;
	}
	return temp;
}

//uint8_t tagRimosso, tagAggiunto; //INUTILE

uint8_t *nomeMed;
uint8_t arrNomiMed[35];

//estraiTagENome(uint8_t m, medicinale med){
//	uint8_t c = 0;
//	for (c=0;c<4;c++){
//		med.tag[c] = m[c];
//	}
//	for (c=0;c<35;c++){
//		med.nome[c] = m[c+4];
//	}
//}
//
//void estraiOra(uint8_t m, medicinale med){
//	uint8_t pvag;
//	if (pvag==0x20){
//		//Assumere una volta al giorno: ho una sola ora per giorno.
//		uint8_t hh=m[51], mm=m[52], ss=m[53];
//		med.oraA = hh;
//		med.minA = mm;
//		med.secA = ss;
//	}else{
//		//Ho più assunzioni al giorno della medicina
//		//DA DECIDERE COME FARE
//	}
//}

int orarioDopoOrario(IP_RTC_TIME_T tempo1, IP_RTC_TIME_T tempo2){
	//Chip_RTC_SetFullTime(&FullTime);
	if (tempo1.time[RTC_TIMETYPE_HOUR]<tempo2.time[RTC_TIMETYPE_HOUR]) return 0;
	else {
		if (tempo1.time[RTC_TIMETYPE_MINUTE]<tempo2.time[RTC_TIMETYPE_MINUTE]) return 0;
		else {
			if (tempo1.time[RTC_TIMETYPE_SECOND]<tempo2.time[RTC_TIMETYPE_SECOND]) return 0;
			else
				return 1;
		}
	}
}
int dataDopoData(IP_RTC_TIME_T data1, IP_RTC_TIME_T data2){
	//Chip_RTC_SetFullTime(&FullTime);
	if (data1.time[RTC_TIMETYPE_YEAR]<data2.time[RTC_TIMETYPE_YEAR]) return 0;
	else {
		if (data1.time[RTC_TIMETYPE_MONTH]<data2.time[RTC_TIMETYPE_MONTH]) return 0;
		else {
			if (data1.time[RTC_TIMETYPE_DAYOFMONTH]<data2.time[RTC_TIMETYPE_DAYOFMONTH]) return 0;
			else return 1;
		}
	}
}
int dataPrimaData(IP_RTC_TIME_T data1, IP_RTC_TIME_T data2){
	//Chip_RTC_SetFullTime(&FullTime);
	if (data1.time[RTC_TIMETYPE_YEAR]>data2.time[RTC_TIMETYPE_YEAR]) return 0;
	else {
		if (data1.time[RTC_TIMETYPE_MONTH]>data2.time[RTC_TIMETYPE_MONTH]) return 0;
		else {
			if (data1.time[RTC_TIMETYPE_DAYOFMONTH]>data2.time[RTC_TIMETYPE_DAYOFMONTH]) return 0;
			else return 1;
		}
	}
}
IP_RTC_TIME_T differenzaDate(IP_RTC_TIME_T data1, IP_RTC_TIME_T data2){ // DA VALORI SBALLATI TRA GIORNI E ORE...
	IP_RTC_TIME_T diff;
	diff.time[RTC_TIMETYPE_SECOND] = data1.time[RTC_TIMETYPE_SECOND] - data2.time[RTC_TIMETYPE_SECOND];
	diff.time[RTC_TIMETYPE_MINUTE] = data1.time[RTC_TIMETYPE_MINUTE] - data2.time[RTC_TIMETYPE_MINUTE];
	diff.time[RTC_TIMETYPE_HOUR] = data1.time[RTC_TIMETYPE_HOUR] - data2.time[RTC_TIMETYPE_HOUR];
	diff.time[RTC_TIMETYPE_DAYOFMONTH] = data1.time[RTC_TIMETYPE_DAYOFMONTH] - data2.time[RTC_TIMETYPE_DAYOFMONTH];
	diff.time[RTC_TIMETYPE_DAYOFWEEK] = data1.time[RTC_TIMETYPE_DAYOFWEEK] - data2.time[RTC_TIMETYPE_DAYOFWEEK];
	diff.time[RTC_TIMETYPE_DAYOFYEAR] = data1.time[RTC_TIMETYPE_DAYOFYEAR] - data2.time[RTC_TIMETYPE_DAYOFYEAR];
	diff.time[RTC_TIMETYPE_MONTH] = data1.time[RTC_TIMETYPE_MONTH] - data2.time[RTC_TIMETYPE_MONTH];
	diff.time[RTC_TIMETYPE_YEAR] = data1.time[RTC_TIMETYPE_YEAR] - data2.time[RTC_TIMETYPE_YEAR];
	return diff;
}
int differenzaGiorni(IP_RTC_TIME_T data1, IP_RTC_TIME_T data2) {
	if (data1.time[RTC_TIMETYPE_YEAR] == data2.time[RTC_TIMETYPE_YEAR]) {
		if (data1.time[RTC_TIMETYPE_DAYOFYEAR] != 0 && data2.time[RTC_TIMETYPE_DAYOFYEAR] != 0) return (data1.time[RTC_TIMETYPE_DAYOFYEAR] - data2.time[RTC_TIMETYPE_DAYOFYEAR]);
	} else {
		int count = 0;
		while (1) {
			if((data2.time[RTC_TIMETYPE_YEAR] % 4 == 0 && data2.time[RTC_TIMETYPE_YEAR] % 100 != 0) || data2.time[RTC_TIMETYPE_YEAR] % 400 == 0) {
			    /* Anno Bisestile */
				if(data2.time[RTC_TIMETYPE_DAYOFYEAR]==366) {
					data2.time[RTC_TIMETYPE_YEAR]++;
					data2.time[RTC_TIMETYPE_DAYOFYEAR]=1;
				} else {
					data2.time[RTC_TIMETYPE_DAYOFYEAR]++;
				}
			} else {
				/* Anno NON Bisestile */
				if(data2.time[RTC_TIMETYPE_DAYOFYEAR]==365) {
					data2.time[RTC_TIMETYPE_YEAR]++;
					data2.time[RTC_TIMETYPE_DAYOFYEAR]=1;
				} else {
					data2.time[RTC_TIMETYPE_DAYOFYEAR]++;
				}
			}
			count++;
//			data2.time[RTC_TIMETYPE_DAYOFYEAR]++; //VA AVANTI ALL'INFINITO! :/
			if(data1.time[RTC_TIMETYPE_DAYOFYEAR]==data2.time[RTC_TIMETYPE_DAYOFYEAR] && data1.time[RTC_TIMETYPE_YEAR]==data2.time[RTC_TIMETYPE_YEAR]) return count;
		}
	}
	return 0;
}

/**
 * @brief	main routine for example_lwip_tcpecho_freertos_18xx43xx
 * @return	Function should not exit
 */
int main(void)
{
	prvSetupHardware();

	//RIGA INUTILE
	Chip_RTC_Init();

	//ALTRA RIGA INUTILE
	//ALTRA RIGA INUTILE

	/*Set current time for RTC 8:59:20PM, 2016-04-11 MONDAY 102*/
	FullTime.time[RTC_TIMETYPE_SECOND]  	= 40;
	FullTime.time[RTC_TIMETYPE_MINUTE]  	= 59;
	FullTime.time[RTC_TIMETYPE_HOUR]    	= 8;
	FullTime.time[RTC_TIMETYPE_DAYOFMONTH] 	= 11;
	FullTime.time[RTC_TIMETYPE_DAYOFWEEK]	= 1;
	FullTime.time[RTC_TIMETYPE_DAYOFYEAR]	= 102;
	FullTime.time[RTC_TIMETYPE_MONTH]   	= 04;
	FullTime.time[RTC_TIMETYPE_YEAR]    	= 2016;

	Chip_RTC_SetFullTime(&FullTime);

	/* Set the RTC to generate an interrupt on each second */
	Chip_RTC_CntIncrIntConfig(RTC_AMR_CIIR_IMSEC, ENABLE);

	/* Enable matching for alarm for second, minute, hour fields only */
	Chip_RTC_AlarmIntConfig(RTC_AMR_CIIR_IMSEC | RTC_AMR_CIIR_IMMIN | RTC_AMR_CIIR_IMHOUR, ENABLE);

	/* Enable RTC interrupt in NVIC */
	NVIC_EnableIRQ((IRQn_Type) RTC_IRQn);

	/* Enable RTC (starts increase the tick counter and second counter register) */
	Chip_RTC_Enable(ENABLE);


	//DIFFERENZA TRA GIORNI
	IP_RTC_TIME_T data1 = FullTime, data2, diffDate;
	int nGiorniDifferenza;
	data2.time[RTC_TIMETYPE_SECOND]  	= 40;
	data2.time[RTC_TIMETYPE_MINUTE]  	= 59;
	data2.time[RTC_TIMETYPE_HOUR]    	= 7;//23;//8;
	data2.time[RTC_TIMETYPE_DAYOFMONTH] = 12;
	data2.time[RTC_TIMETYPE_DAYOFWEEK]	= 2;
	data2.time[RTC_TIMETYPE_DAYOFYEAR]	= 103;
	data2.time[RTC_TIMETYPE_MONTH]   	= 04;
	data2.time[RTC_TIMETYPE_YEAR]    	= 2017;

	diffDate = differenzaDate(data2, data1); //DA VALORI SBALLATI TRA GIORNI E ORE...

	nGiorniDifferenza = differenzaGiorni(data2, data1);

	uint8_t counter = 0;
	ptrMedic = inizializzaListaMed();
	for(counter = 0; counter < maxR; counter++){
		if (l_tabellaMedicine[counter][65]==0) { //se più volte al giorno == 0
			if (l_tabellaMedicine[counter][59]==0) {

				ptrMedic = inserisciInTestaListaMed(ptrMedic, l_tabellaMedicine[counter]);

			} else {
				if (l_tabellaMedicine[counter][58] == 0) { //DA ASSUMERE

					ptrMedic = inserisciInTestaListaMed(ptrMedic, l_tabellaMedicine[counter]);

					l_tabellaMedicine[counter][58] ++;
				} else {
					l_tabellaMedicine[counter][58] ++;
					if (l_tabellaMedicine[counter][58] = l_tabellaMedicine[counter][59]) {
						l_tabellaMedicine[counter][58] = 0;
					}

				}
			}
		} else {
			//conta quante volte prendere la medicina
			if (l_tabellaMedicine[counter][65]==1) {
				//controlla la ripetizione
				Chip_RTC_GetFullTime(&FullTime);
				IP_RTC_TIME_T dataIn;
				dataIn.time[RTC_TIMETYPE_DAYOFMONTH] = l_tabellaMedicine[counter][43];
				dataIn.time[RTC_TIMETYPE_MONTH] = l_tabellaMedicine[counter][44];
				dataIn.time[RTC_TIMETYPE_YEAR] = (l_tabellaMedicine[counter][45]<<8) | l_tabellaMedicine[counter][46];

				int oraInizio = l_tabellaMedicine[counter][51];
				int intervalloOre = l_tabellaMedicine[counter][57];
				if (FullTime.time[RTC_TIMETYPE_DAYOFMONTH]==dataIn.time[RTC_TIMETYPE_DAYOFMONTH] && FullTime.time[RTC_TIMETYPE_MONTH]==dataIn.time[RTC_TIMETYPE_MONTH] && FullTime.time[RTC_TIMETYPE_YEAR]==dataIn.time[RTC_TIMETYPE_YEAR]) {
					//E' il giorno di inizio e lascio l'ora iniziale invariata
					oraInizio = oraInizio;
				} else {
					int go = 1;
					while (go){
						if ((oraInizio-intervalloOre)>=0) {
							oraInizio = oraInizio-intervalloOre;
						} else {
							go = 0;
						}
					}
				}
				int temp = oraInizio;
				int contatore = 1;
				while ((temp+intervalloOre) < 24) {
					temp = temp+intervalloOre;
					contatore++;
				}
				int c = 0;
				IP_RTC_TIME_T oraMed;
				oraMed.time[RTC_TIMETYPE_HOUR] = oraInizio;
				oraMed.time[RTC_TIMETYPE_MINUTE] = l_tabellaMedicine[counter][52];
				oraMed.time[RTC_TIMETYPE_SECOND] = l_tabellaMedicine[counter][53];
				for (c=0; c<contatore; c++) {

					ptrMedic = inserisciInTestaListaMedOra(ptrMedic, l_tabellaMedicine[counter], oraMed);

					oraMed.time[RTC_TIMETYPE_HOUR] += intervalloOre;
				}

			} else {
				//orari custom
				int n = l_tabellaMedicine[counter][65];
				int c = 0;
				IP_RTC_TIME_T oraMed;
				oraMed.time[RTC_TIMETYPE_HOUR] = l_tabellaMedicine[counter][51];
				oraMed.time[RTC_TIMETYPE_MINUTE] = l_tabellaMedicine[counter][52];
				oraMed.time[RTC_TIMETYPE_SECOND] = l_tabellaMedicine[counter][53];

				ptrMedic = inserisciInTestaListaMedOra(ptrMedic, l_tabellaMedicine[counter], oraMed);

				for (c=0; c<n-1; c++) {
					oraMed.time[RTC_TIMETYPE_HOUR] = l_tabellaMedicine[counter][66+3*c];
					oraMed.time[RTC_TIMETYPE_MINUTE] = l_tabellaMedicine[counter][67+3*c];
					oraMed.time[RTC_TIMETYPE_SECOND] = l_tabellaMedicine[counter][68+3*c];

					ptrMedic = inserisciInTestaListaMedOra(ptrMedic, l_tabellaMedicine[counter], oraMed);

				}

			}
		}
	}
	ptrMedic = ordinamento_lista_med(ptrMedic);

	//Inizio dall'orario attuale e non considero tutti i medicinali precedenti in giornata
	IP_RTC_TIME_T oraAllarme;
	oraAllarme.time[RTC_TIMETYPE_HOUR]  = ptrMedic->oraA;
	oraAllarme.time[RTC_TIMETYPE_MINUTE]  = ptrMedic->minA;
	oraAllarme.time[RTC_TIMETYPE_SECOND]  = ptrMedic->secA;
	while (orarioDopoOrario(FullTime, oraAllarme)) {
		ptrMedic = ptrMedic->next;
		oraAllarme.time[RTC_TIMETYPE_HOUR]  = ptrMedic->oraA;
		oraAllarme.time[RTC_TIMETYPE_MINUTE]  = ptrMedic->minA;
		oraAllarme.time[RTC_TIMETYPE_SECOND]  = ptrMedic->secA;
	}

	FullTime.time[RTC_TIMETYPE_HOUR]  = oraAllarme.time[RTC_TIMETYPE_HOUR];//ptrMedic->oraA;
	FullTime.time[RTC_TIMETYPE_MINUTE]  = oraAllarme.time[RTC_TIMETYPE_MINUTE];//ptrMedic->minA;
	FullTime.time[RTC_TIMETYPE_SECOND]  = oraAllarme.time[RTC_TIMETYPE_SECOND];//ptrMedic->secA;
	Chip_RTC_SetFullAlarmTime(&FullTime);


//	CICLO FOR PER LEGGERE TUTTI I BIT DI UN BYTE
//	unsigned bit=0, n_bits = 4u, *bits = (unsigned*)malloc(sizeof(unsigned) * n_bits);
//	for (bit = 0; bit < n_bits; ++bit, input >>1)
//		bits[bit] = input & 1;


	uint8_t i;
	for (i=0;i<35;i++){
		//char t = (char)m1[4+i];
		//nomeMed[i]=t;
		arrNomiMed[i]=l_tabellaMedicine[0][4+i];
	}
	nomeMed=&arrNomiMed;



	vSemaphoreCreateBinary( xSemaDataAvail );
	vSemaphoreCreateBinary( xSemaGUIend );

/*	xTaskCreate(vTSCTask, (signed char *) "vTSCTask",
				configMINIMAL_STACK_SIZE, NULL, TASK_PRIO_TOUCHSCREEN,
				(xTaskHandle *) NULL); */

/* BASIC ONE
	xTaskCreate(vLcdTask, (signed char *) "vLCDTask",
				configMINIMAL_STACK_SIZE * 4, NULL, TASK_PRIO_LCD,
				(xTaskHandle *) NULL); */

	xTaskCreate(vLcdTaskNew, (signed char *) "vLCDTaskNew",
				configMINIMAL_STACK_SIZE * 4, NULL, TASK_PRIO_LCD,
				(xTaskHandle *) NULL);

	/* Add another thread for initializing physical interface. This
	   is delayed from the main LWIP initialization. */
/*	xTaskCreate(vSetupIFTask, (signed char *) "SetupIFx",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL); */

	xTaskCreate(vSetupPollTask, (signed char *) "SetupPoll",
				configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

/*
	xTaskCreate(vRTCTask, (signed char *) "RTCTask",
				configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 2UL),
				(xTaskHandle *) NULL);
*/

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

/**
 * @}
 */
