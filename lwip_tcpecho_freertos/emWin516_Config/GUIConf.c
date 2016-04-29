/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2012  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.16 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIConf.c
Purpose     : Display controller initialization
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// Define the available number of bytes available for the GUI
//
//#define GUI_NUMBYTES  0x2000
//#define GUI_NUMBYTES  (1024 * 1024) * 2  // x MByte
//
// Define the average block size
//
#define GUI_BLOCKSIZE 0x128

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*
* Define these in application code 
*/
extern U32 GUI_Memory_Size;
extern U32 GUI_Memory[];
extern U32 GUI_Block_Size;

/*********************************************************************
*
*       GUI_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   available memory for the GUI.
*/
void GUI_X_Config(void) {
  //
  // 32 bit aligned memory area
  //
  //static U32 aMemory[GUI_NUMBYTES / 4] __attribute__((at(0x28050000)));//__attribute__((at(0x28250000)));
  //
  // Assign memory to emWin
  //
  GUI_ALLOC_AssignMemory(GUI_Memory, GUI_Memory_Size);
  GUI_ALLOC_SetAvBlockSize(GUI_Block_Size);
  //
  // Set default font
  //
  GUI_SetDefaultFont(GUI_FONT_6X8);
}

/*************************** End of file ****************************/
