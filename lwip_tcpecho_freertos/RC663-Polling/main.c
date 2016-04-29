/****************************************************************************
 *   $Id:: i2c_main.c                                                       $
 *   Project: NXP LPC11xx I2C example
 *
 *   Description:
 *     This file contains main entry.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#include "board.h"
#if 0
/* Definitions of which drivers we
 * want to have enabled.
 * Just set to zero you want to
 * have disabled. */
#include <driver_config.h>
#include <target_config.h>
/* Includes the type definitions for
 * the NXP Family Microprocessors */
#include <lpc_types.h>
/* Include the GPIO (General Purpose Input/Output)
 * code header definition.
 * We need that for the microcontroller
 * to interact with the outside of the world. */
#include <lpc43xx_gpio.h>
#include <string.h>
/* Debug I/O routines for LPCXpresso projects. Uses semihosting to
 * send printf output to a window in the debugger on the PC. Calls
 * LGPL printf routine in lib_small_printf_m0 library. */
#include <debug_printf.h>
#endif
/* Configuration Headers */
/* Controls build behavior of components */
#include <ph_NxpBuild.h>
/* Status code definitions */
#undef bool
#include <ph_Status.h>

/* Reader Library Headers */
/* Generic ISO14443-3A Component of
 * Reader Library Framework */
#include <phpalI14443p3a.h>
/* Generic ISO14443-4 Component of
 * Reader Library Framework */
#include <phpalI14443p4.h>
/* Generic MIFARE(R) Ultralight Application
 * Component of Reader Library Framework */
#include <phalMful.h>
/* Generic KeyStore Component of
 * Reader Library Framework */
/* In that example we don't use any
 * key. But we need the key components
 * for some function calls and you maight
 * need it when using crypto with
 * Ultralight-C cards.
 */
#include <phKeyStore.h>

#include <phpalSli15693.h>
//#include <phalI15693.h>
//#include <phalI15693_Sw.h>
#include <phpalSli15693_Sw.h>
//#include <phalFelica.h>
#include <phpalFelica.h>
#include <phpalI14443p3b.h>

#define debug_printf( a )

/* Generic MIFARE(R) Application Component of Reader Library Framework */
#include <phalMfc.h>   

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* RTC Library */
#include "rtc_18xx_43xx.h"


/************************** PRIVATE DEFINITIONS *************************/


/************************************************************************/

#undef  PH_CHECK_SUCCESS_FCT(status,fct)
#define PH_CHECK_SUCCESS_FCT(status,fct) {(status) = (fct);}
/* We are using SSP for the internal
 * chip communication between RC663
 * and the microcontroller. */
// #define I2C_USED

#if DEBUG_MESSAGE
	#define debug_printf_msg 		debug_printf
	#define debug_printf_hex_msg	debug_printf_hex
#else
	#define debug_printf_msg(...)
	#define debug_printf_hex_msg(...)
#endif

extern xSemaphoreHandle xSemaDataAvail, xSemaGUIend;

typedef struct {
  uint8_t nameTitle[32];
  uint8_t nameFirstname[32];
  uint8_t nameMiddleName[32];
  uint8_t nameLastname[32];
  uint8_t nameSuffix[32];
  uint8_t CompName[32];
  uint8_t CompStreet[32];
  uint8_t CompZip[32];
  uint8_t CompCity[32];
  uint8_t CompState[32];
  uint8_t CompCountry[32];
  uint8_t CompPhone[32];
  uint8_t CompFax[32];
  uint8_t CompEmail[32];
  uint8_t CompURL[32];
  uint8_t ContactCellPho[32];
  uint8_t ContactNote[32];
} anagrafica_t;

anagrafica_t *data;
anagrafica_t anagrafica;

typedef struct {
	uint8_t info[4];
	struct Nodo_t *prox;
} Nodo_t;
Nodo_t *lista;
//Nodo_t nodo;
/*typedef Nodo_t *Lista;
Lista lista1;*/

extern IP_RTC_TIME_T FullTime;
IP_RTC_TIMEINDEX_T timeTypeMio; //unused now
#define maxR 5
#define maxC 100
extern uint8_t l_tabellaMedicine[maxR][maxC];

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
extern	medicinale medic;
extern medicinale *ptrMedic;
extern medicinale* inizializzaListaMed();
extern medicinale* inserisciInTestaListaMed(medicinale* l, uint8_t m[100]);
extern medicinale* ordinamento_lista_med(medicinale *pointer);

/* Set the default key for the Mifare (R) Classic cards MAD secret key  */
static /* const */ uint8_t MAD_Key[6] = {0xa0U, 0xa1U, 0xa2U, 0xa3U, 0xa4U, 0xa5U};
static /* const */ uint8_t NFC_Key[6] = {0xd3U, 0xf7U, 0xd3U, 0xf7U, 0xd3U, 0xf7U};

