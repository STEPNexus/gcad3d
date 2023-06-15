//  Demo_TSTF_1.c         Demo test-tool-file-based
/*
 *
 * Copyright (C) 2022 CADCAM-Services Franz Reiter (franz.reiter@cadcam.co.at)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
-----------------------------------------------------
TODO:
TSTF_load - fileName is not yet symbolic ..
  ..

-----------------------------------------------------
Modifications:
2022-01-10 First version - replacing testTool CTRLF_*       RF
  ..

-----------------------------------------------------
*/
#ifdef globTag
 void TSTF(){}
#endif
/*!
\file  ../APP/Demo_TSTF_1.c
\brief  minimum example
\code
=====================================================
List_functions_start:
..
List_functions_end:
=====================================================

\endcode *//*----------------------------------------


Does execute file ../APP/Demo_tstf_1.dat

Test_it:
- set Option/compile DLL ON
- select Plugin / Demo_tstf_1
- modify this file and ../APP/Demo_tstf_1.dat without stopping gcad;
- Ctrl-P will rebuild and restart the plugin 


See INF_tstf__ in ../ut/tstf__.c
Info remote-command-control codes see ../../doc/html/RemoteControl_en.htm

*/


// definition "export"
#include "../xa/export.h"


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ut/ut_geo.h"              // UT_INT_MAX
#include "../ut/ut_txt.h"              // UTX_*
#include "../xa/mdl__.h"               // SIZMF*
#include "../xa/ap_dir.h"              // AP_get_bas_dir
#include "../xa/xa_msg.h"              // MSG_* ERR_*
#include "../db/ut_DB.h"               // DB_get_Text



//----------------------------------------------------------------
// EXPORTS to main-module
export int gCad_main ();
export int gCad_fini ();
export int  TSTF_cb (char*);


//----------------------------------------------------------------
// ext aus xa.c:
extern  char      AP_mod_fnam[SIZMFNam];  // der Modelname





//=========================================================
  int gCad_main () {
//=========================================================
// user has selected this plugin; starting ...

  int    irc;
  char   s1[256];


  // write to Main-Infowindow ..
  TX_Print("gCad_main of Demo_TSTF_1.dll");

  // get modelname from Mainprog
  printf("Modelname = %s\n",AP_mod_fnam);

  //----------------------------------------------------------------
  // load and execute commandfile; start at label, stop end exit at following label.
  sprintf(s1, "%ssrc/APP/Demo_tstf_1.dat",AP_get_bas_dir());
  // irc = TSTF_load (s1, ":L1:"); // CirSeg-Tria
  irc = TSTF_load (s1, ":L2:"); // CirSeg-Tria



  // get text-only (stored with "N#="text")
  TX_Print("- text of N20 is |%s|\n",DB_get_Text (20L));


  //----------------------------------------------------------------
  // finish application
  gCad_fini ();

  return 0;

}


//=========================================================
  int gCad_fini () {
//=========================================================
// dll being unloaded - reset Input, kill all open windows !!!

  printf("gCad_fini Demo_TSTF_1\n");

  // write to Main-Infowindow ..
  TX_Print("...... gCad_fini Demo_TSTF_1");

  AP_User_reset ();     // close application

  return 0;

}


//================================================================
  int TSTF_cb (char *txt) {
//================================================================
// callback from TSTF-mudule (see TSTF_load INF_tstf__)
// execute private testfunctions - must start with "::: " in <cmdfile>
// MS-Win: function must be exported.


  char    *p2, cmd[128];


  printf("TSTF_cb-Demo_tstf_1 |%s|\n",txt);

  // get s1 = functionname
  p2 = UTX_cp_word__ (cmd, txt);
     printf("  cmd=|%s| p2=|%s|\n",cmd,p2);



  // dispatch
  if(!strcmp (cmd, "myFunc1")) {
    myFunc1 (p2);

  } else {
    printf(" - %s\n",txt);
  }

  return 0;

}

//================================================================
  int myFunc1 (char *data) { 
//================================================================

  printf(" >>>>>>>>>>> myFunc1 - %s\n",data);

  return 0;

}


//================  EOF  ==================================
