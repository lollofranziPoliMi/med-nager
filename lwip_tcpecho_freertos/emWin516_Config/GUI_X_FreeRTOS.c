/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*           (C) 1996    SEGGER Microcontroller Systeme GmbH          *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File        : GUI_X.C
Purpose     : Config / System dependent externals for GUI
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

/* FreeRTOS include files */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "board.h"

/*********************************************************************
*
*       Global data
*/
static xSemaphoreHandle xQueueMutex;
static xSemaphoreHandle xSemaTxDone;

/*********************************************************************
*
*      Timing:
*                 GUI_X_GetTime()
*                 GUI_X_Delay(int)

  Some timing dependent routines require a GetTime
  and delay function. Default time unit (tick), normally is
1 ms.
*/

int GUI_X_GetTime(void)
{
	return ((int) xTaskGetTickCount());
}

void GUI_X_Delay(int ms)
{
	vTaskDelay( ms );
}

/*********************************************************************
*
*       GUI_X_Init()
*
* Note:
*     GUI_X_Init() is called from GUI_Init is a possibility to init
*     some hardware which needs to be up and running before the GUI.
*     If not required, leave this routine blank.
*/

void GUI_X_Init(void) {
}


/*********************************************************************
*
*       GUI_X_ExecIdle
*
* Note:
*  Called if WM is in idle state
*/

void GUI_X_ExecIdle(void) {}

/*********************************************************************
*
*      Multitasking:
*
*                 GUI_X_InitOS()
*                 GUI_X_GetTaskId()
*                 GUI_X_Lock()
*                 GUI_X_Unlock()
*
* Note:
*   The following routines are required only if emWin is used in a
*   true multi task environment, which means you have more than one
*   thread using the emWin API.
*   In this case the
*                       #define GUI_OS 1
*  needs to be in GUIConf.h
*/

/* Init OS */
void GUI_X_InitOS(void)
{ 
	/* Create Mutex lock */
	xQueueMutex = xSemaphoreCreateMutex();
	configASSERT (xQueueMutex != NULL);

	/* Queue Semaphore */ 
	vSemaphoreCreateBinary( xSemaTxDone );
	configASSERT ( xSemaTxDone != NULL );
}

void GUI_X_Unlock(void)
{ 
	xSemaphoreGive( xQueueMutex );	
}

void GUI_X_Lock(void)
{
	xSemaphoreTake( xQueueMutex, portMAX_DELAY );
}

/* Get Task handle */
U32  GUI_X_GetTaskId(void) 
{	  
	return ((U32) xTaskGetCurrentTaskHandle());
}

void GUI_X_WaitEvent (void) 
{
	while( xSemaphoreTake(xSemaTxDone, portMAX_DELAY ) != pdTRUE );
}


void GUI_X_SignalEvent (void) 
{
	xSemaphoreGive( xSemaTxDone );
}

/*********************************************************************
*
*      Logging: OS dependent

Note:
  Logging is used in higher debug levels only. The typical target
  build does not use logging and does therefor not require any of
  the logging routines below. For a release build without logging
  the routines below may be eliminated to save some space.
  (If the linker is not function aware and eliminates unreferenced
  functions automatically)

*/

void GUI_X_Log     (const char *s) { DEBUGOUT(s); }
void GUI_X_Warn    (const char *s) { DEBUGOUT(s); }
void GUI_X_ErrorOut(const char *s) { DEBUGOUT(s); }

/*************************** End of file ****************************/