uint8_t cardImage[15*48] = {
		0x03,0xFF,0x01,0x71,0xC2,0x0C,0x00,0x00,0x01,0x5F,0x74,0x65,0x78,0x74,0x2F,0x78, //	...q....._text/x
		0x2D,0x76,0x43,0x61,0x72,0x64,0x42,0x45,0x47,0x49,0x4E,0x3A,0x56,0x43,0x41,0x52, //	-vCardBEGIN:VCAR
		0x44,0x0D,0x0A,0x56,0x45,0x52,0x53,0x49,0x4F,0x4E,0x3A,0x33,0x2E,0x30,0x0D,0x0A, //	D..VERSION:3.0..
		0x46,0x4E,0x3A,0x4B,0x6C,0x61,0x75,0x73,0x20,0x53,0x74,0x65,0x70,0x68,0x61,0x6E, //	FN:Klaus Stephan
		0x0D,0x0A,0x4E,0x3A,0x53,0x74,0x65,0x70,0x68,0x61,0x6E,0x3B,0x4B,0x6C,0x61,0x75, //	..N:Stephan;Klau
		0x73,0x3B,0x3B,0x3B,0x0D,0x0A,0x4F,0x52,0x47,0x3A,0x41,0x76,0x6E,0x65,0x74,0x20, //	s;;;..ORG:Avnet
		0x45,0x4D,0x47,0x20,0x47,0x6D,0x62,0x48,0x2C,0x20,0x53,0x49,0x4C,0x49,0x43,0x41, //	EMG GmbH SILICA
		0x0D,0x0A,0x41,0x44,0x52,0x3B,0x54,0x59,0x50,0x45,0x3D,0x57,0x4F,0x52,0x4B,0x3A, //	..ADR;TYPE=WORK:
		0x3B,0x3B,0x57,0x6F,0x6C,0x66,0x65,0x6E,0x62,0xFC,0x74,0x74,0x65,0x6C,0x65,0x72, //	;;Wolfenb.tteler
		0x20,0x53,0x74,0x72,0x2E,0x32,0x32,0x3B,0x42,0x72,0x61,0x75,0x6E,0x73,0x63,0x68, //	 Str.22;Braunsch
		0x77,0x65,0x69,0x67,0x3B,0x4E,0x69,0x65,0x64,0x65,0x72,0x73,0x61,0x63,0x68,0x3B, //	weig;Niedersach;
		0x33,0x38,0x31,0x30,0x32,0x3B,0x47,0x65,0x72,0x6D,0x61,0x6E,0x79,0x0D,0x0A,0x54, //	38102;Germany..T
		0x45,0x4C,0x3B,0x54,0x59,0x50,0x45,0x3D,0x57,0x4F,0x52,0x4B,0x3A,0x2B,0x34,0x39, //	EL;TYPE=WORK:+49
		0x20,0x35,0x33,0x31,0x20,0x32,0x32,0x30,0x37,0x33,0x20,0x31,0x39,0x0D,0x0A,0x54, //	 531 22073 19..T
		0x45,0x4C,0x3B,0x54,0x59,0x50,0x45,0x3D,0x48,0x4F,0x4D,0x45,0x2C,0x63,0x65,0x6C, //	EL;TYPE=HOME,cel
		0x6C,0x3A,0x2B,0x34,0x39,0x20,0x31,0x37,0x31,0x20,0x36,0x33,0x34,0x30,0x33,0x32, //	l:+49 171 634032
		0x34,0x0D,0x0A,0x54,0x45,0x4C,0x3B,0x54,0x59,0x50,0x45,0x3D,0x46,0x41,0x58,0x3A, //	4..TEL;TYPE=FAX:
		0x2B,0x34,0x39,0x20,0x34,0x31,0x35,0x31,0x20,0x38,0x36,0x39,0x39,0x37,0x37,0x0D, //	+49 4151 869977.
		0x0A,0x45,0x4D,0x41,0x49,0x4C,0x3B,0x54,0x59,0x50,0x45,0x3D,0x49,0x4E,0x54,0x45, //	.EMAIL;TYPE=INTE
		0x52,0x4E,0x45,0x54,0x3A,0x6B,0x6C,0x61,0x75,0x73,0x2E,0x73,0x74,0x65,0x70,0x68, //	RNET:klaus.steph
		0x61,0x6E,0x40,0x73,0x69,0x6C,0x69,0x63,0x61,0x2E,0x63,0x6F,0x6D,0x0D,0x0A,0x55, //	an@silica.com..U
		0x52,0x4C,0x3A,0x68,0x74,0x74,0x70,0x3A,0x2F,0x2F,0x77,0x77,0x77,0x2E,0x73,0x69, //	RL:http://www.si
		0x6C,0x69,0x63,0x61,0x2E,0x63,0x6F,0x6D,0x0D,0x0A,0x45,0x4E,0x44,0x3A,0x56,0x43, //	lica.com..END:VC
		0x41,0x52,0x44,0x0D,0x0A,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //	ARD.............


uint8_t cardMAD[32] = {
0x5A,0x01,0x03,0xE1,0x03,0xE1,0x03,0xE1,0x03,0xE1,0x03,0xE1,0x03,0xE1,0x03,0xE1,
0x03,0xE1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

uint8_t *arr[10], *pcardImage;


// Forward declarations
static void Reset_RC663_device(void);

/************************** PRIVATE VARIABLES *************************/


/************************** PRIVATE FUNCTIONS *************************/
// void SysTick_Handler (void) ;


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/

#define toHex(a)   ( ( (a)>='A' )?((a)-'A'+10):((a)-'0') )
/*
 * cerca
 */
static int vCardParse( char *src, uint8_t **dst )
{
	char    *pS;
	uint8_t *pR, *pD;
	int i, len;

	/* cerca il campo nominato */
	for( pS = src, pR = &cardImage[9]; (pR-&cardImage[0]) < sizeof( cardImage) && *pR != 0 && *pS != 0; )
	{
		if( tolower(*pS) == tolower(*pR) )
		{
			pS++;
			pR++;
		}
		else
		{
			pS = src;
			while( (++pR-&cardImage[0]) < sizeof( cardImage) && *pR != 0 && *pR != 0x0A)
				;
			pR++;
		}
	}
	/* copia i campi separati da ; */
    len = 0;
	for( i = 0, pD = dst[0]; (pR-&cardImage[0]) < sizeof( cardImage) && *pR != 0x0d && *pR != 0x0A ; )
	{
		if( *pR )
		{
			if( *pR != ';' )
			{
				if( (pD-dst[i])<32 )
                {
                    if( *pR == '=' )
                    {
                        pR++;
                        *pD  = ( toHex( *pR )<<4 );
                        pR++;
                        *pD += toHex( *pR );
                        pR++;
                        if( *pD == 0x0A )
                        {
                            *pD = 0x00;
                            if( ( pD = dst[++i] ) == NULL )
                                return(len);
                            continue;
                        } 
                        pD++;
                    }
                    else
					    *pD++ = *pR++;
                    len++;
                }
                else
                  pR++;
			}
			else
			{
				*pD = 0x00;
				if( ( pD = dst[++i] ) == NULL )
					return(len);
				pR++;
			}
		}
		else
		{
			*pD = 0x00;
			if( ( pD = dst[++i] ) == NULL )
				return(len);
		}
	}
	*pD = 0x00;
    return(len);
}


/*******************************************************************************
**   Reset RC663 device
**
**   The ports for the reset are connected trough GPIO. So we use
**   that connection to perform the reset of the RC663.
**   GPIOSetDir and GPIOSetValue are driver functions we have to use for
**   the reset.
**
**   The RC663 is reseted by a rising edge of the RESET signal.
**   Just after the reset it monitors the interface pins AD0 and AD1
**   to detect the interface to be used.
**   After an Power-On-Reset, the LPC micro has all its pins in HIGH state.
**   So it is necessary to put the RESET signal of the RC663 in LOW state
**   and then pull it in HIGH state to reset the IC
**   and to select the interface to be used.
**   Because of the importance of holding the connections until we get an
**   rising edge, we trigger short delays to be sure having stable signals
**   at the rising edge.
*******************************************************************************/
static void delay(uint32_t i) {
	while (i--) {}
}

static void Reset_RC663_device(void)
{

	Chip_SCU_PinMux(0xD,2,MD_EZI,FUNC4);	// PD_2 GPIO 6.16, reset

	/* Set RESET port pin to output */
	Chip_GPIO_WriteDirBit(0x6, 16, true);

	/* RESET signal low */
	Chip_GPIO_WritePortBit(0x6, 16, false);

	/* delay of ~1,2 ms */
	delay( 10000 );

	/* RSET signal high to reset the RC663 IC */
	Chip_GPIO_WritePortBit(0x6, 16, true);

	/* delay of ~1,2 ms */
	delay( 10000 );
    
	/* RESET signal low */
	Chip_GPIO_WritePortBit(0x6, 16, false);

	/* delay of ~1,2 ms */
	delay( 10000 );

}

#define sak_ul         0x00
#define sak_ulc	       0x00
#define sak_mini       0x09
#define sak_mfc_1k     0x08
#define sak_mfc_4k     0x18
#define sak_mfp_2k_sl1 0x08
#define sak_mfp_4k_sl1 0x18
#define sak_mfp_2k_sl2 0x10
#define sak_mfp_4k_sl2 0x11
#define sak_mfp_2k_sl3 0x20
#define sak_mfp_4k_sl3 0x20
#define sak_desfire    0x20
#define sak_jcop       0x28
#define sak_layer4     0x20

#define atqa_ul        0x4400
#define atqa_ulc       0x4400
#define atqa_mfc       0x0200
#define atqa_mfp_s     0x0400
#define atqa_mfp_x     0x4200
#define atqa_desfire   0x4403
#define atqa_jcop      0x0400
#define atqa_mini      0x0400
#define atqa_nPA       0x0800
#define atqa_mfp_s2    0x4400 // = UL, UL EV1, UL_C, PLUs, P3sr008

#define mifare_ultralight    0x01
#define mifare_ultralight_c  0x02
#define mifare_classic       0x03
#define mifare_classic_1k    0x04
#define mifare_classic_4k    0x05
#define mifare_plus          0x06
#define mifare_plus_2k_sl1   0x07
#define mifare_plus_4k_sl1   0x08
#define mifare_plus_2k_sl2   0x09
#define mifare_plus_4k_sl2   0x0A
#define mifare_plus_2k_sl3   0x0B
#define mifare_plus_4k_sl3   0x0C
#define mifare_desfire       0x0D
#define jcop                 0x0F
#define mifare_mini          0x10
#define nPA                  0x11

// Externat declarations
static void Reset_RC663_device(void);
uint32_t DetectMifare(void *pHal);
uint32_t DetectMultiMifare(void *pHal);
uint8_t DetectTag15693(void *pHal);
uint8_t DetectFelica(void *pHal);
uint8_t DetectTypB(void *pHal);
void detect_vCard( void );

void  vCardReset( void );
void  vCardDisplay( void );
void  vCardUID( char *cardType, uint8_t *bUid );
void showCardUIDList( char *cardType, Nodo_t *lista, uint8_t *nCardDetected, uint8_t *nCardPrevious );
void showTime( char *cardType, Nodo_t *lista, uint8_t *nCardDetected, uint8_t *nCardPrevious, IP_RTC_TIME_T *pTime);

char  Card_Type[40], vCardPresent;
uint8_t bUid[10], bUidPresent[100], nCardDetected, nCardPrevious;
void  GUI_DataAvailable (void);

#define	debug_printf(a)

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
int fIntervalReached;
int fAlarmTimeMatched;
int On0, On1;
/*extern */uint8_t tagRimosso[4], tagAggiunto[4];

/*******************************************************************************
**   Main Function
*******************************************************************************/
const uint8_t ASCII[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };


/* Gets and shows the current time and date */
//static void showTime(IP_RTC_TIME_T *pTime)
//{
//	DEBUGOUT("Time: %.2d:%.2d:%.2d %.2d/%.4d\r\n", pTime->time[RTC_TIMETYPE_HOUR],
//			 pTime->time[RTC_TIMETYPE_MINUTE],
//			 pTime->time[RTC_TIMETYPE_SECOND],
//			 pTime->time[RTC_TIMETYPE_MONTH],
//			 pTime->time[RTC_TIMETYPE_YEAR]);
//}


void RTC_IRQHandler(void)
{
	uint32_t sec;

	/* Toggle heart beat LED for each second field change interrupt */
	if (Chip_RTC_GetIntPending(RTC_INT_COUNTER_INCREASE)) {
		/* Clear pending interrupt */
		Chip_RTC_ClearIntPending(RTC_INT_COUNTER_INCREASE);
		//On0 = (bool) !On0;
		switch (On0){
		case 0: On0=1;
		case 1: On0=0;
		default: On0=0;
		}
		Board_LED_Set(0, On0);
	}

	/* display timestamp every second */
	sec = Chip_RTC_GetTime(RTC_TIMETYPE_SECOND);
//	if (!(sec % 5)) {
//		fIntervalReached = 1;	/* set flag for background */
//		if (fAlarmTimeMatched) {
//			fAlarmTimeMatched = 0;
//		}
//	}

	/* Check for alarm match */
	if (Chip_RTC_GetIntPending(RTC_INT_ALARM)) {
		/* Clear pending interrupt */
		Chip_RTC_ClearIntPending(RTC_INT_ALARM);
		fAlarmTimeMatched = 1;	/* set alarm handler flag */
	}
}

Nodo_t* inizializza(){
	return NULL;
}
Nodo_t* inserisciInTesta(Nodo_t* l, uint8_t buid[4]){
	Nodo_t *temp;
	temp=malloc(sizeof(Nodo_t));
	//temp->info = buid;
	strcpy(temp->info,buid);
	temp->prox = l;
	return temp;
}

void assegnamento_uint8_t(Nodo_t *dest, Nodo_t *sur){
	int i;
	for (i=0; i<4; i++)
		dest->info[i] = sur->info[i];
}

Nodo_t* ordinamento_lista(Nodo_t *pointer){

	if (pointer && pointer->prox) {
		int k;

		do {
			Nodo_t *punt = pointer;
			k=0;

			while (punt->prox) {
				Nodo_t *punt1 = punt;
				punt = punt->prox;
				int ret = memcmp((punt->info),(punt1->info),4);
				if (ret>0) {
					Nodo_t *temp;
					temp = malloc(sizeof(Nodo_t));
					assegnamento_uint8_t(temp,punt1);
					assegnamento_uint8_t(punt1,punt);
					assegnamento_uint8_t(punt,temp);
					k=1;
				}
			}
		} while (k != 0);
	}
	return pointer;

}

void tag_aggiunto(Nodo_t *listaPrev, Nodo_t *lista){
	/*Capire il tag AGGIUNTO*/
	Nodo_t *tLP = listaPrev;
	Nodo_t *tL = lista;
	if (listaPrev != NULL){
		while (lista != NULL ){
			if ((lista->info[0]==listaPrev->info[0]) && (lista->info[1]==listaPrev->info[1]) && (lista->info[2]==listaPrev->info[2]) && (lista->info[3]==listaPrev->info[3])) {
				lista = lista->prox;
				listaPrev = tLP;
			} else {
				if (listaPrev->prox != NULL){
					listaPrev = listaPrev->prox;
				} else {
					break;
				}
			}
		}
	}
	/*HO CAPITO QUAL'E' IL TAG AGGIUNTO*/
	tagAggiunto[0] = lista->info[0];
	tagAggiunto[1] = lista->info[1];
	tagAggiunto[2] = lista->info[2];
	tagAggiunto[3] = lista->info[3];
	/*Riparto dalla testa delle liste*/
	listaPrev = tLP;
	lista = tL;
	//return tagAggiunto;
}

void tag_rimosso(Nodo_t *listaPrev, Nodo_t *lista){
	/*Capire il tag RIMOSSO*/

	/*COMPARAZIONE LISTE*/
	/*il primo tag lo comparo con tutti, se non trovo un'uguaglianza "break" perchè è il tag in più che è stato rimosso*/
	Nodo_t *tLP = listaPrev;
	Nodo_t *tL = lista;

	while (listaPrev != NULL ){
		if ((listaPrev->info[0]==lista->info[0]) && (listaPrev->info[1]==lista->info[1]) && (listaPrev->info[2]==lista->info[2]) && (listaPrev->info[3]==lista->info[3])) {
			lista = tL;
			listaPrev = listaPrev->prox;
		} else {
			if (lista->prox != NULL){
				lista = lista->prox;
			} else {
				break;
			}
		}
	}
	/*HO CAPITO QUAL'E' IL TAG RIMOSSO*/
	tagRimosso[0] = listaPrev->info[0];
	tagRimosso[1] = listaPrev->info[1];
	tagRimosso[2] = listaPrev->info[2];
	tagRimosso[3] = listaPrev->info[3];
	/*Riparto dalla testa delle liste*/
	listaPrev = tLP;
	lista = tL;
	//return tagRimosso;
}


int vSetupPollTask (void)
{
    /* BFL (Basic Function Library)
     * data parameter storage */
    phbalReg_Stub_DataParams_t balReader;
    phhalHw_Rc663_DataParams_t halReader;
    phStatus_t status;
    uint8_t bHalBufferReader[0x40];
    void    *pHal;


#if 0
    UART_CFG_Type UART0_Config;
    UART_CFG_Type UART3_Config;

    scu_pinmux(0xE ,14 , MD_PUP, FUNC4); 	// P7.14 STATUS_LED SERIZ2
    GPIO_SetDir(LED1_PORT, LED1_MASK, 1);
    GPIO_SetValue( LED1_PORT, LED1_MASK);
    
    freq = CGU_GetPCLKFrequency(CGU_PERIPHERAL_M4CORE);
    // M3Frequency is automatically set when SetClock(BASE_M3_CLK... was called.
    SysTick_Config(freq/1000);  		   // Generate interrupt @ 1000 Hz
        
    /*
      set UART to test
    */
    scu_pinmux(0xF,10,MD_PDN,FUNC1);	// PF_10 TXD0
    scu_pinmux(0xF,11,MD_EZI,FUNC1);	// PF_11 RXD0
    UART_ConfigStructInit( &UART0_Config );
    UART_Init( LPC_USART0, &UART0_Config );
    UART_TxCmd(LPC_USART0, ENABLE );
    UART_Send( LPC_USART0, "RSR888", 6, BLOCKING);
    /* end test UART 0 */
    
    /*
      set test SIM uart
    */
    scu_pinmux(0xC,13,MD_PDN,FUNC4);	// PC_12 SIM_RESET ( GPIO6.12 )
    GPIO_SetDir(6, (1<<12), 1);
    GPIO_ClearValue(6, (1<<12));           // reset
    scu_pinmux(0xF,5,MD_PDN,FUNC1);	// PF_5  CLK3
    scu_pinmux(0x4,1,MD_EZI,FUNC6);	// P4_1  TXD3/RXD3
    
    UART_ConfigStructInit( &UART3_Config );
    UART3_Config.Baud_rate = 9600*372/16; // to set divisor
    UART_Init( LPC_USART3, &UART3_Config );
    LPC_USART3->SCICTRL =
    ( 1 ) |                 /* enable ISO7816 */
    ((1 & 0xff) << 8) |     /* guard bits */
    ((0 & 0x7) << 5);       /* retries */
    LPC_USART3->OSR = 371 << 4;
    UART_TxCmd(LPC_USART3, ENABLE );
    msec = 32;  while(msec);
    GPIO_SetValue(6, (1<<12));       // reset off
    bLength = 0;
    msec = 32;
    while( msec ) {
	if( LPC_USART3->LSR & UART_LSR_RDR ) {
                i = UART_ReceiveByte(LPC_USART3);
	        bBufferReader[bLength++] = ASCII[( i >> 4 )& 0x0f ];
		bBufferReader[bLength++] = ASCII[i & 0x0f ];
	}
    }
    UART_Send(LPC_USART0, bBufferReader, bLength, BLOCKING );
    /* end test SIM Uart */
#endif

	/* Perform a hardware reset */
    Reset_RC663_device();
    memset( &halReader, 0x00, sizeof( halReader) );

    debug_printf("\nStart");

    /* Initialize the Reader BAL (Bus Abstraction Layer) component */
    phbalReg_Stub_Init(&balReader, sizeof(phbalReg_Stub_DataParams_t));

    /* Initialize the Reader HAL (Hardware Abstraction Layer) component */
    status = phhalHw_Rc663_Init(
        &halReader,
        sizeof(phhalHw_Rc663_DataParams_t),
        &balReader,
        0,
        bHalBufferReader,
        sizeof(bHalBufferReader),
        bHalBufferReader,
        sizeof(bHalBufferReader));

    /* Set the parameter to use the SPI interface */
    halReader.bBalConnectionType = PHHAL_HW_BAL_CONNECTION_SPI;

    /* Set the generic pointer */
    pHal = &halReader;
//    vCardReset( );
    /*******************************************************************************
     * Lets start the polling
     */
		  lista = inizializza();
		  Nodo_t *listaPrev;


    debug_printf("/****** Begin Polling ******/");
nCardDetected=0;
	for(;;)
	{

		Chip_RTC_GetFullTime(&FullTime);
		if ((FullTime.time[RTC_TIMETYPE_HOUR] == 0) && (FullTime.time[RTC_TIMETYPE_MINUTE] == 0) && (FullTime.time[RTC_TIMETYPE_SECOND] == 0)){
			if (ptrMedic != NULL) ptrMedic=NULL;
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
								int stop = 1;
								while (stop){
									if ((oraInizio-intervalloOre)>=0) {
										oraInizio = oraInizio-intervalloOre;
									} else {
										stop = 0;
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

			if ((ptrMedic->oraA == 0) && (ptrMedic->minA == 0) && (ptrMedic->secA == 0)) {
				fAlarmTimeMatched = 1;
			} else {
			FullTime.time[RTC_TIMETYPE_HOUR]  = ptrMedic->oraA;
			FullTime.time[RTC_TIMETYPE_MINUTE]  = ptrMedic->minA;
			FullTime.time[RTC_TIMETYPE_SECOND]  = ptrMedic->secA;
			Chip_RTC_SetFullAlarmTime(&FullTime);
			}

		}

vTaskDelay(80);

//medic.tag;
nCardPrevious=nCardDetected;
listaPrev = inizializza();
listaPrev = lista;
lista = inizializza();
nCardDetected=0;
		vCardPresent = 0;
		Board_LED_Set( 0, 1 );
		/*
		 * Detecting Mifare cards works on the Blueboards RC663, MFRC630 and MFRC631.
		 * On the Blueboard SLRC610 this if() will just be false.
		 */

		if (DetectMultiMifare(pHal))
		{
				Board_LED_Set( 0, 0 );
//				detect_vCard();
				/*if (DetectMultiMifare(pHal))
				  						{
				  									Board_LED_Set( 0, 0 );
				  									detect_vCard();
				  						}*/

		  /* reset the IC  */
		  //PH_CHECK_SUCCESS_FCT(status, phhalHw_Rc663_Cmd_SoftReset(pHal));
		}
		else
			Card_Type[0] = 0x00;

		/*SORT LISTE*/
		if (lista != NULL)
			lista = ordinamento_lista(lista);
		if (listaPrev != NULL)
			listaPrev = ordinamento_lista(listaPrev);

		if (nCardDetected<nCardPrevious){
			/*Capire il tag RIMOSSO*/
			tag_rimosso(listaPrev, lista);

		} else {
			if (nCardDetected>nCardPrevious){
				/*Capire il tag AGGIUNTO*/
				tag_aggiunto(listaPrev, lista);
			}
		}
		vTaskDelay(10);
		xSemaphoreGive( xSemaDataAvail );
		vTaskDelay(10);
		while( xSemaphoreTake(xSemaGUIend, portMAX_DELAY ) != pdTRUE );

	}
}

phKeyStore_Rc663_DataParams_t Rc663keyStore;
phalMfc_Sw_DataParams_t  alMfc;
phpalI14443p4_Sw_DataParams_t I14443p4;
phpalMifare_Sw_DataParams_t palMifare;
phpalI14443p3a_Sw_DataParams_t I14443p3a;


uint32_t DetectMifare(void *pHal)
{
//  phpalI14443p4_Sw_DataParams_t I14443p4;
//  phpalMifare_Sw_DataParams_t palMifare;
//  phpalI14443p3a_Sw_DataParams_t I14443p3a;

  phStatus_t status;
  uint8_t cryptoEnc[8];
  uint8_t cryptoRng[8];

//  phKeyStore_Rc663_DataParams_t Rc663keyStore;
  phalMful_Sw_DataParams_t alMful;

//  uint8_t bUid[10];
  uint8_t bLength;
  uint8_t bMoreCardsAvailable;
  uint32_t sak_atqa = 0;
  uint8_t pAtqa[2];
  uint8_t bSak[1];
  uint16_t detected_card = 0xFFFF;



  /* Initialize the 14443-3A PAL (Protocol Abstraction Layer) component */
  PH_CHECK_SUCCESS_FCT(status, phpalI14443p3a_Sw_Init(&I14443p3a,
              sizeof(phpalI14443p3a_Sw_DataParams_t), pHal));

  /* Initialize the 14443-4 PAL component */
  PH_CHECK_SUCCESS_FCT(status, phpalI14443p4_Sw_Init(&I14443p4,
              sizeof(phpalI14443p4_Sw_DataParams_t), pHal));

  /* Initialize the Mifare PAL component */
  PH_CHECK_SUCCESS_FCT(status, phpalMifare_Sw_Init(&palMifare,
              sizeof(phpalMifare_Sw_DataParams_t), pHal, &I14443p4));

  /* Initialize the keystore component */
  PH_CHECK_SUCCESS_FCT(status, phKeyStore_Rc663_Init(&Rc663keyStore,
              sizeof(phKeyStore_Rc663_DataParams_t), pHal));

  /* Initialize the Mifare (R) Classic AL component - set NULL because
     * the keys are loaded in E2 by the function */
  /* phKeyStore_SetKey */
  PH_CHECK_SUCCESS_FCT(status, phalMfc_Sw_Init(&alMfc,
	      sizeof(phalMfc_Sw_DataParams_t), &palMifare, /*&Rc663keyStore*/ NULL));

      /* Initialize Ultralight(-C) AL component */
  PH_CHECK_SUCCESS_FCT(status, phalMful_Sw_Init(&alMful,
              sizeof(phalMful_Sw_DataParams_t), &palMifare, &Rc663keyStore,
              &cryptoEnc, &cryptoRng));

  /* Reset the RF field */
  PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldReset(pHal));

  /* Apply the type A protocol settings
   * and activate the RF field. */
  PH_CHECK_SUCCESS_FCT(status,
              phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A));

  /* Empty the pAtqa */
  memset(pAtqa, '\0', 2);
  status = phpalI14443p3a_RequestA(&I14443p3a, pAtqa);

  /* Reset the RF field */
  PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldReset(pHal));

  /* Empty the bSak */
  memset(bSak, '\0', 1);

  /* Activate the communication layer part 3
   * of the ISO 14443A standard. */
  status = phpalI14443p3a_ActivateCard(&I14443p3a,
                        NULL, 0x00, bUid, &bLength, bSak, &bMoreCardsAvailable);
  phpalI14443p3a_HaltA(&I14443p3a);
//  debug_printf_msg("SAK:");
//  debug_printf_hex_msg(&bSak, 1);
//  debug_printf_msg("ATQA:");
//  debug_printf_hex_msg(&pAtqa, 2);
//  _debug_printf_flush();

  sak_atqa = bSak[0] << 24 | pAtqa[0] << 8 | pAtqa[1];
  sak_atqa &= 0xFFFF0FFF;

  // Detect mini or classic
  switch (sak_atqa)
  {
    case sak_mfc_1k << 24 | atqa_mfc:
      strcpy( Card_Type,"MIFARE Classic");
      detected_card &= mifare_classic;
      break;
    case sak_mfc_4k << 24 | atqa_mfc:
      strcpy( Card_Type,"MIFARE Classic");
      detected_card &= mifare_classic;
      break;
    case sak_mfp_2k_sl1 << 24 | atqa_mfp_s:
      strcpy( Card_Type,"MIFARE Classic");
      detected_card &= mifare_classic;
      break;
    case sak_mini << 24 | atqa_mini:
      strcpy( Card_Type,"MIFARE Mini");
      detected_card &= mifare_mini;
      break;
    case sak_mfp_4k_sl1 << 24 | atqa_mfp_s:
      strcpy( Card_Type,"MIFARE Classic");
      detected_card &= mifare_classic;
      break;
    case sak_mfp_2k_sl1 << 24 | atqa_mfp_x:
      strcpy( Card_Type,"MIFARE Classic");
      detected_card &= mifare_classic;
      break;
    case sak_mfp_4k_sl1 << 24 | atqa_mfp_x:
      strcpy( Card_Type,"MIFARE Classic");
      detected_card &= mifare_classic;
      break;
    default:
      break;
  }

  if (detected_card == 0xFFFF)
  {
    sak_atqa = bSak[0] << 24 | pAtqa[0] << 8 | pAtqa[1];
    switch (sak_atqa)
    {
      case sak_ul << 24 | atqa_ul:
        strcpy( Card_Type,"MIFARE Ultralight");
        detected_card &= mifare_ultralight;
        break;
      case sak_mfp_2k_sl2 << 24 | atqa_mfp_s:
        strcpy( Card_Type,"MIFARE Plus");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_2k_sl3 << 24 | atqa_mfp_s:
        strcpy( Card_Type,"MIFARE Plus");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_4k_sl2 << 24 | atqa_mfp_s:
        strcpy( Card_Type,"MIFARE Plus");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_2k_sl2 << 24 | atqa_mfp_x:
        strcpy( Card_Type,"MIFARE Plus");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_2k_sl3 << 24 | atqa_mfp_x:
        strcpy( Card_Type,"MIFARE Plus");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_4k_sl2 << 24 | atqa_mfp_x:
        strcpy( Card_Type,"MIFARE Plus");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_2k_sl3 << 24 | atqa_mfp_s2:
        // printf("Card_Type: MIFARE Plus 2K \"s\"\n");
        strcpy( Card_Type,"MIFARE Plus 2K \"s\" ");
        detected_card &= mifare_plus;
      break;
      case sak_desfire << 24 | atqa_desfire:
        strcpy( Card_Type,"MIFARE DESFire");
        detected_card &= mifare_desfire;
        break;
      case sak_jcop << 24 | atqa_jcop:
        strcpy( Card_Type,"JCOP");
        detected_card &= jcop;
        break;
      case sak_layer4 << 24 | atqa_nPA:
		strcpy( Card_Type,"German eID (neuer Personalausweis)");
		detected_card &= nPA;
		break;
      default:
        return 0;
    }
  }
  #ifdef DEBUG
//    _debug_printf_flush();
  #endif

  return detected_card;
}

uint32_t DetectMultiMifare(void *pHal)
{
	//  phpalI14443p4_Sw_DataParams_t I14443p4;
	//  phpalMifare_Sw_DataParams_t palMifare;
	//  phpalI14443p3a_Sw_DataParams_t I14443p3a;

	  phStatus_t status;
	  uint8_t cryptoEnc[8];
	  uint8_t cryptoRng[8];

	//  phKeyStore_Rc663_DataParams_t Rc663keyStore;
	  phalMful_Sw_DataParams_t alMful;

	//  uint8_t bUid[10];
	  uint8_t bLength;
	  uint8_t bMoreCardsAvailable;
	  uint32_t sak_atqa = 0;
	  uint8_t pAtqa[2];
	  uint8_t bSak[1];
	  uint16_t detected_card = 0xFFFF;



	  /* Initialize the 14443-3A PAL (Protocol Abstraction Layer) component */
	  PH_CHECK_SUCCESS_FCT(status, phpalI14443p3a_Sw_Init(&I14443p3a,
	              sizeof(phpalI14443p3a_Sw_DataParams_t), pHal));

	  /* Initialize the 14443-4 PAL component */
	  PH_CHECK_SUCCESS_FCT(status, phpalI14443p4_Sw_Init(&I14443p4,
	              sizeof(phpalI14443p4_Sw_DataParams_t), pHal));

	  /* Initialize the Mifare PAL component */
	  PH_CHECK_SUCCESS_FCT(status, phpalMifare_Sw_Init(&palMifare,
	              sizeof(phpalMifare_Sw_DataParams_t), pHal, &I14443p4));

	  /* Initialize the keystore component */
	  PH_CHECK_SUCCESS_FCT(status, phKeyStore_Rc663_Init(&Rc663keyStore,
	              sizeof(phKeyStore_Rc663_DataParams_t), pHal));

	  /* Initialize the Mifare (R) Classic AL component - set NULL because
	     * the keys are loaded in E2 by the function */
	  /* phKeyStore_SetKey */
	  PH_CHECK_SUCCESS_FCT(status, phalMfc_Sw_Init(&alMfc,
		      sizeof(phalMfc_Sw_DataParams_t), &palMifare, /*&Rc663keyStore*/ NULL));

	      /* Initialize Ultralight(-C) AL component */
	  PH_CHECK_SUCCESS_FCT(status, phalMful_Sw_Init(&alMful,
	              sizeof(phalMful_Sw_DataParams_t), &palMifare, &Rc663keyStore,
	              &cryptoEnc, &cryptoRng));

	  /* Reset the RF field */
	  PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldReset(pHal));

	  /* Apply the type A protocol settings
	   * and activate the RF field. */
	  PH_CHECK_SUCCESS_FCT(status,
	              phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A));

	  /* Empty the pAtqa */
	  memset(pAtqa, '\0', 2);
	  status = phpalI14443p3a_RequestA(&I14443p3a, pAtqa);

	  /* Reset the RF field */
	  PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldReset(pHal));

	  /* Empty the bSak */
	  memset(bSak, '\0', 1);

	  /* Activate the communication layer part 3
	   * of the ISO 14443A standard. */
	  status = phpalI14443p3a_ActivateCard(&I14443p3a,
	                        NULL, 0x00, bUid, &bLength, bSak, &bMoreCardsAvailable);

	  int i=0;
	  while (i<99){
	  	bUidPresent[i] = 0;
	  	i+=1;
	  }

	  if (status!=513){
		  lista = inserisciInTesta(lista,bUid);
		  nCardDetected+=1;

	  } else {
		  int i=0;
		  while (i<10){
		  	bUid[i] = 0;
		  	i+=1;
		  }
		  lista = inserisciInTesta(lista,bUid);
		  nCardDetected=0;
	  }
	  //bUid= 0;
	  while(status==0){
		  phpalI14443p3a_HaltA(&I14443p3a);
	  	  status = phpalI14443p3a_ActivateCard(&I14443p3a,
	  	                        NULL, 0x00, bUid, &bLength, bSak, &bMoreCardsAvailable);
	  	  if (status==0) {
			  lista = inserisciInTesta(lista,bUid);
	  		  nCardDetected+=1;
	  	  }
	  	  //bUid= 0;
	  }

	//  debug_printf_msg("SAK:");
	//  debug_printf_hex_msg(&bSak, 1);
	//  debug_printf_msg("ATQA:");
	//  debug_printf_hex_msg(&pAtqa, 2);
	//  _debug_printf_flush();

	  sak_atqa = bSak[0] << 24 | pAtqa[0] << 8 | pAtqa[1];
	  sak_atqa &= 0xFFFF0FFF;

	  // Detect mini or classic
	  switch (sak_atqa)
	  {
	    case sak_mfc_1k << 24 | atqa_mfc:
	      strcpy( Card_Type,"MIFARE Classic");
	      detected_card &= mifare_classic;
	      break;
	    case sak_mfc_4k << 24 | atqa_mfc:
	      strcpy( Card_Type,"MIFARE Classic");
	      detected_card &= mifare_classic;
	      break;
	    case sak_mfp_2k_sl1 << 24 | atqa_mfp_s:
	      strcpy( Card_Type,"MIFARE Classic");
	      detected_card &= mifare_classic;
	      break;
	    case sak_mini << 24 | atqa_mini:
	      strcpy( Card_Type,"MIFARE Mini");
	      detected_card &= mifare_mini;
	      break;
	    case sak_mfp_4k_sl1 << 24 | atqa_mfp_s:
	      strcpy( Card_Type,"MIFARE Classic");
	      detected_card &= mifare_classic;
	      break;
	    case sak_mfp_2k_sl1 << 24 | atqa_mfp_x:
	      strcpy( Card_Type,"MIFARE Classic");
	      detected_card &= mifare_classic;
	      break;
	    case sak_mfp_4k_sl1 << 24 | atqa_mfp_x:
	      strcpy( Card_Type,"MIFARE Classic");
	      detected_card &= mifare_classic;
	      break;
	    default:
	      break;
	  }

	  if (detected_card == 0xFFFF)
	  {
	    sak_atqa = bSak[0] << 24 | pAtqa[0] << 8 | pAtqa[1];
	    switch (sak_atqa)
	    {
	      case sak_ul << 24 | atqa_ul:
	        strcpy( Card_Type,"MIFARE Ultralight");
	        detected_card &= mifare_ultralight;
	        break;
	      case sak_mfp_2k_sl2 << 24 | atqa_mfp_s:
	        strcpy( Card_Type,"MIFARE Plus");
	        detected_card &= mifare_plus;
	        break;
	      case sak_mfp_2k_sl3 << 24 | atqa_mfp_s:
	        strcpy( Card_Type,"MIFARE Plus");
	        detected_card &= mifare_plus;
	        break;
	      case sak_mfp_4k_sl2 << 24 | atqa_mfp_s:
	        strcpy( Card_Type,"MIFARE Plus");
	        detected_card &= mifare_plus;
	        break;
	      case sak_mfp_2k_sl2 << 24 | atqa_mfp_x:
	        strcpy( Card_Type,"MIFARE Plus");
	        detected_card &= mifare_plus;
	        break;
	      case sak_mfp_2k_sl3 << 24 | atqa_mfp_x:
	        strcpy( Card_Type,"MIFARE Plus");
	        detected_card &= mifare_plus;
	        break;
	      case sak_mfp_4k_sl2 << 24 | atqa_mfp_x:
	        strcpy( Card_Type,"MIFARE Plus");
	        detected_card &= mifare_plus;
	        break;
	      case sak_mfp_2k_sl3 << 24 | atqa_mfp_s2:
	        // printf("Card_Type: MIFARE Plus 2K \"s\"\n");
	        strcpy( Card_Type,"MIFARE Plus 2K \"s\" ");
	        detected_card &= mifare_plus;
	      break;
	      case sak_desfire << 24 | atqa_desfire:
	        strcpy( Card_Type,"MIFARE DESFire");
	        detected_card &= mifare_desfire;
	        break;
	      case sak_jcop << 24 | atqa_jcop:
	        strcpy( Card_Type,"JCOP");
	        detected_card &= jcop;
	        break;
	      case sak_layer4 << 24 | atqa_nPA:
			strcpy( Card_Type,"German eID (neuer Personalausweis)");
			detected_card &= nPA;
			break;
	      default:
	        return 0;
	    }
	  }
	  #ifdef DEBUG
	//    _debug_printf_flush();
	  #endif

	  return detected_card;
}

