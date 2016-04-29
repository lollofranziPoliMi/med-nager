These emWin library files were built using:

arm-none-eabi-gcc gcc version 4.5.1 (Red Suite 2010Q4 by Code Red).

IAR ANSI C/C++ Compiler V6.40.1.53790/W32 for ARM
Copyright 1999-2012 IAR Systems AB.. 

Visual Studio 2010 Express edition


The library files in the uVision 4 folder are copies of the library files in the EmbeddedWorkbench 6 folder. Only the filenames are different.

There is one set of library files for each of the 5 ARM cores and one for the x86 core.

Library files that include '_d5' are built with GUI_DEBUG_LEVEL=5. Linking with these libraries will cause diagnostic messages for log entries, warnings, and entries to be produced.
The file GUI_Debug.h contains a description of these debug levels.

In this version Name Mangling has been resolved.
