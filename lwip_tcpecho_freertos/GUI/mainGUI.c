/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2011     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : GUI_WIDGET_GraphYtDemo.c
Purpose : Demonstrates the use of the GRAPH widget
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdlib.h>
#include <string.h>

//#include "BSP.h"
#include "GUI.h"
#include "DIALOG.h"
#include "GRAPH.h"
#include "logo_silica_11022010.c"
#include <lpc_types.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/**
 * @brief Touch screen initialisation done flag
 */
volatile int tsc_init_done = 0;				/*!< TSC Initiliation done flag */

/**
 * @brief GUI buffers required for emwin library
 */
#define GUI_NUMBYTES  ((1024 * 1024) * 2)	/*!< Size of GUI buffer in No. of bytes */
#define GUI_BLOCKSIZE (0x128)				/*!< GUI block size */

U32 GUI_Memory[GUI_NUMBYTES / sizeof(U32)];	/*!< GUI buffer  TODO locate_at SDRAM*/
U32 GUI_Memory_Size = GUI_NUMBYTES;					/*!< GUI buffer size */
U32 GUI_Block_Size = GUI_BLOCKSIZE;					/*!< GUI block size */

/* define vCard */
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

extern anagrafica_t *data;

typedef struct{
	uint8_t info[4];
	struct Nodo_t *prox;
} Nodo_t;
extern Nodo_t *lista, *listaScheduled;
extern IP_RTC_TIME_T FullTime;
//extern Nodo {
//	uint8_t info[4];
//	struct Nodo *prox;
//} Nodo;
//extern Nodo *Lista;
extern uint8_t* nomeMed;

typedef struct {
	uint8_t tag[4];
	char nome[35];
	int secA;
	int minA;
	int oraA;
	int giorniAllaScadenza;//int scaduta;
	int avvisare;
	int importanza;
	int presa;
	struct medicinale *next;
} medicinale;
extern	medicinale medic;
extern medicinale *ptrMedic;
//extern uint8_t numeroRigheMax;
//extern uint8_t m[numeroRigheMax][100];
#define maxR 100
#define maxC 100
extern uint8_t l_tabellaMedicine[maxR][maxC];

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define MAX_VALUE 100
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
/**
 * Global variables used by emWin application
 */
static char  empty = 255;
static volatile int start = 0;				/* Start flag for counter */
static volatile short counter = 0;			/* Count value */

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
WM_HWIN hDlg, hGraph = 0;


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _cbCallback
*
* Purpose:
*   Callback function of the dialog
*/
static void _cbCallback(WM_MESSAGE * pMsg) {

  hDlg = pMsg->hWin;
  switch (pMsg->MsgId) {

  case WM_INIT_DIALOG:
    break;

  case WM_NOTIFY_PARENT:
    break;

  default:
    WM_DefaultProc(pMsg);
  }
}
/**
 * LCD appplication task function for FreeRTOS & uCOS-III
 */

extern 	uint32_t _aVRAM[];

#define X_firm  105
#define X_tel   170

#define X_firm  105
#define X_tel   170

void vCardDisplay( void )
{
    char fullname[64];
    
    if( empty == 2 )
        return;

    WM_SetDesktopColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();
    GUI_GIF_Draw((const void *)aclogo_silica_11022010, sizeof(aclogo_silica_11022010), 100, 0);

    strcpy( fullname, data->nameFirstname );
    strcat( fullname, " " );
    strcat( fullname, data->nameLastname );
    GUI_SetFont(&GUI_Font24B_ASCII);
    GUI_DispStringAt( fullname, 20, 70);
    GUI_SetFont(&GUI_Font16B_ASCII);
    GUI_DispStringAt( data->CompName, 20, X_firm);
    GUI_SetFont(&GUI_Font13_ASCII);
    GUI_DispStringAt( data->CompStreet, 20, X_firm+16);    
    GUI_DispStringAt( data->CompCity, 20, X_firm+16+13);
    
    strcpy( fullname, data->CompZip );
    strcat( fullname, " " );
    strcat( fullname, data->CompState );
    strcat( fullname, " " );
    strcat( fullname, data->CompCountry );
    GUI_DispStringAt( fullname, 20, X_firm+16+13+13);    
    
    strcpy( fullname, "TEL : " );
    strcat( fullname, data->CompPhone );
    GUI_DispStringAt( fullname, 20, X_tel);    
    
    strcpy( fullname, "FAX : " );
    strcat( fullname, data->CompFax );
    GUI_DispStringAt( fullname, 20, X_tel+13);    
    
    strcpy( fullname, "CELL : " );
    strcat( fullname, data->ContactCellPho );
    GUI_DispStringAt( fullname, 20, X_tel+13+13);    

    GUI_SetColor( (GUI_COLOR)0x007f7f7f );
    GUI_FillRect( 0, 214, 319, 239 );
    GUI_SetBkColor((GUI_COLOR)0x007f7f7f);
    GUI_SetColor(GUI_RED);
    GUI_SetFont(&GUI_Font16B_ASCII);
    GUI_DispStringAt( "The Engineering of Distribution.", 20, 220);

    empty = 2;
}

#define X_M4  40
#define X_663  135

int modifica=0, statoDisplay=0, statoDisplayP=0;


