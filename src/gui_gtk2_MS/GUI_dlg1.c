/*
 *
 * Copyright (C) 2020 CADCAM-Servies Franz Reiter (franz.reiter@cadcam.co.at)
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
Select file for open.
Returns selected file via stdout.
All testoutput to stdout must be precede with "##"
-----------------------------------------------------
TODO:

-----------------------------------------------------
Modifications:
2020-01-10 Created. RF.

=============================================================================
Using:  see INF_GUI_dlg1   (../inf/../inf/GUI.c)


=============================================================================
// ReBuild:
. ../options.sh && make -f GUI_dlg1.mak


// ReBuild and test:
./GUI_test_dlg1.sh


cat /tmp/debug.dat

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>              // for ...

#include <gtk/gtk.h>

#include "../ut/ut_umem.h"          // Memspc
#include "../ut/ut_txTab.h"         // UtxTab_NEW
// #include "../ut/deb_prt.h"          // printd

// activate printd
// #define printd printf
// disable printd
#define printd if(0) printf



// #include "../gui/gui__.h"


//----------------------------------------------------------------
// prototypes
  void UTX_CleanCR (char* string);
  int OS_osVar_eval__ (char *fno, char *fni, int fnoSiz);
  int GUI_dlg1_cancel (void *parent, void *data);
  char** UTX_wTab_file (char *memspc, int memSiz, char *fnam);
  char** OS_wTab_file (char **memspc, char *fnam);

//----------------------------------------------------------------
// GLOBAL:
  int        nArg, upState=0, iEnt=0, lEnt=64;
  char       **parLst, *wTabSpc, fnOut[400], *fnIn;

  GtkWidget  *wEnt;

  char *AP_bin_dir;         // dir binaries


//================================================================
  int GUI_get_fontSiz (int *isx, int *isy, void *win1) {
//================================================================
// Output:
//   isx     nr of pixels hor.
//   isy     nr of pixels vert.


  int       UI_fontsizX, UI_fontsizY;
  GdkFont   *wFont;
  GtkStyle  *UI_style_act;


  //----------------------------------------------------------------
// #ifdef gtk1
//   // Style hat farben, Font GdkFont *font, BackgrndPixmap usw
//   UI_style_act = gtk_widget_get_style (win1);
//   // Breite/Hoehe eines einzelnen Char einlesen. = 7,21
//   UI_fontsizX = gdk_char_width (UI_style_act->font, '0');
//   UI_fontsizY = gdk_char_height (UI_style_act->font, '0');
// #endif


  // Style hat farben, Font GdkFont *font, BackgrndPixmap usw
  UI_style_act = gtk_widget_get_style (win1);


  wFont = gtk_style_get_font (UI_style_act);
  // wFont = (GdkFont*) UI_style_act->font_desc;

  // Breite/Hoehe eines einzelnen Char einlesen. = 7,21
  UI_fontsizX = gdk_char_width (wFont, '0');
  *isx = UI_fontsizX;
  *isx += *isx / 2;   // + space between 2 chars


  UI_fontsizY = gdk_char_height (wFont, '0'); // + wFont->ascent + wFont->descent;
  // funktioniert in MS-Win leider nicht; liefert 21 statt 10
  // UI_fontsizY = wFont->ascent - wFont->descent;


  *isy = UI_fontsizY;
  *isy += *isy / 2;   // + space between 2 lines

	// *isx = 10;
	// *isy = 20;

    printf("##  ex-GUI_get_fontSiz isx = %d isy = %d\n",*isx,*isy);

  return 0;

}


//================================================================
  int GUI_dlg_dlgbe1_CB (void *parent, void *data) {
//================================================================
// button pressed

  char   sOut[256];

  printd(" GUI_dlg_dlgbe1_CB %d\n",*((int*)data));


  sprintf(sOut, "%d", *((int*)data));

  // add entryText
  if(iEnt) {
    strncat(sOut, (char*)gtk_entry_get_text(GTK_ENTRY(wEnt)), lEnt);
    sOut[lEnt] = '\0';
  }

  // return buttonNr   [ + entry]
  return GUI_dlg1_exit (sOut);

}


//================================================================
  int GUI_dlg_dlgbe1__ () {
//================================================================
// dlgbe = dialog with optional entry and 1 - 9 buttons
// returns index of selected button as first char of output; followed by entry-text.
// Cancel returns empty output.
// first button returns "0".
// - entrysize max 256
// 
//
// Input:  parameters parLst[nArg]
//   exenam dlgbe "infText" Cancel  NO    YES  --ent "entPreset" 16
//     0      1    infText  btNr+3  ..                entTxt     entSiz
//
// Output: a single line via file;
//   output empty = cancel 
//   first character is the selected button-nr; 0=first-button; 1=second ..
//   second char until EOL-char ist the content of the entry.
//
// imported from GUI_DialogEntry

  int    ii, btNr, isx, isy;
  char   *infTxt, *entTxt;
  GtkWidget    *UI_win=NULL, *box0, *box1, *w1, *bt1;

static int  btNra[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };


  printf("GUI_dlg_dlgbe1__  %d\n",nArg);

  // prepare filename, title
  if(nArg < 2)  return GUI_dlg1_err1 ();

  btNr = 0;
  entTxt = NULL;
  ii = 1;
  infTxt = parLst[ii];

  //----------------------------------------------------------------
  // find "--ent"
  for(;;) {
    ++ii;
    if(ii >= nArg) break;
    if(!strcmp(parLst[ii],"--ent")) {
      // next parameter must be entry-preset
      ++ii;
      if(ii >= nArg) return GUI_dlg1_err1 ();
      entTxt = parLst[ii];
      iEnt = 1;
      // next parameter can be size of entry
      ++ii;
      if(ii >= nArg) break;
      lEnt = atoi(parLst[ii]);
      if((lEnt < 2)||(lEnt > 256)) {
        printf(" *** ERROR GUI_dlg_dlgbe1__ - entSiz not 2-256 chars ..\n");
        return GUI_dlg1_err1 ();
      }
      break;
    }
    if(ii >= nArg) return GUI_dlg1_err1 ();
    ++btNr;
  }
    printf(" btNr=%d\n",btNr);



  //----------------------------------------------------------------
  // set 0-9 buttons
  // if(!btNr) GUI_dlg1_err1 ();
  if(btNr > 9) {
    printf(" *** ERROR GUI_dlg_dlgbe1__ - max 9 buttons ..\n");
    return GUI_dlg1_err1 ();
  }

    // TESTBLOCK
    if(iEnt) {
      printf(" entTxt=|%s| siz=%d\n",entTxt,lEnt);
    } else printf(" no-entry;\n");
    printd("  btNr = %d\n",btNr);
    for(ii=0; ii < btNr; ++ii) printf(" bt %d |%s|\n",ii,parLst[ii+2]);
    printf(" infTxt |%s| entTxt |%s| lEnt = %d\n",infTxt,entTxt,lEnt);
    // END TESTBLOCK


  //----------------------------------------------------------------
  L_start:
  // open window
  UI_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW(UI_win), GTK_WIN_POS_MOUSE);
  gtk_window_set_title (GTK_WINDOW (UI_win), " ... ");
  g_signal_connect (UI_win, "destroy",
                    G_CALLBACK(GUI_dlg1_cancel), NULL);

  // gtk_container_set_border_width(GTK_CONTAINER(UI_win), 4);

  GUI_get_fontSiz (&isx, &isy, UI_win);

  box0 = gtk_vbox_new (TRUE, 0);   // gtk2; TRUE=all objs same space
  // box0 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_set_spacing (GTK_BOX(box0), 10);
  gtk_container_add (GTK_CONTAINER (UI_win), box0);


  w1 = gtk_label_new (infTxt);
  // gtk_container_add (GTK_CONTAINER (box0), w1);
  gtk_box_pack_start (GTK_BOX (box0), w1, TRUE, TRUE, isx);

  //----------------------------------------------------------------
  // create entry .....................
  if(iEnt) {
    wEnt = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY(wEnt), lEnt);
    // preset Text
    gtk_entry_set_text (GTK_ENTRY(wEnt), entTxt);
    // gtk_entry_set_has_frame (GTK_ENTRY(wEnt), FALSE);
    gtk_container_add (GTK_CONTAINER (box0), wEnt);
  }


  //----------------------------------------------------------------
  // create buttons .....................
  box1 = gtk_hbox_new (TRUE, 0);   // gtk2; TRUE=all objs same space
  // box1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_set_spacing (GTK_BOX(box1), 5);
  gtk_container_add (GTK_CONTAINER (box0), box1);

  for(ii=0; ii < btNr; ++ii) {
    bt1 = gtk_button_new_with_label (parLst[ii+2]);
    g_signal_connect (bt1, "clicked",
                      G_CALLBACK(GUI_dlg_dlgbe1_CB),
                      &btNra[ii]);
    // gtk_container_add (GTK_CONTAINER (box1), bt1);
    gtk_box_pack_start (GTK_BOX (box1), bt1, TRUE, TRUE, isx);

  }

  gtk_widget_show_all (box0);
  gtk_widget_show (UI_win);

  GUI_update__ ();

  upState = 1;

  gtk_main ();

  return 0;

}


//================================================================
  int GUI_dlg1_list1_cbSel (void *selection, void* data) {
//================================================================
/// \code
/// GUI_dlg1_list1_cbSel      INTERNAL  callback list selection (list1 & list2)
///
/// \endcode


  int          irc, nCol;
  GtkTreeIter  iter;
  GtkTreeModel *model;
  gchar        *txt1, *txt2=NULL, *txt3=NULL;
  char         sOut[256];


  printd("## GUI_dlg1_list1_cbSel state = %d\n",upState);

  if(!upState) return (FALSE);    // startup must be complete


  if(gtk_tree_selection_get_selected (selection, &model, &iter)) {

    // get nr of columns
    nCol = gtk_tree_model_get_n_columns (model);


    // // get treePath as string (LineNr)
    // txt1 = gtk_tree_model_get_string_from_iter (model, &iter);
      // printf(" treePath=|%s|\n",txt1);
    // g_free (txt1);


//     pTab[0] = &GUI_dlg1_list1_evt;
//     pTab[1] = &GUI_dlg1_list1_msbt;
//     pTab[2] = &nCol;

    // decode selected row
    // iCol = 0;      // get data for first column
    gtk_tree_model_get (model, &iter, 0, &txt1, -1);  // access Spalte 0
//     pTab[3] = txt1;
    strcpy(sOut, txt1);
      // printf ("GUI_dlg1_list1_cbSel col0 |%s|\n", (char*)pTab[3]);

//     if(nCol > 1) {
//       gtk_tree_model_get (model, &iter, 1, &txt2, -1);  // access Spalte 1
//       pTab[4] = txt2;
//         // printf ("GUI_dlg1_list1_cbSel col1 |%s|\n", (char*)pTab[4]);
//     }

//     if(nCol > 2) {
//       gtk_tree_model_get (model, &iter, 2, &txt3, -1);  // access Spalte 2
//       pTab[5] = txt3;
//         // printf ("GUI_dlg1_list1_cbSel col2 |%s|\n", (char*)pTab[5]);
//     }

    g_free (txt1);
    if(txt2) g_free (txt2);
    if(txt3) g_free (txt3);

    // gtk_widget_destroy (wfl1);
    return GUI_dlg1_exit (sOut);

  // } else {
    // go->uFunc (NULL, NULL);
  }

//   GUI_dlg1_list1_msbt = 0;

  // GUI_update__ ();  // Gtk2 only - else crash deleting List !

  // return TRUE;  // skip the default handler
  return (FALSE);  // DONT skip the default handler

}


//===================================================================
  int GUI_dlg1_list1_f (void *list_store, char *fnam, int cnr) {
//===================================================================
/// GUI_dlg1_list1_f          INTERNAL  populate list
/// 1 or 2 columns from file (sep = blank)

  int         i1;
  GtkTreeIter iter;
  char        cbuf[256], *p1;
  FILE        *fpi;


  printd("## GUI_dlg1_list1_f |%s|\n",fnam);


  if ((fpi = fopen (fnam, "r")) == NULL) {
    return -1;
  }

  i1 = 0;

  while (!feof (fpi)) {
    ++i1;
    if(i1 > 10000) {
      fclose(fpi);
      return -2;
    }

    if (fgets (cbuf, 256, fpi) == NULL) break;
    if(cbuf[0] == '#') continue;  // skip comments
    // cut after first word
    p1 = strchr(cbuf, ' ');

    gtk_list_store_append (list_store, &iter);


    //----------------------------------------------------------------
    // 1 col-
    if(cnr == 1) {
      if(p1) { *p1 = '\0'; ++p1;}
      else { UTX_CleanCR (cbuf); p1 = cbuf;}
        // printd(" _list1_f-add-1 |%s|\n",cbuf);



      gtk_list_store_set (list_store, &iter,
                          0, cbuf,        // Col.0, data,
                          -1);            // EndOfParameters !

    //----------------------------------------------------------------
    // all
    } else {
      // display complete line
      UTX_CleanCR (cbuf);   // remove  CR-LF
      // find delimiter
//       p1 = strchr (cbuf, ' ');
//       if(p1) {
//         *p1 = '\0';
//         ++p1;
//         while(*p1  == ' ') ++p1;
//       // } else {
//       }
      gtk_list_store_set (list_store, &iter,
                          0, cbuf,        // Col.0, data,
                          // 1, p1,          // Col.1, data,
                          -1);            // EndOfParameters !
    }
  }

  fclose(fpi);

  return 0;

}




//================================================================
  int GUI_dlg1_list1__ () {
//================================================================
// Cancels returns empty output.

  int          wsx, wsy, isx, isy, cnr;
  GtkWidget    *UI_list1_win=NULL, *box0;
  GtkListStore *list_store;
  GtkWidget    *treeView;
  GtkCellRenderer *renderer;
  GtkWidget    *scrolled_win;
  GtkTreeViewColumn *col1;        //, *col2;
  GtkTreeSelection *select;
  char         *p1, *p2, *fNam, *wTit, *opts;



  // prepare filename, title
  if(nArg < 4)  return GUI_dlg1_err1 ();
  fNam = parLst[1];
  wTit = parLst[2];
  opts = parLst[3];


  //----------------------------------------------------------------
  // decode options
  wsx = 0;
  wsy = 0;
  cnr = 1; // nr columns; 1 (default, first word of line) or 0 (a=0=complete line)
  p1 = opts;

  L_nxt__:
  if(!*p1) goto L_nxt_e;
  if(*p1 == 'x') wsx = strtol (&p1[1], &p2, 10);
  else if(*p1 == 'y') wsy = strtol (&p1[1], &p2, 10);
  else if(*p1 == 'a') cnr = 0; {p2 = p1 + 1;}
  if(!*p2) goto L_nxt_e;
  ++p2;  // skip ','
  p1 = p2;
  goto L_nxt__;

  L_nxt_e:
    // printf(" dlg1_list1__ wsx=%d wsy=%d cnr=%d\n",wsx,wsy,cnr);


  //----------------------------------------------------------------
  // open window
  UI_list1_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW(UI_list1_win), GTK_WIN_POS_MOUSE);
  gtk_window_set_title (GTK_WINDOW (UI_list1_win), wTit);
  g_signal_connect (UI_list1_win, "destroy",
                    G_CALLBACK(GUI_dlg1_cancel), NULL);

  gtk_container_set_border_width(GTK_CONTAINER(UI_list1_win), 4);

  GUI_get_fontSiz (&isx, &isy, UI_list1_win);

  list_store = gtk_list_store_new (1, G_TYPE_STRING);

  // populate list
  GUI_dlg1_list1_f (list_store, fNam, cnr);

  // create a new view of data
  treeView = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  g_object_unref (list_store);


  gtk_tree_view_set_enable_search (GTK_TREE_VIEW(treeView), FALSE);
  // else comes callback from keyIn = crash ! (disable edit lines)

  if(!wTit)        // remove headerbar
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeView), FALSE);

  // create adjustments for treeView
  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_ALWAYS);

  // erste Spalte
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (G_OBJECT (renderer), "ypad", 0, NULL);

  // define columns; title, type.
  if(wTit) p1 = wTit;
  else     p1 = NULL;
  col1 = gtk_tree_view_column_new_with_attributes (p1,
                            renderer,
                            "text", 0,  // attributes ..
                            NULL);      // terminator
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), col1);

  gtk_container_add (GTK_CONTAINER (scrolled_win), treeView);


  //----------------------------------------------------------------
  // Setup the selection handler
//   GUI_dlg1_list1_msbt = 0;
  select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeView));

  // SINGLE does not select already selected !
  gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
  // gtk_tree_selection_set_mode (select, GTK_SELECTION_BROWSE);

  // gtk_widget_set_events (GTK_WIDGET(treeView),
                         // GDK_BUTTON_PRESS_MASK|
                         // GDK_KEY_PRESS_MASK);

  // sig changed: from mouse-select, key-blank, key-PageDwn !
  g_signal_connect (G_OBJECT (select), "changed",
                    G_CALLBACK (GUI_dlg1_list1_cbSel),
                    NULL);      // PTR_MEMOBJ(go->mem_obj));


  //----------------------------------------------------------------
  gtk_widget_set_size_request (GTK_WIDGET(scrolled_win),    // or box0
                               wsx * isx, wsy * isy);
  // gtk_widget_set_hexpand (GTK_WIDGET(scrolled_win), TRUE);
  // gtk_widget_set_vexpand (GTK_WIDGET(scrolled_win), TRUE);



  //----------------------------------------------------------------
  gtk_widget_show (treeView);
  // gtk_widget_show_all (list_store);


  box0 = gtk_vbox_new (TRUE, 0);
  // gtk_box_pack_start (GTK_BOX (box0), scrolled_win, TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (box0), scrolled_win);
  gtk_container_add (GTK_CONTAINER (UI_list1_win), box0);
  gtk_widget_show_all (box0);
  gtk_widget_show (UI_list1_win);


  // gtk_window_set_transient_for (GTK_WINDOW(UI_list1_win), GTK_WINDOW(parWin));
  // gtk_window_set_modal (GTK_WINDOW (UI_list1_win), TRUE);


  gtk_tree_selection_unselect_all (select);   // geht ned ..

  GUI_update__ ();

  upState = 1;

  gtk_main ();

  return 0;

}


//================================================================
  int GUI_dlg1_info1__ () {
//================================================================
// display infotext in messageWindow with close-Button.
// 1,2 or 3 lines; separated with "\n".
// Example: \"Line1\nLine2\nLine3 \n- ERR: NOT DISPLAYED; ONLY 3 LINES !\"

 
  int          i1, lNr, sl;
  GtkWidget    *wdlg;
  char         sTxt[1024], *p0, *p1, *pa[4], *pe;

  printf("## GUI_dlg1_info1__\n");

  // prepare text, title
  if(nArg < 2)  return GUI_dlg1_err1 ();
  
  sl = strlen(parLst[1]);
  if(sl > 1020) {
    printf("ERROR GUI_dlg1_info1__ text too long\n");
    exit(1);
  }

  strcpy(sTxt, parLst[1]);

  p0 = &sTxt[0];
  pe = &sTxt[strlen(sTxt)-1];
  lNr = 0;
    // printf(" p0-1=|%s|\n",p0);
  for(i1=0; i1<3; ++i1) {
    if(p0 >= pe) break;
    p1 = strstr(p0, "\\n");
    if(p1) {
      *p1 = '\0';
      p1 += 2;
      pa[i1] = p1;
      ++lNr;
        // printf(" i1=%d p0|p1=|%s|%s|\n",i1,p0,p1);
      p0 = p1;
      continue;
    }
    pa[i1] = p0;
    ++lNr;
    break;
  }
    // printf(" lNr=%d\n",lNr);



  //----------------------------------------------------------------
  // open window
  if(lNr == 1) {
    wdlg = gtk_message_dialog_new (
           NULL, // GTK_WINDOW (UI_MainWin),  // keep on top of this else NULL
           GTK_DIALOG_DESTROY_WITH_PARENT,
           GTK_MESSAGE_OTHER, // .._INFO .._WARNING .._ERROR
           GTK_BUTTONS_CLOSE,
           "%s",sTxt);

  } else if(lNr == 2) {
    wdlg = gtk_message_dialog_new (
           NULL, // GTK_WINDOW (UI_MainWin),  // keep on top of this else NULL
           GTK_DIALOG_DESTROY_WITH_PARENT,
           GTK_MESSAGE_OTHER, // .._INFO .._WARNING .._ERROR
           GTK_BUTTONS_CLOSE,
           "%s\n%s",sTxt,pa[0]);

  } else {      //if(lNr == 2) {
    wdlg = gtk_message_dialog_new (
           NULL, // GTK_WINDOW (UI_MainWin),  // keep on top of this else NULL
           GTK_DIALOG_DESTROY_WITH_PARENT,
           GTK_MESSAGE_OTHER, // .._INFO .._WARNING .._ERROR
           GTK_BUTTONS_CLOSE,
           "%s\n%s\n%s",sTxt,pa[0],pa[1]);
  }


  // which icon:
  //   GTK_MESSAGE_INFO,GTK_MESSAGE_QUESTION
  //   GTK_MESSAGE_ERROR,GTK_MESSAGE_WARNING

  // Buttons:
  //   GTK_BUTTONS_OK, GTK_BUTTONS_CLOSE, GTK_BUTTONS_CANCEL
  //   GTK_BUTTONS_YES_NO, GTK_BUTTONS_OK_CANCEL

  // wait, modal:
  // start waiting; does not return until user clicks button.
  gtk_dialog_run (GTK_DIALOG (wdlg));    // wait (modal) !
  // GTK_RESPONSE_OK GTK_RESPONSE_YES GTK_RESPONSE_CANCEL ..

  gtk_widget_destroy (wdlg);

  GUI_dlg1_exit ("-");

  return 0;

}


//================================================================
  int GUI_dlg1_exit (char *sOut) {
//================================================================
// exit - provide selection to caller via stdout

  int     i1;
  FILE    *fpo;

  printf("## GUI_dlg1_exit |%s|\n",sOut);

  // info has no output ..
  if(!strcmp(parLst[1],"info")) goto L_exit;

  i1 = nArg - 1;
  if((fpo=fopen(fnOut,"w")) == NULL) {
    printf("***** symdir__ - Error GUI_file GUI_dlg1_exit Open %s\n",fnOut);
    exit(1);
  }

  fprintf(fpo, "%s\n",sOut);
  fclose(fpo);


  L_exit:
    // gtk_widget_hide (UI_list1_win);
    // gtk_signal_emit_by_name (GTK_OBJECT(UI_list1_win),"destroy");
    // gtk_widget_destroy (UI_list1_win);


  // remove inputFile
  OS_file_delete (fnIn);
 

  exit(0);

}


//================================================================
  int GUI_dlg1_cancel (void *parent, void *data) {
//================================================================
/// \code
/// INTERNAL
/// list-window exiting; report to user ..
/// \endcode


  printd("## GUI_dlg1_cancel\n");

  return (GUI_dlg1_exit (""));

}

  

//================================================================
  int main (int argc, char *argv[]) {
//================================================================
// Input:
//   argv[1]    inputfilename
// Output:
//   stdout     full selected filename; empty for Cancel;


  int    irc,i1;


  nArg = argc;

  printf("## start exe GUI_dlg1 V1.1 .. %d\n",argc);

  if(argc < 2) goto L_exit;

  for(i1=0; i1<argc; ++i1) printf("## GUI_dlg1 argv[%d]=|%s|\n",i1,argv[i1]);

  gtk_disable_setlocale ();  // sonst Beistrich statt Decimalpunkt !! (LC_ALL)

  i1 = 0;
  gtk_init (&i1, NULL);

  // get filename fnOut from fnIn (argv[1])
  fnIn = argv[1];
  irc = GUI_file_out_in (fnOut, fnIn);

  // get parLst = content of file argv[1]
  parLst = OS_wTab_file (&wTabSpc, fnIn);

  nArg = 0;
  while(parLst[nArg]) {
    printf("## %d = %s\n",nArg,parLst[nArg]);
    ++nArg;
  }

  // parLst[0] is list1 info or dlgbe
  if(!strcmp(parLst[0],"list1")) return GUI_dlg1_list1__ ();
  if(!strcmp(parLst[0],"info"))  return GUI_dlg1_info1__ ();
  if(!strcmp(parLst[0],"dlgbe")) return GUI_dlg_dlgbe1__ ();


  L_exit:
    printf("ERROR - unknown dialogtype |%s| \n",parLst[0]);
  return -1;


  L_inpErr:
    printf("***** error input parameter %d .. \n",i1);
    return -1;

}


//================================================================
  int GUI_dlg1_err1 () {
//================================================================
 
  int   i1;

  printf("ERROR GUI_dlg1 error parameters\n");
  for(i1=0; i1<nArg; ++i1) printf(" GUI_file argv[%d]=|%s|\n",i1,parLst[i1]);
  
  exit(1);

}
 
// EOF