uint8_t DetectTag15693(void *pHal)
{
  phStatus_t status;
  uint8_t bMask[PHPAL_SLI15693_UID_LENGTH];
  uint8_t Dsfid;
//  uint8_t bUid[10];
  uint8_t bMoreCardsAvailable;
  phpalSli15693_Sw_DataParams_t palSli15693;
//  phalI15693_Sw_DataParams_t alI15693;

  /* init. 15693 pal */
  PH_CHECK_SUCCESS_FCT(status, phpalSli15693_Sw_Init(&palSli15693,
                                    sizeof(phpalSli15693_Sw_DataParams_t), pHal));

  /* Apply the 15693 protocol settings */
  PH_CHECK_SUCCESS_FCT(status, phhalHw_ApplyProtocolSettings(pHal,
                                                     PHHAL_HW_CARDTYPE_ISO15693));

  /* Activate 15693 tags */
  memset(bMask, 0, sizeof(bMask));
  status = phpalSli15693_Sw_ActivateCard(&palSli15693,
           PHPAL_SLI15693_ACTIVATE_DEFAULT,
           (PHPAL_SLI15693_FLAG_NBSLOTS | PHPAL_SLI15693_FLAG_DATA_RATE),
           0, bMask, 0, &Dsfid, bUid, &bMoreCardsAvailable);

  if (PH_ERR_SUCCESS == status)
  {
    strcpy( Card_Type,"ISO-15693 tag");
    return true;
  }

  return false;
}