int confronta_tag(uint8_t inf[4], uint8_t tUID[4]){
	int u = 0;
	if (inf[0]==tUID[0] && inf[1]==tUID[1] && inf[2]==tUID[2] && inf[3]==tUID[3]) {
		u = 1;
		return u;
	}else{
		u = 0;
		return u;
	}
}
void assegna_medicinale(medicinale *dest, medicinale *sur){
	int i = 0;
	for (i=0; i<4; i++)
		dest->tag[i] = sur->tag[i];
	for (i=0; i<35; i++)
		dest->nome[i] = sur->nome[i];
	dest->secA = sur->secA;
	dest->minA = sur->minA;
	dest->oraA = sur->oraA;
	dest->giorniAllaScadenza = sur->giorniAllaScadenza;
	dest->avvisare = sur->avvisare;
	dest->importanza = sur->importanza;
	dest->presa = sur->presa;
}
medicinale* ordinamento_lista_med(medicinale *pointer){

	if (pointer && pointer->next) {
		int k;

		do {
			medicinale *punt = pointer;
			k=0;

			while (punt->next) {
				medicinale *punt1 = punt;
				punt = punt->next;
				int ret = 0;
				if (punt->oraA < punt1->oraA){
					ret = 1;
				}else if (punt->oraA == punt1->oraA) {
					if (punt->minA < punt1->minA) {
						ret = 1;
					}else if (punt->minA == punt1->minA) {
						if (punt->secA < punt1->secA) {
							ret = 1;
						}else {
							ret = 0;
						}
					}
				}
				//int ret = memcmp((punt->info),(punt1->info),4);
				if (ret>0) {
					medicinale *temp;
					temp = malloc(sizeof(medicinale));
					assegna_medicinale(temp,punt1);
					assegna_medicinale(punt1,punt);
					assegna_medicinale(punt,temp);
//					assegnamento_uint8_t(temp,punt1);
//					assegnamento_uint8_t(punt1,punt);
//					assegnamento_uint8_t(punt,temp);

//					temp = punt1;
//					punt1 = punt;
//					punt = temp;
					k=1;
				}
			}
		} while (k != 0);
	}
	return pointer;

}
void vCardReset( void ){

/*
  if( empty == 0 ) {
    return;
  }
*/

  WM_SetDesktopColor(GUI_BLACK);
  GUI_SetBkColor(GUI_BLACK);
  GUI_SetColor(GUI_GREEN);
  //GUI_Clear();
  GUI_SetFont(&GUI_Font32B_ASCII);
  GUI_DispStringAt( "NESSUNA", 120-7*10/2, 50);
  GUI_DispStringAt( "MEDICINA", 120-7*10/2, 80);
  GUI_DispStringAt( "PRESENTE", 120-7*10/2, 110);

  showTime(NULL, NULL, NULL, NULL);



  empty = 0;
}

/*
 * a vCard has been detected
 */

void vCardUID( char *cardType, uint8_t *bUidPresent, uint8_t *nCardDetected, uint8_t *nCardPrevious ){

  char msg[20];

/*  if( empty == 1 || empty == 2 ) {	// to trovato almeno una vCard
    return;
  }*/

  if (nCardDetected==nCardPrevious){
	  return;
  }
  WM_SetDesktopColor(GUI_BLACK);
  GUI_SetBkColor(GUI_BLACK);
  GUI_SetColor(GUI_GREEN);
  GUI_Clear();

//  GUI_SetFont(&GUI_Font32B_ASCII);
//  GUI_DispStringAt( cardType, 120-strlen(cardType)*10/2, 20);
  if( bUidPresent != NULL )
  {
	  int i=0;
	  for (i=0;i<nCardDetected;i++){
	    sprintf( msg, "UID %02X : %02X%02X%02X%02X", i+1, bUidPresent[4*i+0], bUidPresent[4*i+1], bUidPresent[4*i+2], bUidPresent[4*i+3] );
	    GUI_DispStringAt( msg, 0, 20*i+20);
	  }

/*
	    sprintf( msg, "UID 01 : %02X%02X%02X%02X", bUidPresent[0], bUidPresent[1], bUidPresent[2], bUidPresent[3] );
	    GUI_DispStringAt( msg, 0, 80);
	    sprintf( msg, "UID 02 : %02X%02X%02X%02X", bUidPresent[4], bUidPresent[5], bUidPresent[6], bUidPresent[7] );
	    GUI_DispStringAt( msg, 0, 120);
*/
  }
  GUI_SetColor( (GUI_COLOR)0x003f3f3f );
  GUI_FillRect( 0, 214, 319, 239 );
  GUI_SetBkColor((GUI_COLOR)0x003f3f3f);
  GUI_SetColor(GUI_RED);
  GUI_SetFont(&GUI_Font24B_ASCII);
  GUI_DispStringAt( "Reading Card", 80, 215);

  empty = 1;
}

void showCardUIDList( char *cardType, Nodo_t *lista, uint8_t *nCardDetected, uint8_t *nCardPrevious ){

  char msg[35];

  WM_SetDesktopColor(GUI_BLACK);
  GUI_SetBkColor(GUI_BLACK);
  GUI_SetColor(GUI_GREEN);

  if( lista != NULL )
  {
	  Nodo_t *testa=lista;

	  int i=0,j=1;
	  sprintf( msg, "ELENCO MEDICINE sched:");
	  GUI_DispStringAt( msg, 0, 20);
	  while (lista != NULL ){
		  int k=0,c=0;
		  for (k=0; k<maxR; k++) {
			  uint8_t tempTag[4];
			  for (c=0;c<4;c++){
				  tempTag[c] = l_tabellaMedicine[k][c];
			  }

			  if (confronta_tag(lista->info,tempTag)==1) {
				  for (c=0;c<35;c++){
					  msg[c] = l_tabellaMedicine[k][c+4];
				  }

					GUI_DispStringAt( msg, 0, 20*j+20);
					j++;

			  }
			  else {
				  //GUI_DispStringAt( "MEDICINA NON REGISTRATA", 0, 20*j+20);
			  }
		  }
//		/*FARE UNA FUNZIONE CONFRONTA-TAG*/
//		uint8_t tempUID[4] = {0x6b,0xec,0x1d,0x9f};// {0x6b,0xec,0x1d,0x9f} = Neo BOROCILLINA; {0x9b,0x83,0x1e,0x9f} = TACHIPIRINA 1000;
//		int uu = confronta_tag(lista->info,tempUID);
//		if (uu==1){//lista->info[0]==0x6b && lista->info[1]==0xec && lista->info[2]==0x1d && lista->info[3]==0x9f){
//			sprintf( msg, "Neo BOROCILLINA");
//			GUI_DispStringAt( msg, 0, 20*j+20);
//			j++;
//
//		} else {
//			if (lista->info[0]==0x9b && lista->info[1]==0x83 && lista->info[2]==0x1e && lista->info[3]==0x9f){
//				sprintf( msg, "TACHIPIRINA 1000");
//				GUI_DispStringAt( msg, 0, 20*j+20);
//				j++;
//			} else {
//				GUI_SetColor(GUI_YELLOW);
//				sprintf( msg, "UID %02X : %02X%02X%02X%02X", i+1, lista->info[0], lista->info[1], lista->info[2], lista->info[3] );
//				GUI_DispStringAt( msg, 0, 20*j+20);
//				i++;
//				j++;
//				GUI_DispStringAt( "MEDICINA NON REGISTRATA", 0, 20*j+20);
//				j++;
//				GUI_SetColor(GUI_GREEN);
//				/*C'E' UN TAG NON REGISTRATO. DA FARE AVVISO CHE UNA MEDICINA NON E' INSERITA CORRETTAMENTE*/
//			}
//		}
		lista=lista->prox;
	}
    lista = testa;
  }


/*RETTANGOLO IN BASSO*/
//  GUI_SetColor( (GUI_COLOR)0x003f3f3f );
//  GUI_FillRect( 0, 214, 319, 239 );
//  GUI_SetBkColor((GUI_COLOR)0x003f3f3f);
//  GUI_SetColor(GUI_RED);
//  GUI_SetFont(&GUI_Font24B_ASCII);
//  GUI_DispStringAt( "Reading Card", 80, 215);

  empty = 1;
}

