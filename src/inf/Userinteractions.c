/* ../inf/Userinteractions.c
../../doc/gcad_doxygen/Userinteractions.dox


================================================================== */
void INF_INTER_USER(){                   /*! \code



INF_Selections
INF_Keyboard
INF_Block_Input         block, unblock functions
INF_Group               object-group
INF_CoordinateSystems
INF_cursorPosition
INF_ConstructionPlane


================================================================== \endcode */}
void INF_CoordinateSystems (){        /*! code


screenCoords     int, pixels, 2D.
userCoords       double; relativ to the active constrPln
worldCoords      double, absolut usercoords. DB keeps worldCoords.


================================================================== \endcode */}
void INF_cursorPosition (){        /*! code


FUNCTIONS:
GR_curPos_WC get curPos (GR_curPos_WC) in worldCoords
GL_get_curPos_last get Coords of last selection - GL_curPos_WC & GL_curPos_SC
GL_get_curPos_WC get curPos in userCoords
GL_get_curPos_CP__ get curPos in userCoords on GL_view_pln
GL_get_curPos_CP_pt get curPos on WC_sur_act
GR_get_curPos_UC get curPos in userCoords on constrPln as Point
UI_GR_get_actPos_ return string-position of GR_actPos
UI_GR_actPos write GR_actPos using GR_get_curPos_UC
AP_obj_add_curPos add cursor-position as "P(x y z)" to string
UI_GR_Indicate INTERN
GL_ck_sel_PT compare pt with last mouseposition
GL_GetEyeLn get curPos (GL_curPos_WC) & vector to eye (GL_eyeX)
GL_GetEyeX get vector to eye (GL_eyeX)
GL_Get_Cen screencenter in userCoords
GL_get_Scale
UI_GR_SelVert get vertext nearest to cursor
GR_set_curPos_CP set GR_curPos_WC = curPos on WC_sur_act
GL_set_curPos_CP set GL_curPos_SC & GL_curPos_WC (GL_get_curPos_CP__)
GL_SetViewPln set GL_view_pln
VARIABLES:
GR_actPos string curPos in userCoords on constrPln as "P(x y z)"
GR_curPos_WC curPos on WC_sur_act (worldCoords)
GL_curPos_WC Point curPos on GL_view_pln (GL_get_curPos_CP__)
GL_curPos_SC Point curPos in screenkoords (GL_set_curPos_CP)
GL_view_pln Plane X-Y-parallel, RotationCenterplane (GL_SetViewPln)
WC_sur_act Plane
GL_cen Point RotationCenterpoint, always in GL_view_pln
GL_eyeX Vector from the screenCenter to the eye


================================================================== \endcode */}
void INF_ConstructionPlane (){        /*! code


see also Transformations
see also xa.c
Plane WC_sur_act; the ConstructionPlane; in xa.c
int WC_sur_ind; DB-Index of the ActiveConstrPlane; 0=none.
double WC_sur_Z; the active Z-Offset to the ConstructionPlane
char WC_sur_txt[16]; displayed name_of_Constr.Plane; is "DZ" or "R20"
Mat_4x3 WC_sur_mat; // TrMat of ActiveConstrPlane
Mat_4x3 WC_sur_imat; // inverse TrMat of ActiveConstrPlane
Functions ConstrPlane:
DL_setRefSys
NC_setRefsys
DL_GetTrInd
UTRA_pt_ucs2wcs point from constructionplane (relativ) to absolut (UCS -> WCS)
UTRA_pt_wcs2ucs point from absolut to relativ (constructionplane) (WCS -> UCS)
UI_sur_act_CB1 interactive setting of ConstructionPlane
UI_Set_ConstPl_Z write Label name_of_Constr.Plane
AP_Set_ConstPl_Z write Label name_of_Constr.Plane
// UI_suract_keyIn mode=2: set & display WC_sur_Z
AP_Get_ConstPl_Z gives ConstPLn as text or Z-vec
AP_Get_ConstPl_vz give Z-vec of ConstructionPlane


================================================================== \endcode */}
void INF_Keyboard (){        /*! code


// in gCad_main: attach KeyIn
AP_UserKeyIn_get (myKeyCB);
// in gCad_fini:
AP_User_reset (); // reset plugin (incl. keybCB)
// keyboard-callback:
int myKeyCB (int key) {
printf("dia_KeyIn %d\n",key);
switch (key) {
case GUI_KeyEsc: // see ../gui/gui_types.h
gCad_fini ();
break;
case 'q':
printf(" q pressed ..\n");
break;
}
return 0;
}
Example: see Demo_gui_plate.c
see also UI_GL_keys__ (KeybCallback from Grafic-window)


================================================================== \endcode */}
void INF_Selections (){        /*! code


xa_sele.c            selection functions
sele_reset
sele_set__ init and set selectionfilter
sele_set_types add additional objecttypes to selectionfilter
Example:
int my_sel_CB (int src, long dli); // prototype for callback-func
int gCad_main () {
// attach grafic selections to function my_sel_CB
AP_UserSelection_get (my_sel_CB);
// reset & enable selection of Typ_APPOBJ only
sele_set__ (Typ_APPOBJ);
// or reset & enable selection of all types
sele_set__ (Typ_goGeom);
// or enable multiple selectiontypes
sele_reset (); // reset (clear)
sele_set_types (Typ_VAR,
Typ_Val,
Typ_LN, // Length
Typ_CI, // Radius
Typ_TmpPT, // indicate (on ConstrPln)
0);
}
//----------------------------------------------------------------
// callback-Funktion for the selections:
#include "../gr/ut_DL.h" // ..
#include "../ut/ut_geo.h" // Point, TYP_EvMouseL ..
int my_sel_CB (int src, long dli) {
int i1, typ;
long dbi;
char *selNam;
Point p1;
DL_Att att1;
if(src == GUI_MouseR) printf(" right mousebutton pressed ..\n");
if(dli >= 0) goto L_obj_selected;
//----------------------------------------------------------------
// indicate; get the position on the construction-plane
// get pointPosition on object
i1 = sele_get_selPos (&p1);
return 0;
//----------------------------------------------------------------
L_obj_selected:
DL_dlRec__dli (&att1, dli); // get the dispListRecord of the selected obj
printf("selected objectTyp = %d\n",att1.typ);
printf("DB-Index of selected object = %d\n",att1.ind);
UI_GR_get_selNam (&typ, &dbi, &selNam);
printf(" typ=%d dbi=%ld nam=|%s|\n",typ,dbi,selNam);
if(!typ) {
TX_Print(".. select object (line or curve ...)");
return 0; // indicate but object wanted
}
// see DevDoc ResolveObjects/Get_DispListObject \ref Get_DispListObject
..
return 0;
}
//----------------------------------------------------------------
// callback-Funktion for keyboard-input:
int my_key_CB (int key) {
switch (key) {
case GUI_KeyEsc:
case 'q':
gCad_fini ();
}
return 0;
}


================================================================== \endcode */}
void INF_Block_Input (){        /*! code


UI_block__ block / unblock Functions, Input, Cursor
UI_block_input block / unblock keystrokes & grafic_selections
UI_block_group block / unblock groupSelections
UI_GR_block UNUSED
UI_block_glEvents UNUSED


================================================================== \endcode */}
void INF_Group (){        /*! code


Grp_Clear (0); // clear group
Grp_add (typ, dbi, -2L, 1);
Grp_upd (0); // Redraw & update GrpNr-label

================================================================== \endcode */}
// eof