uint8_t DetectFelica(void *pHal)
{
  phStatus_t status;
  uint8_t bBufferReader[0x60];
  uint8_t bMoreCardsAvailable;
  phpalFelica_Sw_DataParams_t palFelica;
  uint8_t bLength;
  uint8_t pIDmPMmOut[16];
  uint8_t pLenIDmPMmOut;

  uint8_t pIDmPMm[16], bIDmPMmLength = 0;
  uint8_t bNumTimeSlots = PHPAL_FELICA_NUMSLOTS_1;
  uint8_t pSysCode[2] = {0xFF, 0xFF};

  PH_CHECK_SUCCESS_FCT(status, phpalFelica_Sw_Init(&palFelica,
                                      sizeof(phpalFelica_Sw_DataParams_t), pHal));

  PH_CHECK_SUCCESS_FCT(status, phhalHw_ApplyProtocolSettings(pHal,
                                                       PHHAL_HW_CARDTYPE_FELICA));

  PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldReset(pHal));

  status = phpalFelica_ActivateCard(&palFelica,
                  pIDmPMm, bIDmPMmLength, pSysCode, bNumTimeSlots, bBufferReader,
                  &bLength, &bMoreCardsAvailable);

  phpalFelica_GetSerialNo(&palFelica, pIDmPMmOut, &pLenIDmPMmOut);