extern int fIntervalReached;
extern uint32_t previousSec;
void showTime( /*char *cardType,  uint8_t *bUidPresent, uint8_t *nCardDetected, uint8_t *nCardPrevious*/)
{
	  char msg[100];

	  Chip_RTC_GetFullTime(&FullTime);
	    sprintf( msg, "%.2d:%.2d:%.2d %.2d/%.2d/%.4d\r\n", FullTime.time[2],
	    		FullTime.time[1],
	    		FullTime.time[0],
	    		FullTime.time[3],
	    		FullTime.time[6],
	    		FullTime.time[7]);

	    GUI_SetColor(GUI_WHITE);
	    GUI_FillRect( 0, 214, 317, 214 );
	    GUI_SetFont(&GUI_Font24B_ASCII);
	    GUI_DispStringAt( msg, 60, 215);
	    GUI_SetColor(GUI_GREEN);

}

void showTimeAlarm( uint8_t *bUidPresent, uint8_t *nCardDetected, uint8_t *nCardPrevious)
{
	  char msg[100];

	  statoDisplay=6;
	  Chip_RTC_GetFullTime(&FullTime);
	  if (statoDisplayP!=6) {
		  GUI_Clear();
	  }

	  int ii=0;
		int k=0,c=0,rimanenti=0,soglia=0,dose=0;
		for (k=0; k<maxR; k++) {
			uint8_t tempTag[4];
			for (c=0;c<4;c++){
				tempTag[c] = l_tabellaMedicine[k][c];
			}
			if (confronta_tag(ptrMedic->tag,tempTag)==1) {
				rimanenti = l_tabellaMedicine[k][62];
				soglia = l_tabellaMedicine[k][64];
				dose = l_tabellaMedicine[k][55];
			}
		}
		    sprintf( msg, "Assumere: %s", ptrMedic->nome);
			    GUI_DispStringAt( msg, 0, 20);
			    sprintf( msg, "Dose: %d pastiglie", dose/4);
				GUI_DispStringAt( msg, 0, 40+20*ii);
				ii++;
			    if (ptrMedic->giorniAllaScadenza<30) {
			    	GUI_SetColor(GUI_YELLOW);
				    sprintf( msg, "Scade entro un mese!");
					GUI_DispStringAt( msg, 0, 40+20*ii);
			    	GUI_SetColor(GUI_GREEN);
			    	ii++;
			    }
			    if (ptrMedic->avvisare) {
			    	GUI_SetColor(GUI_ORANGE);
				    sprintf( msg, "Stai per esaurire il medicinale");
					GUI_DispStringAt( msg, 0, 40+20*ii);
			    	GUI_SetColor(GUI_GREEN);
			    	ii++;
			    }
		    	GUI_SetColor(GUI_GREEN);
			    sprintf( msg, "Rimangono %d pastiglie", rimanenti);
				GUI_DispStringAt( msg, 0, 40+20*ii);
				ii++;
			    sprintf( msg, "Soglia di avviso impostata: %d", soglia);
				GUI_DispStringAt( msg, 0, 40+20*ii);

			    showTime();

			    statoDisplayP=statoDisplay;
	  empty = 1;
}

/*
 * LCD display thread
 */
extern char  Card_Type[40], vCardPresent;
extern uint8_t bUid[10], bUidPresent[100], nCardDetected, nCardPrevious, nCardTot;
extern xSemaphoreHandle xSemaDataAvail, xSemaGUIend;
extern int fAlarmTimeMatched;
extern uint8_t tagRimosso[4], tagAggiunto[4];


portTASK_FUNCTION(vLcdTask, pvParameters)
{

	lcdInit( (uint32_t)&_aVRAM[0]);
	GUI_Init();
	WM_SetDesktopColor(GUI_GREEN);
	WM_SetCreateFlags(WM_CF_MEMDEV);
//	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
/*	 Create Mutex lock*/
//M	vCardUID( Card_Type, bUid);
//M	        vCardDisplay( );
	while ( 1 ) {
		while( xSemaphoreTake(xSemaDataAvail, portMAX_DELAY ) != pdTRUE );
		if( Card_Type[0] )
        {//M	vCardUID( Card_Type, bUid);
        //M		vCardDisplay( );
			if( vCardPresent )
                    vCardDisplay( );
            else
                    vCardUID( Card_Type, bUid, nCardDetected, nCardPrevious);
            }
		else
			vCardReset( );
		xSemaphoreGive( xSemaGUIend );
//		GUI_Delay(10);
		vTaskDelay( 10 );
	}
}