//  debug_printf_hex_msg(&pIDmPMmOut, 16);



  if (PH_ERR_SUCCESS == status)
  {
    if (pIDmPMmOut[0] == 0x01 && pIDmPMmOut[1] == 0xFE)
	strcpy( Card_Type,"NFC Forum compliant device");
    else
	strcpy( Card_Type,"JIS X 6319-4 compatible card");
    return true;
  }

  return false;
}

uint8_t DetectTypB(void *pHal)
{
  phStatus_t status = 0;
  phpalI14443p3b_Sw_DataParams_t  palI14443p3B;
  uint8_t pPupi[4];
  uint8_t bPupiLength = 0;
  uint8_t bNumSlots = 0;
  uint8_t bAfi = 0, pAtqb[13], bAtqbLen = 0, bExtAtqb = 0;
  uint8_t bCid = 0, bFsdi = 0, bDri = 0, bDsi = 0;
  uint8_t bMbli = 0, bMoreCardsAvaliable = 0;

  PH_CHECK_SUCCESS_FCT(status, phpalI14443p3b_Sw_Init(&palI14443p3B, sizeof(palI14443p3B), pHal));

  PH_CHECK_SUCCESS_FCT(status, phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443B));

  PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldReset(pHal));

  status = phpalI14443p3b_ActivateCard(&palI14443p3B, pPupi, bPupiLength, bNumSlots, bAfi,
                                       bExtAtqb, bFsdi, bCid, bDri, bDsi, pAtqb, &bAtqbLen, &bMbli,
                                       &bMoreCardsAvaliable);

  if (PH_ERR_SUCCESS == status)
  {
    strcpy( Card_Type,"Type B card");
    return true;
  }

  return false;
}