portTASK_FUNCTION(vLcdTaskOpenDay, pvParameters)
{

	lcdInit( (uint32_t)&_aVRAM[0]);
	GUI_Init();
	WM_SetDesktopColor(GUI_GREEN);
	WM_SetCreateFlags(WM_CF_MEMDEV);
	char messaggio[50],msgg[3][70];
	statoDisplay=1;
	while ( 1 ) {
		while( xSemaphoreTake(xSemaDataAvail, portMAX_DELAY ) != pdTRUE );

		if (fIntervalReached==1) {
			GUI_ClearRect(0,214,317,249);
			showTime();
			fIntervalReached=0;
		}

//CALCOLA STATO DISPLAY SITUAZIONE NORMALE
		if (!fAlarmTimeMatched) {
			Nodo_t *temp1 = lista;
			Nodo_t *temp2 = listaScheduled;
			int j=0;
			statoDisplay=1;
			while (temp2!=NULL) {
				if(confronta_tag(temp1->info,temp2->info)==1) {
					temp1=lista;
					temp2=temp2->prox;
				} else {
					if (temp1->prox!=NULL) {
						temp1=temp1->prox;
					} else {
						//temp2 non è presente nella scatola
							statoDisplay=2;
							int k=0,c=0;
							for (k=0; k<maxR; k++) {
								uint8_t tempTag[4];
								for (c=0;c<4;c++){
									tempTag[c] = l_tabellaMedicine[k][c];
								}
								if (confronta_tag(temp2->info,tempTag)==1) {
									for (c=0;c<35;c++){
										msgg[j][c] = l_tabellaMedicine[k][c+4];
									}
									j++;

									temp1=lista;
									temp2=temp2->prox;
								}
							}
					}//FINE else //temp2 non è presente nella scatola
				}
			}

			if (statoDisplay==1) {
				if (statoDisplayP!=1){
					GUI_Clear();
				}
				GUI_DispStringAt( "Nessuna medicina", 120-7*10/2, 50);
				GUI_DispStringAt( "da assumere", 120-7*10/2, 100);
			showTime();
			}

			if (statoDisplay==2) {
				if (statoDisplayP!=2){
					GUI_Clear();
				}
				GUI_Clear();
				int c=0;
				GUI_DispStringAt( "Nella scatola manca:", 80, 20);
				for (c=0;c<j;c++){
					GUI_DispStringAt( msgg[c], 80, 20*c+40);
				}
				showTime();
			}
			statoDisplayP=statoDisplay;
			vTaskDelay(300);
		}//FINE if !fAlarmTimeMatched
		else { //fAlarmTimeMatched
			showTimeAlarm(bUidPresent,nCardDetected,nCardPrevious);



			if ((nCardDetected<nCardPrevious) /*&& (tag)*/) {
				//GUI_Clear();
				if (tagRimosso[0] == ptrMedic->tag[0] && tagRimosso[1] == ptrMedic->tag[1] && tagRimosso[2] == ptrMedic->tag[2] && tagRimosso[3] == ptrMedic->tag[3] ){ //tag corretto
					GUI_SetColor(GUI_GREEN);
					GUI_Clear();
					sprintf( messaggio, "Medicina Corretta!");
					GUI_DispStringAt( messaggio, 40, 100);
					vTaskDelay(5000);
					if (nCardDetected<nCardTot) {
						GUI_DispStringAt( "Riposiziona la medicina", 0, 100);
						vTaskDelay(1000);
					}
					fAlarmTimeMatched=0;
					modifica=1;
					//MODIFICARE DOSI RIMANENTI
					int cc = 0;
					for (cc=0; cc<maxR; cc++) {
						if ((ptrMedic->tag[1] == l_tabellaMedicine[cc][1]) && (ptrMedic->tag[2] == l_tabellaMedicine[cc][2]) && (ptrMedic->tag[3] == l_tabellaMedicine[cc][3]) && (ptrMedic->tag[4] == l_tabellaMedicine[cc][4])) {
							uint8_t dose = l_tabellaMedicine[cc][55], rimasteInt = l_tabellaMedicine[cc][62], rimasteFraz = l_tabellaMedicine[cc][63];
							uint16_t rimaste = (rimasteInt << 2) | rimasteFraz; //0b11
							rimaste = rimaste-(uint16_t)dose; //SOLO QUANDO LA ASSUMO!
							uint8_t temp = rimaste;
							temp = temp&0b00000011;
							rimasteFraz = temp ;
							rimaste >>= 2;
							rimasteInt = (uint8_t) rimaste;

							l_tabellaMedicine[cc][62] = rimasteInt;
							l_tabellaMedicine[cc][63] = rimasteFraz;
						}

					}
					ptrMedic = ptrMedic->next;

					IP_RTC_TIME_T oraAllarme;
					oraAllarme.time[RTC_TIMETYPE_HOUR]  = ptrMedic->oraA;
					oraAllarme.time[RTC_TIMETYPE_MINUTE]  = ptrMedic->minA;
					oraAllarme.time[RTC_TIMETYPE_SECOND]  = ptrMedic->secA;

					if (orarioDopoOrario(FullTime,oraAllarme)) {
						fAlarmTimeMatched = 1;
					}
					else{
						FullTime.time[RTC_TIMETYPE_HOUR] = oraAllarme.time[RTC_TIMETYPE_HOUR];
						FullTime.time[RTC_TIMETYPE_MINUTE] = oraAllarme.time[RTC_TIMETYPE_MINUTE];
						FullTime.time[RTC_TIMETYPE_SECOND] = oraAllarme.time[RTC_TIMETYPE_SECOND];
						Chip_RTC_SetFullAlarmTime(&FullTime);
					}

				} else{
					//mostra che hai tolto il tag ERRATO!
					GUI_SetColor(GUI_RED);
					GUI_Clear();
					sprintf( messaggio, "Medicina ERRATA!");
					GUI_DispStringAt( messaggio, 40, 100);
					showTimeAlarm(bUidPresent, nCardDetected, nCardPrevious);
					vTaskDelay(2000);
					modifica=1;
				}
			}


		}
		xSemaphoreGive( xSemaGUIend );
	}
}
//    	if (fAlarmTimeMatched){
//    		if (nCardDetected==nCardPrevious) {/*DA METTERE IL TAG ASSOCIATO ALL'ALLARME*/
//    			showTimeAlarm(bUidPresent, nCardDetected, nCardPrevious );
//    			//fAlarmTimeMatched=0;
//    			//vTaskDelay(500);
//    		}else{
//    			if ((nCardDetected<nCardPrevious) /*&& (tag)*/) {
//    				//GUI_Clear();
//        			if (tagRimosso[0] == ptrMedic->tag[0] && tagRimosso[1] == ptrMedic->tag[1] && tagRimosso[2] == ptrMedic->tag[2] && tagRimosso[3] == ptrMedic->tag[3] ){ //tag corretto
//    					//mostra l''ok e togli alarm (fAlarmTimeMatched = 0;)
////                				while (tagAggiunto!=ptrMedic->tag) {
////                					//TEMPORIZZARE IL TUTTO!
////                				}
//    					GUI_SetColor(GUI_GREEN);
//        				sprintf( messaggio, "Medicina Corretta!");
//        				GUI_DispStringAt( messaggio, 0, 150);
//        				vTaskDelay(5000);
//        				if (nCardDetected<nCardTot) {
//        					GUI_DispStringAt( "Riposiziona la medicina", 0, 150);
//        				}
//        				fAlarmTimeMatched=0;
//        				modifica=1;
//        				//MODIFICARE DOSI RIMANENTI
//        				int cc = 0;
//        				for (cc=0; cc<maxR; cc++) {
//            				if ((ptrMedic->tag[1] == l_tabellaMedicine[cc][1]) && (ptrMedic->tag[2] == l_tabellaMedicine[cc][2]) && (ptrMedic->tag[3] == l_tabellaMedicine[cc][3]) && (ptrMedic->tag[4] == l_tabellaMedicine[cc][4])) {
//            					uint8_t dose = l_tabellaMedicine[cc][55], rimasteInt = l_tabellaMedicine[cc][62], rimasteFraz = l_tabellaMedicine[cc][63];
//            					uint16_t rimaste = (rimasteInt << 2) | rimasteFraz; //0b11
//            					rimaste = rimaste-(uint16_t)dose; //SOLO QUANDO LA ASSUMO!
//            					uint8_t temp = rimaste;
//            					temp = temp&0b00000011;
//            					rimasteFraz = temp ;
//            					rimaste >>= 2;
//            					rimasteInt = (uint8_t) rimaste;
//
//            					l_tabellaMedicine[cc][62] = rimasteInt;
//            					l_tabellaMedicine[cc][63] = rimasteFraz;
//            				}
//
//        				}
//        				ptrMedic = ptrMedic->next;
//
//        				IP_RTC_TIME_T oraAllarme;
//						oraAllarme.time[RTC_TIMETYPE_HOUR]  = ptrMedic->oraA;
//						oraAllarme.time[RTC_TIMETYPE_MINUTE]  = ptrMedic->minA;
//						oraAllarme.time[RTC_TIMETYPE_SECOND]  = ptrMedic->secA;
//
//        				if (orarioDopoOrario(FullTime,oraAllarme)) {
//        					fAlarmTimeMatched = 1;
//							}
//        				else{
//        					FullTime.time[RTC_TIMETYPE_HOUR] = oraAllarme.time[RTC_TIMETYPE_HOUR];
//        					FullTime.time[RTC_TIMETYPE_MINUTE] = oraAllarme.time[RTC_TIMETYPE_MINUTE];
//        					FullTime.time[RTC_TIMETYPE_SECOND] = oraAllarme.time[RTC_TIMETYPE_SECOND];
//							Chip_RTC_SetFullAlarmTime(&FullTime);
//        				}
//
//    				} else{
//    					//mostra che hai tolto il tag ERRATO!
//    					GUI_SetColor(GUI_RED);
//    					sprintf( messaggio, "Medicina ERRATA!");
//    					GUI_DispStringAt( messaggio, 0, 150);
//    					showTimeAlarm(bUidPresent, nCardDetected, nCardPrevious);
//    					vTaskDelay(2000);
//    					modifica=1;
//    				}
//
//    			}else {
//    				vTaskDelay(20);
//    			}
//
//    		}
//    	}
//		if( Card_Type[0] )
//        {//M	vCardUID( Card_Type, bUid);
//        //M		vCardDisplay( );
//			if( vCardPresent )
//                    vCardDisplay( );
//            else{
//            	//GUI_ClearRect(0,0,317,213);
//                    //vCardUID( Card_Type, bUidPresent, nCardDetected, nCardPrevious);
////                showCardUIDList( Card_Type, lista, nCardDetected, nCardPrevious);
//                showCardUIDList( Card_Type, listaScheduled, nCardDetected, nCardPrevious);
//            	//showTime(Card_Type, bUidPresent, nCardDetected, nCardPrevious );
//            	if (fAlarmTimeMatched){
//            		if (nCardDetected==nCardPrevious) {/*DA METTERE IL TAG ASSOCIATO ALL'ALLARME*/
//            			showTimeAlarm(bUidPresent, nCardDetected, nCardPrevious );
//            			//fAlarmTimeMatched=0;
//            			//vTaskDelay(500);
//            		}else{
//            			if ((nCardDetected<nCardPrevious) /*&& (tag)*/) {
//            				//GUI_Clear();
//                			if (tagRimosso[0] == ptrMedic->tag[0] && tagRimosso[1] == ptrMedic->tag[1] && tagRimosso[2] == ptrMedic->tag[2] && tagRimosso[3] == ptrMedic->tag[3] ){ //tag corretto
//            					//mostra l''ok e togli alarm (fAlarmTimeMatched = 0;)
////                				while (tagAggiunto!=ptrMedic->tag) {
////                					//TEMPORIZZARE IL TUTTO!
////                				}
//            					GUI_SetColor(GUI_GREEN);
//                				sprintf( messaggio, "Medicina Corretta!");
//                				GUI_DispStringAt( messaggio, 0, 150);
//                				vTaskDelay(5000);
//                				if (nCardDetected<nCardTot) {
//                					GUI_DispStringAt( "Riposiziona la medicina", 0, 150);
//                				}
//                				fAlarmTimeMatched=0;
//                				modifica=1;
//                				//MODIFICARE DOSI RIMANENTI
//                				int cc = 0;
//                				for (cc=0; cc<maxR; cc++) {
//                    				if ((ptrMedic->tag[1] == l_tabellaMedicine[cc][1]) && (ptrMedic->tag[2] == l_tabellaMedicine[cc][2]) && (ptrMedic->tag[3] == l_tabellaMedicine[cc][3]) && (ptrMedic->tag[4] == l_tabellaMedicine[cc][4])) {
//                    					uint8_t dose = l_tabellaMedicine[cc][55], rimasteInt = l_tabellaMedicine[cc][62], rimasteFraz = l_tabellaMedicine[cc][63];
//                    					uint16_t rimaste = (rimasteInt << 2) | rimasteFraz; //0b11
//                    					rimaste = rimaste-(uint16_t)dose; //SOLO QUANDO LA ASSUMO!
//                    					uint8_t temp = rimaste;
//                    					temp = temp&0b00000011;
//                    					rimasteFraz = temp ;
//                    					rimaste >>= 2;
//                    					rimasteInt = (uint8_t) rimaste;
//
//                    					l_tabellaMedicine[cc][62] = rimasteInt;
//                    					l_tabellaMedicine[cc][63] = rimasteFraz;
//                    				}
//
//                				}
//                				ptrMedic = ptrMedic->next;
//
//                				IP_RTC_TIME_T oraAllarme;
//								oraAllarme.time[RTC_TIMETYPE_HOUR]  = ptrMedic->oraA;
//								oraAllarme.time[RTC_TIMETYPE_MINUTE]  = ptrMedic->minA;
//								oraAllarme.time[RTC_TIMETYPE_SECOND]  = ptrMedic->secA;
//
//                				if (orarioDopoOrario(FullTime,oraAllarme)) {
//                					fAlarmTimeMatched = 1;
//									}
//                				else{
//                					FullTime.time[RTC_TIMETYPE_HOUR] = oraAllarme.time[RTC_TIMETYPE_HOUR];
//                					FullTime.time[RTC_TIMETYPE_MINUTE] = oraAllarme.time[RTC_TIMETYPE_MINUTE];
//                					FullTime.time[RTC_TIMETYPE_SECOND] = oraAllarme.time[RTC_TIMETYPE_SECOND];
//									Chip_RTC_SetFullAlarmTime(&FullTime);
//                				}
//
//            				} else{
//            					//mostra che hai tolto il tag ERRATO!
//            					GUI_SetColor(GUI_RED);
//            					sprintf( messaggio, "Medicina ERRATA!");
//            					GUI_DispStringAt( messaggio, 0, 150);
//            					showTimeAlarm(bUidPresent, nCardDetected, nCardPrevious);
//            					vTaskDelay(2000);
//            					modifica=1;
//            				}
//
//            			}else {
//            				vTaskDelay(20);
//            			}
//            			/*DA IMPLEMENTARE IL CONTROLLO SUL TEMPO DI MANCANZA DEL TAG*/
////            			fAlarmTimeMatched=0;
////            			FullTime.time[RTC_TIMETYPE_MINUTE]  = 16;
////            			FullTime.time[RTC_TIMETYPE_SECOND]  = 00;
////            			Chip_RTC_SetFullAlarmTime(&FullTime);
//            		}
//            	} //else
//            		//showTime(Card_Type, bUidPresent, nCardDetected, nCardPrevious );
//            }
//        }
//		else {
//			/*FARE CONTROLLO SE TAG TOLTO E' DELL'ALLARME O NO*/
//			vCardReset( );
//
//		}
//		xSemaphoreGive( xSemaGUIend );
////		GUI_Delay(10);
//		vTaskDelay( 10 );
//	}
//}