void detect_vCard( void )
{
  phStatus_t status = 0;
  uint8_t i;

  vCardPresent = 0;
  /* Mifare Classic card, set Key Store, store MAD key at index 2 */
  PH_CHECK_SUCCESS_FCT(status, phKeyStore_SetKey(&Rc663keyStore, 0, 0,
                               PH_KEYSTORE_KEY_TYPE_MIFARE, &MAD_Key[0], 0));
            
  /* Mifare Classic card, set Key Store, store NFC_Key at intex 1 */
  PH_CHECK_SUCCESS_FCT(status, phKeyStore_SetKey(&Rc663keyStore, 1, 0,
				PH_KEYSTORE_KEY_TYPE_MIFARE, &NFC_Key[0], 0));
            
  /* Mifare Classic card, send authentication for sector 0, with MAD_KEY idx 0 */
  PH_CHECK_SUCCESS_FCT(status, phalMfc_Authenticate(&alMfc, 0,
			        PHHAL_HW_MFC_KEYA, 0, 0, bUid, 4));
                
  debug_printf_msg("\n**** Authentication successful");
                
  memset( cardMAD, 0x00, sizeof( cardMAD) );
            
  /* Read Block 1 */
  PH_CHECK_SUCCESS_FCT(status, phalMfc_Read(&alMfc, 1, &cardMAD[0]));
  /* Read Block 2 */
  PH_CHECK_SUCCESS_FCT(status, phalMfc_Read(&alMfc, 2, &cardMAD[16]));
            
  /* Read CARD */
  
  for( pcardImage = &cardImage[0], i = 4; i < 16*4; i+=4 )
  {
    if( cardMAD[i/2] == 0x03 && cardMAD[i/2+1] == 0xE1 )
    {
    /* Mifare Classic card, send authentication for sector i, with NFC_Key stored at idx 1 */
     if( PH_ERR_SUCCESS != phalMfc_Authenticate(&alMfc, i,
				PHHAL_HW_MFC_KEYA, 1, 0, bUid, 4)) break;
                  
    /* Prepare data for Mifare Read blobk i, using NFC_Key */
    PH_CHECK_SUCCESS_FCT(status, phalMfc_Read(&alMfc, i, pcardImage));
    pcardImage+=16;
                    
     /* Prepare data for Mifare Read blobk i+1, using NFC_key*/
    PH_CHECK_SUCCESS_FCT(status, phalMfc_Read(&alMfc, i+1, pcardImage))
    pcardImage+=16;
                    
    /* Prepare data for Mifare Read blobk i+2, using NFC_key*/
    PH_CHECK_SUCCESS_FCT(status, phalMfc_Read(&alMfc, i+2, pcardImage));
    pcardImage+=16;
    }
  }   // for
            
  data = &anagrafica;
            
  /* interpret vCARD structure, if any */
              
  if( pcardImage != &cardImage[0] )
   {
    arr[0] = data->nameLastname;
    arr[1] = data->nameFirstname;
    arr[2] = data->nameMiddleName;
    arr[3] = data->nameTitle;
    arr[4] = NULL;
    vCardParse( "N:", arr );
    
    arr[0] = data->CompEmail;
    arr[1] = NULL;
    vCardParse( "EMAIL;TYPE=INTERNET:", arr );
    
    arr[0] = data->CompURL;
    arr[1] = NULL;
    vCardParse( "URL:", arr );
    
    arr[0] = data->CompPhone;
    arr[1] = NULL;
    if( !vCardParse( "TEL;TYPE=WORK:", arr ) )
        vCardParse( "TEL;WORK:", arr );
    
    arr[0] = data->CompFax;
    arr[1] = NULL;
    if( !vCardParse( "TEL;TYPE=FAX:", arr ) )
        vCardParse( "TEL;WORK;FAX:", arr );
    
    arr[0] = data->ContactCellPho;
    arr[1] = NULL;
    if( !vCardParse( "TEL;TYPE=HOME,cell:", arr ) )
      if( !vCardParse( "TEL;TYPE=WORK,cell:", arr ) )
         vCardParse( "TEL;cell:", arr );
    
    arr[0] = data->CompName;
    arr[1] = NULL;
    vCardParse( "ORG:", arr );
    
    arr[0] = data->CompStreet;
    arr[1] = data->CompStreet;
    arr[2] = data->CompStreet;
    arr[3] = data->CompCity;
    arr[4] = data->CompState;
    arr[5] = data->CompZip;
    arr[6] = data->CompCountry;
    arr[7] = NULL;
    if( !vCardParse( "ADR;TYPE=WORK:", arr ) )
      if( !vCardParse( "ADR;WORK:", arr ) )
        vCardParse( "ADR;WORK;ENCODING=QUOTED-PRINTABLE:", arr );
    vCardPresent = 1;
  }
}

/*int vRTCTask (void)
{
	Chip_RTC_Init();
	 Set current time for RTC 2:00:00PM, 2012-10-05
	FullTime.time[RTC_TIMETYPE_SECOND]  = 0;
	FullTime.time[RTC_TIMETYPE_MINUTE]  = 0;
	FullTime.time[RTC_TIMETYPE_HOUR]    = 14;
	FullTime.time[RTC_TIMETYPE_MONTH]   = 10;
	FullTime.time[RTC_TIMETYPE_YEAR]    = 2012;

	Chip_RTC_Enable(ENABLE);
}*/

/******************************************************************************
**                            End Of File
******************************************************************************/