portTASK_FUNCTION(vLcdTaskNew, pvParameters)
{

	lcdInit( (uint32_t)&_aVRAM[0]);
	GUI_Init();
	WM_SetDesktopColor(GUI_GREEN);
	WM_SetCreateFlags(WM_CF_MEMDEV);
//	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
	/* Create Mutex lock */
//M	vCardUID( Card_Type, bUid);
//M	        vCardDisplay( );
	char messaggio[50];
	while ( 1 ) {
		while( xSemaphoreTake(xSemaDataAvail, portMAX_DELAY ) != pdTRUE );
		if (modifica) {
			GUI_Clear();
			modifica=0;
			showTime();
		}
		  if(fIntervalReached==1) {
			  GUI_ClearRect(0,214,317,249);
			  showTime();
			  fIntervalReached=0;
		  }

		if( Card_Type[0] )
        {//M	vCardUID( Card_Type, bUid);
        //M		vCardDisplay( );
			if( vCardPresent )
                    vCardDisplay( );
            else{
            	//GUI_ClearRect(0,0,317,213);
                    //vCardUID( Card_Type, bUidPresent, nCardDetected, nCardPrevious);
//                showCardUIDList( Card_Type, lista, nCardDetected, nCardPrevious);
                showCardUIDList( Card_Type, listaScheduled, nCardDetected, nCardPrevious);
            	//showTime(Card_Type, bUidPresent, nCardDetected, nCardPrevious );
            	if (fAlarmTimeMatched){
            		if (nCardDetected==nCardPrevious) {/*DA METTERE IL TAG ASSOCIATO ALL'ALLARME*/
            			showTimeAlarm(bUidPresent, nCardDetected, nCardPrevious );
            			//fAlarmTimeMatched=0;
            			//vTaskDelay(500);
            		}else{
            			if ((nCardDetected<nCardPrevious) /*&& (tag)*/) {
            				//GUI_Clear();
                			if (tagRimosso[0] == ptrMedic->tag[0] && tagRimosso[1] == ptrMedic->tag[1] && tagRimosso[2] == ptrMedic->tag[2] && tagRimosso[3] == ptrMedic->tag[3] ){ //tag corretto
            					//mostra l''ok e togli alarm (fAlarmTimeMatched = 0;)
//                				while (tagAggiunto!=ptrMedic->tag) {
//                					//TEMPORIZZARE IL TUTTO!
//                				}
            					GUI_SetColor(GUI_GREEN);
                				sprintf( messaggio, "Medicina Corretta!");
                				GUI_DispStringAt( messaggio, 0, 150);
                				vTaskDelay(5000);
                				while(nCardDetected<nCardTot) {
                					GUI_DispStringAt( "Riposiziona la medicina", 0, 150);
                				}
                				fAlarmTimeMatched=0;
                				modifica=1;
                				//MODIFICARE DOSI RIMANENTI
                				int cc = 0;
                				for (cc=0; cc<maxR; cc++) {
                    				if ((ptrMedic->tag[1] == l_tabellaMedicine[cc][1]) && (ptrMedic->tag[2] == l_tabellaMedicine[cc][2]) && (ptrMedic->tag[3] == l_tabellaMedicine[cc][3]) && (ptrMedic->tag[4] == l_tabellaMedicine[cc][4])) {
                    					uint8_t dose = l_tabellaMedicine[cc][55], rimasteInt = l_tabellaMedicine[cc][62], rimasteFraz = l_tabellaMedicine[cc][63];
                    					uint16_t rimaste = (rimasteInt << 2) | rimasteFraz; //0b11
                    					rimaste = rimaste-(uint16_t)dose; //SOLO QUANDO LA ASSUMO!
                    					uint8_t temp = rimaste;
                    					temp = temp&0b00000011;
                    					rimasteFraz = temp ;
                    					rimaste >>= 2;
                    					rimasteInt = (uint8_t) rimaste;

                    					l_tabellaMedicine[cc][62] = rimasteInt;
                    					l_tabellaMedicine[cc][63] = rimasteFraz;
                    				}

                				}
                				ptrMedic = ptrMedic->next;

                				IP_RTC_TIME_T oraAllarme;
								oraAllarme.time[RTC_TIMETYPE_HOUR]  = ptrMedic->oraA;
								oraAllarme.time[RTC_TIMETYPE_MINUTE]  = ptrMedic->minA;
								oraAllarme.time[RTC_TIMETYPE_SECOND]  = ptrMedic->secA;

                				if (orarioDopoOrario(FullTime,oraAllarme)) {
                					fAlarmTimeMatched = 1;
									}
                				else{
                					FullTime.time[RTC_TIMETYPE_HOUR] = oraAllarme.time[RTC_TIMETYPE_HOUR];
                					FullTime.time[RTC_TIMETYPE_MINUTE] = oraAllarme.time[RTC_TIMETYPE_MINUTE];
                					FullTime.time[RTC_TIMETYPE_SECOND] = oraAllarme.time[RTC_TIMETYPE_SECOND];
									Chip_RTC_SetFullAlarmTime(&FullTime);
                				}

            				} else{
            					//mostra che hai tolto il tag ERRATO!
            					GUI_SetColor(GUI_RED);
            					sprintf( messaggio, "Medicina ERRATA!");
            					GUI_DispStringAt( messaggio, 0, 150);
            					showTimeAlarm(bUidPresent, nCardDetected, nCardPrevious);
            					vTaskDelay(2000);
            					modifica=1;
            				}

            			}else {
            				vTaskDelay(20);
            			}
            			/*DA IMPLEMENTARE IL CONTROLLO SUL TEMPO DI MANCANZA DEL TAG*/
//            			fAlarmTimeMatched=0;
//            			FullTime.time[RTC_TIMETYPE_MINUTE]  = 16;
//            			FullTime.time[RTC_TIMETYPE_SECOND]  = 00;
//            			Chip_RTC_SetFullAlarmTime(&FullTime);
            		}
            	} //else
            		//showTime(Card_Type, bUidPresent, nCardDetected, nCardPrevious );
            }
        }
		else {
			/*FARE CONTROLLO SE TAG TOLTO E' DELL'ALLARME O NO*/
			vCardReset( );

		}
		xSemaphoreGive( xSemaGUIend );
//		GUI_Delay(10);
		vTaskDelay( 10 );
	}
}

//portTASK_FUNCTION(vLcdTaskNew, pvParameters)
//{
//
//	lcdInit( (uint32_t)&_aVRAM[0]);
//	GUI_Init();
//	WM_SetDesktopColor(GUI_GREEN);
//	WM_SetCreateFlags(WM_CF_MEMDEV);
////	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
//	/* Create Mutex lock */
////M	vCardUID( Card_Type, bUid);
////M	        vCardDisplay( );
////	Lista lista;
//	char messaggio[50];
//	while ( 1 ) {
//		while( xSemaphoreTake(xSemaDataAvail, portMAX_DELAY ) != pdTRUE );
//		if( Card_Type[0] )
//        {//M	vCardUID( Card_Type, bUid);
//        //M		vCardDisplay( );
//			if( vCardPresent )
//                    vCardDisplay( );
//            else{
//                    //vCardUID( Card_Type, bUidPresent, nCardDetected, nCardPrevious);
//                    showCardUIDList( Card_Type, lista, nCardDetected, nCardPrevious);
//            	showTime(Card_Type, bUidPresent, nCardDetected, nCardPrevious );
//            	if (fAlarmTimeMatched){
//            		if (nCardDetected==nCardPrevious) {/*DA METTERE IL TAG ASSOCIATO ALL'ALLARME*/
//            			showTimeAlarm(bUidPresent, nCardDetected, nCardPrevious );
//            			//fAlarmTimeMatched=0;
//            			//vTaskDelay(500);
//            		}else{
//            			if ((nCardDetected<nCardPrevious) /*&& (tag)*/) {
////            				if (tagRimosso[0] == 0x9B && tagRimosso[1] == 0x83 && tagRimosso[2] == 0x1E && tagRimosso[3] == 0x9F ){ //tag corretto
//                			if (tagRimosso[0] == ptrMedic->tag[0] && tagRimosso[1] == ptrMedic->tag[1] && tagRimosso[2] == ptrMedic->tag[2] && tagRimosso[3] == ptrMedic->tag[3] ){ //tag corretto
//            					//mostra l''ok e togli alarm (fAlarmTimeMatched = 0;)
////                				while (tagAggiunto!=ptrMedic->tag) {
////                					//TEMPORIZZARE IL TUTTO!
////                				}
//            					GUI_SetColor(GUI_GREEN);
//                				sprintf( messaggio, "Medicina Corretta!");
//                				GUI_DispStringAt( messaggio, 0, 150);
//                				vTaskDelay(10000);
//                				fAlarmTimeMatched=0;
//                				//MODIFICARE DOSI RIMANENTI
//                				int cc = 0;
//                				for (cc=0; cc<maxR; cc++) {
//                    				if ((ptrMedic->tag[1] == l_tabellaMedicine[cc][1]) && (ptrMedic->tag[2] == l_tabellaMedicine[cc][2]) && (ptrMedic->tag[3] == l_tabellaMedicine[cc][3]) && (ptrMedic->tag[4] == l_tabellaMedicine[cc][4])) {
//                    					uint8_t dose = l_tabellaMedicine[cc][55], rimasteInt = l_tabellaMedicine[cc][62], rimasteFraz = l_tabellaMedicine[cc][63];
//                    					uint16_t rimaste = (rimasteInt << 2) | rimasteFraz; //0b11
//                    					rimaste = rimaste-(uint16_t)dose; //SOLO QUANDO LA ASSUMO!
//                    					uint8_t temp = rimaste;
//                    					temp = temp&0b00000011;
//                    					rimasteFraz = temp ;
//                    					rimaste >>= 2;
//                    					rimasteInt = (uint8_t) rimaste;
//
//                    					l_tabellaMedicine[cc][62] = rimasteInt;
//                    					l_tabellaMedicine[cc][63] = rimasteFraz;
//                    				}
//
//                				}
//                				ptrMedic = ptrMedic->next;
//
//                				IP_RTC_TIME_T oraAllarme;
//								oraAllarme.time[RTC_TIMETYPE_HOUR]  = ptrMedic->oraA;
//								oraAllarme.time[RTC_TIMETYPE_MINUTE]  = ptrMedic->minA;
//								oraAllarme.time[RTC_TIMETYPE_SECOND]  = ptrMedic->secA;
//
//                				if (orarioDopoOrario(FullTime,oraAllarme)) {
//                					fAlarmTimeMatched = 1;
//									}
//                				else{
//                					FullTime.time[RTC_TIMETYPE_HOUR] = oraAllarme.time[RTC_TIMETYPE_HOUR];
//                					FullTime.time[RTC_TIMETYPE_MINUTE] = oraAllarme.time[RTC_TIMETYPE_MINUTE];
//                					FullTime.time[RTC_TIMETYPE_SECOND] = oraAllarme.time[RTC_TIMETYPE_SECOND];
//									Chip_RTC_SetFullAlarmTime(&FullTime);
//                				}
//
//            				} else{
//            					//mostra che hai tolto il tag ERRATO!
//            					GUI_SetColor(GUI_RED);
//            					sprintf( messaggio, "Medicina ERRATA!");
//            					GUI_DispStringAt( messaggio, 0, 150);
//            					showTimeAlarm(bUidPresent, nCardDetected, nCardPrevious);
//            					vTaskDelay(2000);
//            				}
//
////             				  GUI_SetBkColor((GUI_COLOR)0x3f3f3f3f);
////            				  GUI_SetColor(GUI_RED);
////            				  GUI_SetFont(&GUI_Font20_ASCII);
////
////            				sprintf( messaggio, "IL TAG %02X%02X%02X%02X E' STATO RIMOSSO", tagRimosso[0], tagRimosso[1], tagRimosso[2], tagRimosso[3] );
////            				GUI_DispStringAt( messaggio, 0, 150);
////            				vTaskDelay(5000);
//            			}else {
//vTaskDelay(20);
//            			}
//            			/*DA IMPLEMENTARE IL CONTROLLO SUL TEMPO DI MANCANZA DEL TAG*/
////            			fAlarmTimeMatched=0;
////            			FullTime.time[RTC_TIMETYPE_MINUTE]  = 16;
////            			FullTime.time[RTC_TIMETYPE_SECOND]  = 00;
////            			Chip_RTC_SetFullAlarmTime(&FullTime);
//            		}
//            	} else
//            		showTime(Card_Type, bUidPresent, nCardDetected, nCardPrevious );
//            }
///*			  GUI_SetColor( (GUI_COLOR)0x003f3f3f );
//			  GUI_FillRect( 0, 214, 319, 239 );
//			  GUI_SetBkColor((GUI_COLOR)0x003f3f3f);
//			  GUI_SetColor(GUI_RED);
//			  GUI_SetFont(&GUI_Font24B_ASCII);
//			  GUI_DispStringAt( "Reading Card", 80, 215);*/
//        }
//		else {
//			/*FARE CONTROLLO SE TAG TOLTO E' DELL'ALLARME O NO*/
//			vCardReset( );
////			sprintf( messaggio, (char*) nomeMed);
////			GUI_DispStringAt( messaggio, 0, 150);
//		}
//		xSemaphoreGive( xSemaGUIend );
////		GUI_Delay(10);
//		vTaskDelay( 10 );
//	}
//}

/*************************** End of file ****************************/
