// ../xa/xa_src.c            2009-05-28           RF
/*
 *
 * Copyright (C) 2015 CADCAM-Services Franz Reiter (franz.reiter@cadcam.co.at)
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
  ..

-----------------------------------------------------
Modifications:
2009-05-28 Created. RF.

-----------------------------------------------------
*/
#ifdef globTag
void SRC(){}
#endif
/*!
\file  ../xa/xa_src.c
\brief check, modify, convert source-objects (text)  (SRC_) 
\code
=====================================================
List_functions_start:

SRC_ck_objDefLn     test sourceLine if its definition-line for typChar

SRC_get_typ         get typ from string, eg from "D(.." or "P21")
SRC_get_oid         get obj-ID from string;

SRC_src_pt3_10      "P(<x> <y> <z>)" from *Point; precision 10 digits
SRC_src_ato         sourceObj (text) from atomicObjs
SRC_src_dbo         create sourceCode of requested type from Database-object
SRC_src_isol_ato1   convert DB-obj (typ, DB-index) into isolated sourceCode (text)

SRC__add_dynTyp     add dynam. type to src; eg Typ_PLN+"P20" -> "R(P20)"

SRC_src_pt_dbo      create obj (src) from point and DB-obj (curve, surface)
AP_src_parPt_selSur create sourceCode for parameteric-point from surface

SRC_parPt_ptDbo     create parametric_point (sourceCode) from point and Db-obj
SRC_LnAc_ptDbo      create L() or C() from obj dbTyp,dbi at position pti

SRC_dump__          dump modelsource

List_functions_end:
=====================================================
see also:
APED_oid_dbo__      make name from typ and DB-index
AP_obj_2_txt
AP_stru_2_txt
AP_obj_add_dbo AP_obj_add_val AP_obj_add_vc AP_obj_add_pt

\endcode *//*----------------------------------------



*/

#ifdef _MSC_VER
#include "../xa/MS_Def0.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>                    // va_list

#include "../ut/ut_geo.h"              // Point ...
#include "../ut/ut_txt.h"              // UTX_db10__
#include "../ut/ut_elli.h"             //
#include "../ut/ut_os.h"

#include "../db/ut_DB.h"               // DB_GetCurv

#include "../xa/xa_sele.h"             // Typ_go*
#include "../ut/ut_memTab.h"           // MemTab
#include "../xa/xa_mem.h"              // memspc51-55
#include "../xa/xa_ato.h"              // ATO_getSpc_tmp__
#include "../xa/xa_obj_txt.h"          // AP_obj_add_dbo



//============ Externe Var: =======================================
// aus xa.c:
extern int       WC_sur_ind;            // Index auf die ActiveConstrPlane
extern Mat_4x3   WC_sur_imat;           // inverse TrMat of ActiveConstrPlane



//================================================================
//================================================================



//===================================================================
  int SRC_ck_objDefLn (long *dbi, char **sSrc, char *sLn, char typ) {
//===================================================================
// SRC_ck_objDefLn       test sourceLine if its definition-line for typChar
//   eg "M20="sm1" .."  is definition-line for typChar 'M'
// Input:
//   sLn     sourceLine   eg "M20="sm1" .."
//   typ     typChar      eg 'M'
// Output:
//   retCode -1  sLn is not definition-line for <typ>
//           0   ok; dbi and sSrc set.
//   dbi     DB-index of object - 20L for "M20"
//   sSrc    position of first char (!=' ') in sLn following '='
//
// see also AP_typ_typChar

  char    *p1, *p2;

  // get model-definitionLine
  p1 = sLn;
  while (*p1 == ' ') ++p1;

  if(*p1 != typ) goto L_tdl_No;

  *dbi = strtol (p1 + 1, &p2, 10);

  if(*p2 == ' ') { while (*p2 == ' ') ++p2; }

  if(*p2 != '=') goto L_tdl_No;

  ++p2;

  while (*p2 == ' ') ++p2;
  *sSrc = p2;
    // printf("ex-SRC_ck_objDefLn %ld |%s|\n",*dbi,p2);
  return 0;


  //----------------------------------------------------------------
  L_tdl_No:
      // printf("ex-SRC_ck_objDefLn -1\n");
    return -1;

}


//================================================================
  int SRC_get_typ (char *src) {
//================================================================
// SRC_get_typ         get typ from string, eg from "D(.." or "P21")
// get requested type from first char
// retCode    typ  0 = error

  int      typ;
  char     *pSrc;


  pSrc = src;
  UTX_pos_skipLeadBlk (pSrc);

  // get typ from first char
  typ = AP_typ_typChar (pSrc[0]);
  if(!typ) goto L_err;

  ++pSrc;
  UTX_pos_skipLeadBlk (pSrc);
  // following char should be numeric (for oid (eg "P21") or '(' for dynam.type
  if(pSrc[0] == '(') goto L_exit;

  // test for numeric (int)
  if(UTX_ck_num_i(pSrc) < 1) goto L_err;


  L_exit:
      // printf("ex-SRC_get_typ %d |%s|\n",typ,src);
    return typ;

  L_err:
    typ = 0;
    printf("***** SRC_get_typ E1 |%s|\n",src);
    goto L_exit;

}


//================================================================
  int SRC_get_oid (long *dbi, char **src) {
//================================================================
// SRC_ck_oid               get obj-ID from string;
// Input:
//   src       startposition of src-obj; 
// Output:
//   src       position of first non-numeric char; unmodified if no obj-ID detected
//   dbi       DB-index
//   returns   type of obj (eg Typ_PT); 0 = no obj-ID;
//
// Example: 
// "A-12 1" returns Typ_SUR, dbi = -12, src-out = " 1"


  int    typ, ii;
  char   *p1, *p2;


  // printf("SRC_get_oid |%s|\n",*src);

  p1 = *src;

  UTX_pos_skipLeadBlk (p1);

  // get typ from first char
  typ = AP_typ_typChar (p1[0]);
  if(!typ) goto L_exit;

  ++p1;

//   // test if cp2 is numeric string
//   ii = UTX_ck_num_digNr (&p2, p1);

  *dbi = strtol (p1, &p2, 10);
  if(p1 == p2) {
    typ = 0;
  } else {
    *src = p2;
  }


  L_exit:
    // printf("ex-SRC_get_oid typ=%d dbi=%ld |%s|\n",typ,*dbi,*src);
  return typ;

}


//================================================================
  int SRC_dump__ (int mode) {
//================================================================
// SRC_dump__       dump modelsource
//   mode   0=into html-file;
//          1=into console

  char   cbuf1[256];
  FILE   *fpo;


    // prepare APTsource: MAN-Mode: copy Edi --> memory.
    AP_SRC_mem_edi ();
    // write mem (UTF_FilBuf0) --> file

    // open temp. html-File
    sprintf(cbuf1, "%stmp.html",AP_get_tmp_dir());

    if(mode == 0) {
      // open html
      UTX_htm_fop (&fpo, cbuf1);
      // cat data -> file
      UTF_wri_f__ (fpo);
      // close
      UTX_htm_fcl (&fpo);
      // disp file
      OS_browse_htm (cbuf1, NULL);

    } else {
      UTF_wri_f__ (stdout);

    } 

  return 0;

}


//========================================================================
  int SRC__add_dynTyp (char *so, int soSiz, int typ, char *si, int mode) {
//========================================================================
// SRC__add_dynTyp                    get sourceObj (eg "P20" -> "R(P20)"
// Input:
//   soSiz       size of so in chars  UU
//   typ         type to get - eg Typ_PLN for example
//   si          obj-source - eg "P20" for example
//   mode        1: display errors; 0: do not display errors
// Output:
//   so
//   retCode     0: ok;  -1: typ not found
// extraced from IE_inpTxtOut

  int       irc = 0, i1, sLen;
  char      *actBuf, s1[80];


  // printf("SRC__add_dynTyp %d |%s|\n",typ,si);

  sLen = strlen(si);
  if(sLen < 1) return -1;

  UTX_pos_skipLeadBlk (si);

  for(i1=0; i1<sLen; ++i1) si[i1] = toupper (si[i1]);
  actBuf = si;
  i1 = IMIN(strlen(si),6);
  strncpy (s1, si, i1);


  // test if more than one work 
  if(UTX_wordnr(si) > 1) s1[0] = '-'; // do not skip enclosing; eg for "D20 REV"

  //----------------------------------------------------------------
  if(typ == Typ_VC) {
    // "C20" -> D(C20)"
    if(s1[0] == 'D') goto L_cpy_txt;
    // eg "DX 2.5" -> "D(DX 2.5)"
    sprintf(so, "D(%s)",actBuf);

  //----------------------------------------------------------------
  } else if(typ == Typ_Tra) {
    // "20" -> "T20"
    if(s1[0] == 'T') goto L_cpy_txt;
    sprintf(so, "T(%s)",actBuf);

  //----------------------------------------------------------------
  } else if(typ == Typ_PT) {
    // "20" -> "P20"
    if(s1[0] == 'P') goto L_cpy_txt;
    sprintf(so, "P(%s)",actBuf);

  //----------------------------------------------------------------
  } else if(typ == Typ_LN) {
    if(s1[0] == 'L') goto L_cpy_txt;
    sprintf(so, "L(%s)",actBuf);

  //----------------------------------------------------------------
  } else if(typ == Typ_CI) {
    if(s1[0] == 'C') goto L_cpy_txt;
    sprintf(so, "C(%s)",actBuf);

  //----------------------------------------------------------------
  } else if(typ == Typ_PLN) {
    // "P20" -> R(P20)"
    if(s1[0] == 'R') goto L_cpy_txt;
    sprintf(so, "R(%s)",actBuf);

  //----------------------------------------------------------------
  } else if(typ == Typ_CV) {
    if(s1[0] == 'S') goto L_cpy_txt;
    sprintf(so, "S(%s)",actBuf);

  //----------------------------------------------------------------
  } else if(typ == Typ_SUR) {
    if(s1[0] == 'A') goto L_cpy_txt;

  //----------------------------------------------------------------
  } else if(typ == Typ_XVal) {
    // first 2 chars must be "X("
    if(!strncmp(si, "X(", 2)) goto L_cpy_txt;  // no modif.
    sprintf(so, "X(%s)",si);

  } else if(typ == Typ_YVal) {
    // first 2 chars must be "Y("
    if(!strncmp(si, "Y(", 2)) goto L_cpy_txt;  // no modif.
    sprintf(so, "Y(%s)",si);

  } else if(typ == Typ_ZVal) {
    // first 2 chars must be "Z("
    if(!strncmp(si, "Z(", 2)) goto L_cpy_txt;  // no modif.
    sprintf(so, "Z(%s)",si);

  //----------------------------------------------------------------
  } else if(typ == Typ_Angle) {
    if(!strncmp(si, "ANG(", 4)) goto L_cpy_txt;  // no modif.
    sprintf(so, "ANG(%s)",si);

  //----------------------------------------------------------------
  } else if(typ == Typ_Val)   {
    if(!strncmp(si, "VAL(", 4)) goto L_cpy_txt;  // no modif.
    sprintf(so, "VAL(%s)",si);

  //----------------------------------------------------------------
  } else if(typ == Typ_PTS)   {
    if(!strncmp(si, "PTS(", 4)) goto L_cpy_txt;  // no modif.
    sprintf(so, "PTS(%s)",si);

  //----------------------------------------------------------------
  } else if(typ == Typ_PTI)   {
    if(!strncmp(si, "PTI(", 4)) goto L_cpy_txt;  // no modif.
    sprintf(so, "PTI(%s)",si);

  //----------------------------------------------------------------
  } else if(typ == Typ_EyePT) {
    // IE_getEyePt (so);
    // goto L_add_mod;
    sprintf(so, "\"%s\"",si);

  //----------------------------------------------------------------
  } else {
    irc = -1;
    if(mode) {
      printf("***************** SRC__add_dynTyp TODO: %d\n",typ);
      TX_Print("***************** SRC__add_dynTyp TODO: %d",typ);
      // see IE_inpTxtOut !
      goto L_cpy_txt;
    }
  }


  //----------------------------------------------------------------
  L_exit:
      // printf(" ex-SRC__add_dynTyp %d |%s|\n",irc,so);
  return irc;


  //----------------------------------------------------------------
  L_cpy_txt:
    strcpy(so, si);
    goto L_exit;

}


//=================================================================
  int SRC_src_pt3_10 (char *s1, Point *pt1) {
//=================================================================
/// \code
/// write struct Point* to string "P(<x> <y> <z>)" precision 10
/// NO leading blank, precision = 10 digits
/// 
// see also AP_obj_add_pt_sp
/// \endcode


  char  *p1;

  // printf("AP_obj_add_pt %f %f %f\n",pt1->x,pt1->y,pt1->z);


  strcpy (s1, "P(");
  s1 += 2;

  p1 = UTX_db10__ (s1, pt1->x);

  *p1 = ' ';
  ++p1;

  p1 = UTX_db10__  (p1, pt1->y);

  *p1 = ' ';
  ++p1;

  p1 = UTX_db10__ (p1, pt1->z);

  strcpy (p1, ")");

    // printf("ex-SRC_src_pt3_10 |%s|\n",s1);

  return 0;

}


//================================================================
  int SRC_src_ato (char *os, int oSiz, int impTyp, ObjAto *ato) {
//================================================================
/// \code
/// SRC_src_ato             sourceObj (text) from atomicObjs
/// replaces SRC_fmt__
/// Input:
///   oSiz        size of os (nr of chars)
///   impTyp      typ of srcObj
/// Output:
///   os          sourceObj eg "P(S# MOD(#) #.)"
///   retCod      0=OK; -1=error
///
/// Example:
/// impTyp    types                       os                          
///  Typ_PT    Typ_PT,Typ_modif,Typ_Val;   "P(S# MOD(#) #.)"
///  Typ_LN    Typ_LN,Typ_modif,Typ_Val;   "L(S# MOD(#))"
///  0;        Typ_Val,Typ_Val,Typ_Val;    "#. #. #."
///
/// see also SRC_src_dbo APT_decode_print
/// \endcode

// was SRC_dbo:
// outTyp     inTyp        outTxt
// PT         Typ_TmpPT    P(curPos)         Y
// LN         Typ_TmpPT
// LN         PT
// VAl        LN           VAL(L)            Y               length


  int       i1, aNr, *atyp, opos, btyp;
  double    *atab, d1;
  char      *sdat, c1;
  void      *vp1;


  // printf("....................................................... \n");
  // printf("SRC_src_ato siz=%d impTyp=%d\n",oSiz,impTyp);
  // ATO_dump__ (ato, "");


  aNr = ato->nr;
  atyp = ato->typ;
  atab = ato->val;
  sdat = ato->txt;
  // ssiz = ato->txsiz;
    // printf(" ato->nr=%d atyp[0]=%d\n",aNr,atyp[0]);


  os[0] = '\0';
  opos = 0;


  if(impTyp) {
    // if impTyp == selTyp skip impTyp; else "P(P20)"  
    // btyp = AP_typDB_typ (atyp[0]);
    // if((aNr == 1) &&(btyp == impTyp)) {
      // APED_oid_dbo__ (os, btyp, (long)atab[0]);   // 2017-03-01
      // goto L_exit;
    if((aNr == 1) &&(atyp[0] == impTyp)) {
      impTyp = 0;

    } else if(impTyp == Typ_PLN) {
      impTyp = 0;

    } else {
//    // if((impTyp == Typ_XVal)||(impTyp == Typ_XVal)||(impTyp == Typ_ZVal))
//    //   impTyp = Typ_PT;
      // add eg "P("
      c1 = AP_typChar_typ (impTyp);
      if(c1 == '-') {TX_Error("SRC_src_ato E1-%d",*atyp); return -1;}
      sprintf (os, "%c(", c1);
    }
  }


  //----------------------------------------------------------------

  for(i1=0; i1<aNr; ++i1) {
    opos += strlen(&os[opos]);
    oSiz -= opos;
    if(oSiz < 32) {TX_Error("SRC_src_ato E2"); return -1;}
      // printf ("  _src_ato [%d] opos=%d oSiz=%d typ=%d tab=%f\n",
              // i1,opos,oSiz,atyp[i1],atab[i1]);

    if(i1) {
      os[opos] = ' ';
      ++opos;
      --oSiz;
    }



    //----------------------------------------------------------------
    if(atyp[i1] == Typ_Val) {
      d1 = UTP_db_comp_0 (atab[i1]);
        // printf(" d1=%lf atab=%lf\n",d1,atab[i1]);
      sprintf (&os[opos], "%lf", d1);
      UTX_del_foll0 (&os[opos]);


    //----------------------------------------------------------------
    } else if(atyp[i1] == Typ_modif) {
      sprintf (&os[opos], "MOD(%d)", (int)atab[i1]);


    //----------------------------------------------------------------
    } else if(atyp[i1] == Typ_PTS) {
      sprintf (&os[opos], "PTS(%d)", (int)atab[i1]);


    //----------------------------------------------------------------
    } else if(atyp[i1] == Typ_PTI) {
      sprintf (&os[opos], "PTI(%d)", (int)atab[i1]);


    //----------------------------------------------------------------
    } else if(atyp[i1] == Typ_SEG) {
      sprintf (&os[opos], "SEG(%d)", (int)atab[i1]);


    //----------------------------------------------------------------
    } else if(atyp[i1] == Typ_TmpPT) {   // P(x y z)
      strcat (&os[opos], "P(");
      UTX_add_fl_10  (&os[opos], atab[i1]);
      strcat (&os[opos], " ");
      ++i1;
      UTX_add_fl_10  (&os[opos], atab[i1]);
      strcat (&os[opos], " ");
      ++i1;
      UTX_add_fl_10  (&os[opos], atab[i1]);
      strcat (&os[opos], ")");


    //----------------------------------------------------------------
    } else if(atyp[i1] == Typ_Txt) {
      strcat (&os[opos], &sdat[(int)atab[i1]]);


    //----------------------------------------------------------------
    } else if(atyp[i1] == Typ_String) {
      strcat (&os[opos], &sdat[(int)atab[i1]]);


    //----------------------------------------------------------------
    } else {
      // get characterID of typ 
      c1 = AP_typChar_typ (atyp[i1]);
      if(c1 == '-') {TX_Error("SRC_src_ato E3-%d",atyp[i1]); return -1;}
      sprintf (&os[opos], "%c%d", c1, (int)atab[i1]);
    }

  }

  if(impTyp) {
    opos += strlen(&os[opos]);
    sprintf (&os[opos], ")");
  }

 
  //----------------------------------------------------------------
  L_exit:
    // printf("ex _src_ato |%s|.............................\n",os);
  return 0;

}


/* REPLACED by SRC_src_pt_dbo
//==============================================================================
  int SRC_LnAc_ptDbo (int mode, char *outBuf, Point *pti, int dbTyp, long dbi) {
//==============================================================================
/// \code
/// create L() or C() from obj dbTyp,dbi at position pti
/// Input:
///   mode   1=line only; 2=line or circ; 3=circ only.
/// \endcode

  
  int        irc=-1, iNr=0, atyp[6], ip, in, ia[3], oTyp;
  double     atab[6], d1;
  ObjGX      oxi;

  // printf("SRC_LnAc_ptDbo mode=%d typ=%d dbi=%ld\n",mode,dbTyp,dbi);

  oxi = DB_GetObjGX (dbTyp, dbi);

  oTyp = Typ_LN;
  if(mode == 3) oTyp = Typ_CI;



  // Plg Clot & CCV:
  if((dbTyp == Typ_CV)    ||
     (dbTyp == Typ_CVPOL) ||
     (dbTyp == Typ_CVTRM))   {

    if((oxi.form == Typ_CVPOL)  ||
       (oxi.typ == Typ_CVTRM))    {

      irc = UT3D_segpar_dboSel (&ip, ia, &in, &d1, dbTyp, dbi);
        // printf(" _segpar_dboSel irc=%d ip=%d in=%d %f\n",irc, ip, in, d1);
      if(irc < 0) {
          printf ("SRC_LnAc_ptDbo objtyp E001 %d\n",dbTyp);
        return -1;
      }

      if(oxi.typ == Typ_CVPOL) {
        if(mode == 3) goto L_err_notLn;
        goto L_par_mod1;   // segment only: L & C both have 1 par 
      }

      if(oxi.typ == Typ_CVTRM) {
        if((mode == 1)&&(irc != Typ_LN)) goto L_err_notLn;
        if((mode == 3)&&(irc != Typ_CI)) goto L_err_notAc;
        oTyp = irc;
        goto L_par_mod1;   // segment only: L & C both have 1 par 
      }


    // connectLine
    } else if(oxi.typ == Typ_CVLNA) {
        // printf(" _LnAc_ptDbo CVLNA !!!\n");
      ULN3_segNr_par_prj_pt_nln (&ia[0], &d1, pti, oxi.siz, oxi.data);
      goto L_par_mod1;   // segment only: L & C both have 1 par 
    }


    return -1;
  }


  L_par_mod1:
  atyp[iNr] = dbTyp;
  atab[iNr] = dbi;
  ++iNr;
  atyp[iNr] = Typ_modif;
  atab[iNr] = ia[0];
  ++iNr;
  goto L_write;


  L_par_mod2:
  atyp[iNr] = dbTyp;
  atab[iNr] = dbi;
  ++iNr;
  atyp[iNr] = Typ_modif;
  atab[iNr] = ia[0];
  ++iNr;
  atyp[iNr] = Typ_modif;
  atab[iNr] = ia[1];
  ++iNr;
  goto L_write;


  // write output
  L_write:
  SRC_fmt__ (outBuf, oTyp, iNr, atyp, atab);
    printf("ex SRC_LnAc_ptDbo |%s|\n", outBuf);


  return 0;



  L_err_notLn:
    TX_Print("no line selected .."); return -1;

  L_err_notAc:
    TX_Print("no circle selected .."); return -1;


}
*/

/* REPLACED with SRC_src_pt_dbo
//==================================================================
  int SRC_vc_ptDbo (char *outBuf, Point *pti, int dbTyp, long dbi) {
//==================================================================
/// \code
/// create vector (sourceCode) from point and Database-object
/// Input:
///   dbTyp, dbi      Database-object
///   pti             point
/// Output:
///   outBuf          output-object (sourceCode)
///   RetCod:
///     0    output complete
///     1    output not yet complete
///    -1    Error
///
///
/// Examples:
/// outBuf            outTyp     inTyp
/// D(C20 0.5)        Typ_PT     Typ_TmpPT 
///
///
/// see IE_cad_sel1           if(typSel == Typ_VAR) {           :3527
/// see SRC_dbo
/// see AP_src_parPt_selSur
/// see AP_src_sel_fmt
/// see AP_stru_2_txt (creates definition-line)
/// \endcode

// get parameter for point on obj.

// UT3D_segpar_dboSel


  int        irc =-1, iNr=0, atyp[6], ip, in, ia[3];
  double     atab[6], d1;
  Point      pto;
  ObjGX      oxi, ox1;
  Memspc     wrkSpc;
  void       *vd;


  // printf("SRC_vc_ptDbo %d %ld\n", dbTyp,dbi);
  // DEB_dump_obj__ (Typ_PT, pti, "  pti");


  // LN
  if(dbTyp == Typ_LN) {
    atyp[iNr] = dbTyp;
    atab[iNr] = dbi;
    ++iNr;
    goto L_write;
  }



  oxi = DB_GetObjGX (dbTyp, dbi);
  if(oxi.typ == Typ_Error) {
    TX_Print("SRC_vc_ptDbo E001 %d %d",dbTyp,dbi);
    return -1;
  }
    // printf(" typ=%d form=%d\n",oxi.typ,oxi.form);



  // Plg Clot & CCV:
  if((dbTyp == Typ_CV)  || 
     (dbTyp == Typ_CVTRM))   {
    irc = UTO__dbo (&vd, dbTyp, dbi);
    if(irc < 0) return -1;
      // printf(" irc=%d\n",irc);
    if((oxi.form == Typ_CVBSP)  ||
       (oxi.form == Typ_CVCLOT))  {
      // irc = UTO_par__pt_obj (&d1, pti, oxi.form, vd);
      irc = UTO_par__pt_obj (&d1, pti, irc, vd);
      if(irc < 0) return -1;
      goto L_par1;                    // D(S parVal)
    }

    if((oxi.form == Typ_CVPOL)  ||
       // (oxi.form == Typ_CVBSP)  ||
       (oxi.typ == Typ_CVTRM))    {
      irc = UT3D_segpar_dboSel (&ip, ia, &in, &d1, dbTyp, dbi);
        // printf(" _dboSel irc=%d ip=%d in=%d d1=%f\n",irc, ip, in, d1);
      if(irc < 0) {
        printf ("SRC_vc_ptDbo objtyp E001 %d\n",dbTyp);
        return -1;
      }

      if((oxi.form == Typ_CVBSP)   ||
         (oxi.form == Typ_CVCLOT))   {
        if(ip >=0) goto L_par_mod1;     // D(S MOD)
        goto L_par1;                    // D(S parVal)
      }
      if(oxi.typ == Typ_CVTRM) goto L_par_mod2;
      goto L_par_mod1;                  // D(S MOD)
    }
    if(oxi.typ == Typ_CVLNA) {
        // printf(" _vc_ptDbo CVLNA !!!\n");
      ULN3_segNr_par_prj_pt_nln (&ia[0], &d1, pti, oxi.siz, oxi.data);
      goto L_par_mod1;   // segment only: L & C both have 1 par 
    }



  } else if((dbTyp == Typ_PLN)      ||
            (dbTyp == Typ_SUR)      ||
            (dbTyp == Typ_Model))   {
    goto L_1obj;
  }


  irc = UTO_par__pt_obj (&d1, pti, oxi.form, oxi.data);
  if(irc < 0) {
    printf ("SRC_vc_ptDbo objtyp not yet supported %d\n",dbTyp);
    return -1;
  }
  goto L_par1;




// see also UTO_par__pt_obj
*
  // // AC
  // if(dbTyp == Typ_CI) {
    // irc = UTO_par__pt_obj (&d1, pti, oxi.form, oxi.data);
    // if(irc < 0) return -1;
    // goto L_par1;
// 
// 
  // } else if(dbTyp == Typ_CV) {
// 
    // if(oxi.form == Typ_CVPOL) {
      // UT3D_par_pt__plg_pt (&d1, pti, oxi.data);
      // goto L_par1;
// 
// 
    // } else if(oxi.form == Typ_CVELL) {
      // d1 = UT3D_angr_elpt (pti, &((CurvElli*)oxi.data)->pc,
                                // &((CurvElli*)oxi.data)->va,
                                // &((CurvElli*)oxi.data)->vb);
        // // printf(" d1-1=%f\n",d1);
      // d1 = UT3D_par1_angr_ell (&d1, (CurvElli*)oxi.data);
        // // printf(" d1-2=%f\n",d1);
      // goto L_par1;
// 
// 
    // } else if(oxi.form == Typ_CVCLOT) {
      // UT3D_par_clotpt (&d1, pti, oxi.data, UT_DISP_cv);
      // goto L_par1;
// 
    // } else if(oxi.form == Typ_CVBSP) {
      // UME_init (&wrkSpc, memspc52, sizeof(memspc52));
      // UT3D_par_pt__pt_cvbsp (&d1, &wrkSpc, oxi.data, pti);
      // goto L_par1;
// 
    // } else {
      // printf("SRC_vc_ptDbo curvetyp not yet supported %d\n",oxi.form);
      // return -1;
    // }
// 
  // } else {
    // printf("SRC_vc_ptDbo objtyp not yet supported %d\n",dbTyp);
    // return -1;
  // }
*

  L_1obj:
  atyp[iNr] = dbTyp;
  atab[iNr] = dbi;
  ++iNr;
  goto L_write;


  L_par1:
  atyp[iNr] = dbTyp;
  atab[iNr] = dbi;
  ++iNr;
  atyp[iNr] = Typ_Val;
  atab[iNr] = d1;
  ++iNr;
  goto L_write;


  L_par_mod1:
  atyp[iNr] = dbTyp;
  atab[iNr] = dbi;
  ++iNr;
  atyp[iNr] = Typ_modif;
  atab[iNr] = ia[0];
  ++iNr;
  goto L_write;


  L_par_mod2:
  atyp[iNr] = dbTyp;
  atab[iNr] = dbi;
  ++iNr;
  atyp[iNr] = Typ_modif;
  atab[iNr] = ia[0];
  ++iNr;
  atyp[iNr] = Typ_modif;
  atab[iNr] = ia[1];
  ++iNr;
  goto L_write;


  // write output
  L_write:
  // SRC_fmt__ (outBuf, Typ_VC, iNr, atyp, atab);
  // SRC_fmt__ (outBuf, Typ_Txt, iNr, atyp, atab);
  // SRC_fmt__ (outBuf, Typ_VC, iNr, atyp, atab);
  SRC_fmt__ (outBuf, Typ_Txt, iNr, atyp, atab);       // 2011-12-14
    // printf("ex SRC_vc_ptDbo |%s|\n", outBuf);


  return 0;

}
*/

/* REPLACED by SRC_src_pt_dbo
//==================================================================
  int SRC_parPt_ptDbo (char *outBuf, Point *pti, int selTyp, long dbi) {
//==================================================================
/// \code
/// create parametric_point (sourceCode) from point and Database-object
/// Input:
///   dbTyp, dbi      Database-object (Ln,Ac,Curv,Surf)
///   pti             point
/// Output:
///   outBuf          output-object (sourceCode); siz ?
///   RetCod:
///     0    output complete
///     1    output not yet complete
///     2    output of nearest selected point; not parametric ..
///    -1    Error
///
///
/// Examples:
/// outBuf                    outTyp     inTyp
/// P(<x> <y> <z>)            Typ_PT     Typ_TmpPT
/// P(<dbo> MOD(#) <par1>)    // from point on CCV
///
///
/// see IE_cad_sel1           if(typSel == Typ_VAR) {           :3527
/// see SRC_dbo
/// see AP_src_parPt_selSur
/// see AP_src_sel_fmt
/// see AP_stru_2_txt (creates definition-line)
/// \endcode


  int       irc = -1, ip, is, ns, dbTyp;
  int       iSeg, iSubSeg, subtypSel, ccvtypSel;
  char      outFmt[16];
  double    dp;


  printf("SRC_parPt_ptDbo  %d %ld\n",selTyp,dbi);
  DEB_dump_obj__ (Typ_PT, pti, "  pti:");


  dbTyp = AP_typDB_typ (selTyp);  // 2013-04-08


  //----------------------------------------------------------------
  if((dbTyp == Typ_LN)     ||
     (dbTyp == Typ_CI)     ||
     (dbTyp == Typ_CV))        {


    // get parameter of point on curve
    irc = SRC_fmt_sub (outBuf, Typ_PT, dbTyp, dbi);
    goto L_exit;


  //----------------------------------------------------------------
  } else if(dbTyp == Typ_SUR) {                // selected: SUR
    irc = AP_src_parPt_selSur (outBuf, pti, dbi);
    goto L_exit;



  //----------------------------------------------------------------
  } else if(dbTyp == Typ_SOL) {                // selected: SOL
    irc = GL_vertex_curPos (pti);
    if(irc == 0) {
      irc = 2;
      outBuf[0] = '\0';
      AP_obj_add_pt (outBuf, pti);
    }
    return 2;
  }


  //----------------------------------------------------------------
  return -1;



  //----------------------------------------------------------------
  L_exit:
    // printf("ex SRC_parPt_ptDbo %d |%s|\n",irc,outBuf);
  return irc;

}
*/


//=======================================================================
  int SRC_src_dbo (char *so, int sMax, int outTyp, int inTyp, long dbi) {
//=======================================================================
/// create sourceCode of requested type from Database-object
// was SRC_dbo




  ObjAto  ato;


  printf("SRC_src_dbo %d %d %ld\n",outTyp,inTyp,dbi);


  ATO_getSpc_tmp__ (&ato, 1);

  ATO_ato_expr_add (&ato, inTyp, (double)dbi, 0);

  return SRC_src_ato (so, sMax, outTyp, &ato);

}


/* replaced by SRC_src_dbo
//================================================================
  int SRC_dbo (char *outBuf, int outTyp, int inTyp, long dbi) {
//================================================================
/// \code
/// create sourceCode of requested type from Database-object
/// Input:
///   inTyp, dbi      Database-object
///   outTyp          requested type of output-object
/// Output:
///   outBuf          output-object (sourceCode)
///   RetCod:
///     0    output complete
///     1    output not yet complete
///    -1    Error
/// 
/// see IE_cad_sel1           if(typSel == Typ_VAR) {           :3527
/// see AP_src_sel_fmt
/// see AP_stru_2_txt (creates definition-line)
/// see APT_decode_print      create sourceObj from atomicObject (direct)
/// \endcode

  int       irc, iNr;
  int       iaNr;
  Point     pt1;
  // char      auxBuf[32], fmt[16];
  // ObjGX     *cv1;
  ObjAto    ato;


  printf("SRC_dbo %d from %d %ld |%s|\n",outTyp,inTyp,dbi,outBuf);


  irc = 0;


  if(strlen(outBuf) < 1) { iNr = 0; goto L_decode0; } 

  // iNr = APT_decode__ (&atyp, &atab, &outBuf);
  iNr = APT_decode__ (&ato, &outBuf);

  


  //----------------------------------------------------------------
  L_decode0:
  //----------------------------------------------------------------
  if(inTyp == Typ_TmpPT) {
    if(outTyp == Typ_PT) goto L_name_0;
    if(outTyp == Typ_LN) goto L_obj_2;          // PT > LN
    if(outTyp == Typ_VC) goto L_obj_2;          // PT > VC = PT, notComplete
    return -1;




  //----------------------------------------------------------------
  } else if(inTyp == Typ_PT) {                // selected: PT
    if(outTyp == Typ_PT) goto L_name_0;         // PT > PT
    if(outTyp == Typ_LN) goto L_obj_2;          // PT > LN
    if(outTyp == Typ_VC) goto L_obj_2;          // PT > VC = PT, notComplete
    if(outTyp == Typ_Val) goto L_obj_2;         // PT > Var = Err
    if(outTyp == Typ_goAxis) goto L_obj_2;      // PT > Axi = PT, notComplete
    return -1;



  //----------------------------------------------------------------
  } else if(inTyp == Typ_LN) {                // selected: LN
    if(outTyp == Typ_PT) goto L_wr_pt;         // PT > PT
    if(outTyp == Typ_VC) goto L_obj_1;          // LN > VC = D(LN)
    if(outTyp == Typ_LN) goto L_obj_2;          // LN > PT+LN
    if(outTyp == Typ_Val) goto L_obj_1;         // LN > Var
    goto L_wr_pt;



  //----------------------------------------------------------------
  } else if(inTyp == Typ_CI) {                        // selected: AC|CI
    // if(outTyp == Typ_VAR) goto L_exit;                // AC > Var = radius
    goto L_wr_pt;




  //----------------------------------------------------------------
  } else if(inTyp == Typ_CV) {                        // selected: CV
    if(outTyp == Typ_VAR) return -1;            // CV > Var = Err
    goto L_wr_pt;



  //----------------------------------------------------------------
  } else if(inTyp == Typ_PLN) {                       // selected: Plane
    if(outTyp == Typ_LN) goto L_obj_2;          // LN > P,R 



  //----------------------------------------------------------------
  } else if((inTyp == Typ_Model)    ||                 // selected: Plane
            (inTyp == Typ_SubModel) ||                   // selected: Plane
            (inTyp == Typ_Mock))       {                 // selected: Plane
    if(outTyp == Typ_PT) goto L_vtx_1;          // M  >  P



  //----------------------------------------------------------------
  } else {
    return -1;
  }



  //========================================================
  L_wr_pt:      // write "P(..)"  fuer LN|AC|Cv|Plg|...   nicht CCV

  irc = SRC_fmt_sub (outBuf, outTyp, inTyp, dbi);


  L_exit:
    // printf("ex SRC_dbo %d |%s|\n",irc,outBuf);

  return irc;



  //----------------------------------------------------------------
  // "<inObj>"
  L_name_0:
      // printf(" L_name_0 %d\n",inTyp);
    if(inTyp == Typ_TmpPT) {
      AP_obj_add_curPos (outBuf);
    } else {
      APED_oid_dbo__ (outBuf, inTyp, dbi);
    }
    goto L_exit;
  


  L_obj_1:   // put in-obj to stack; output "<outTyp>(<inObj>)"
    iNr = 0;
    goto L_obj_a;


  L_obj_2:  
    // 1. obj: direct out ("<outTyp>"); object not yet complete; 
    if (iNr < 1) { irc = 1; goto L_name_0; }
    //second obj: complete.

  
  L_obj_a:   // add in-obj to stack; output "<outTyp>([.. ]<inObj>)"
    ato.typ[iNr] = inTyp;
    ato.val[iNr] = dbi;
    ++iNr;
    SRC_fmt__ (outBuf, outTyp, iNr, ato.typ, ato.val);
    goto L_exit;

  

  L_vtx_1:          // M  >  P
    GL_SelVert__ (&pt1);
      // DEB_dump_obj__ (Typ_PT, &pt1, "sel.Vert:");

    // invert transformation if ConstrPln is set;
    //   will be inverted in APT_decode_pt
    if(CONSTRPLN_IS_ON) {
      UT3D_pt_tra_pt_m3 (&pt1, WC_sur_imat, &pt1);
    }

    AP_obj_add_pt (outBuf, &pt1);
    goto L_exit;

}


//================================================================
  int SRC_fmt_tab (char *s1, int i0, ...) {
//================================================================
// SRC_fmt_tab   write sourceCode formatted for depending obj; eg "D(P1 P2)"
// arguments = pairs of type (int) and values (double).
// LAST ARGUMENT (type) MUST BE 0 !
// Example:
// SRC_fmt (outBuf, Typ_XVal, Typ_PT, d1, 0);

  int     atyp[6], aNr;
  double  atab[6];
  va_list va;

  // printf("SRC_fmt_tab %d\n",i0);

  va_start (va, i0);
  aNr = 0;

  L_nxt_arg:
    atyp[aNr] = va_arg (va, int);
      // printf("       _stat_set__ %d\n",i1);
    if(atyp[aNr] == 0) {
      va_end (va);
      SRC_fmt__ (s1, i0, aNr, atyp, atab);
      return 0;
    }
    atab[aNr] = va_arg (va, double);
      // printf(" aNr=%d atyp=%d atab=%f\n",aNr,atyp[aNr],atab[aNr]);
    ++aNr;
    if(aNr < 6) goto L_nxt_arg;


  printf("***** SRC_fmt_tab E001 \n");
  return -1;
}


//=============================================================================
  int SRC_fmt__ (char *outBuf, int outTyp, int aNr, int *atyp, double *atab) {
//=============================================================================
/// \code
/// DO NOT USE; use new func SRC_src_ato
///  write sourceCode formatted for depending obj; eg "D(P1 P2)"
/// Input:
///   outTyp    primary typ; Typ_Txt: none.
///   aNr       nr of records in atyp/atab
///   atyp      secondary object-types (in bracket)
///   atab      db-indices of secondary objects
/// see also SRC_src_ato
/// \endcode


  int   i1, i2;
  long  dbi;
  char  *p1, s1[32];


  // printf("SRC_fmt__ ityp=%d aNr=%d\n",outTyp,aNr);
  // for(i1=0; i1<aNr; ++i1) printf(" %d typ=%d tab=%f\n",i1,atyp[i1],atab[i1]);


  if(outTyp == Typ_Val) {
    strcpy (outBuf, "VAL(");
    p1 = &outBuf[4];


  } else if(outTyp == Typ_VC) {
    strcpy (outBuf, "D(");
    p1 = &outBuf[2];


  } else if(outTyp == Typ_XVal) {
    strcpy (outBuf, "X(");
    p1 = &outBuf[2];


  } else if(outTyp == Typ_Txt) {      // no implicit type
    outBuf[0] = '\0';
    p1 = &outBuf[0];


  } else {
    outBuf[0] = AP_typChar_typ (outTyp);
    outBuf[1] = '(';
    outBuf[2] = '\0';
    p1 = &outBuf[2];
  }


  // add objs of atyp/atab - records
  for(i1=0; i1<aNr; ++i1) {

    if(atyp[i1] == Typ_TmpPT) {
      AP_obj_add_curPos (p1);
      continue;
    }

    if(atyp[i1] == Typ_Val) {
      AP_obj_add_val (p1, atab[i1]);
      p1 = &p1[strlen(p1)];
      continue;
    }

    if(atyp[i1] == Typ_modif) {
      i2 = atab[i1];
      sprintf(s1, " MOD(%d)", i2);
      strcat(p1, s1);
      p1 = &p1[strlen(p1)];
      continue;
    }


    dbi = atab[i1];
    if(dbi < 0) {           // P-9; from P(0 0)
      SRC_src_isol_ato1 (p1, atyp[i1], &atab[i1]);
    } else {
      // add objID
      AP_obj_add_dbo (p1, atyp[i1], dbi);
    }
  }
 

  if(outTyp != Typ_Txt)      // no implicit type
    strcat (p1, ")");

  // printf("ex SRC_fmt__ |%s|\n",outBuf);

 
  return 0;

}
*/

//================================================================
  int SRC_src_isol_ato1 (char *outBuf, int typ, double *val) {
//================================================================
/// \code
/// convert obj (typ,val) into isolated sourceCode (text)
/// Input:
///   typ,val     obj to convert
/// Output:
///   outBuf      src-representation of obj (typ,val)
///
/// Examples:
/// "P-9" -> "P(0 0 0)"
/// "D19" -> "D(0 1 0)"
/// "V1"  -> "12.3"
/// Typ_Val|Typ_XVal|Typ_YVal|Typ_ZVal: "12.3"
/// \endcode

  void   *vp;
  long   dbi;

  // printf("SRC_src_isol_ato1 typ=%d  val=%f\n",typ,*val);

  outBuf[0] = '\0';

  switch (typ) {

    case Typ_Val:
    case Typ_XVal:     // X(P#)
    case Typ_YVal:
    case Typ_ZVal:
      AP_obj_add_val (outBuf, *val); // make "val" from value
      break;

    case Typ_VAR:
      dbi = *val;
      vp = DB_get_Var (dbi);
      AP_obj_add_val (outBuf, *val); // make "P(x y z)" from db-point
      break;

    case Typ_PT:
      dbi = *val;
      vp = DB_get_PT (dbi);
      AP_obj_add_pt (outBuf, vp); // make "P(x y z)" from db-point
      break;

    case Typ_VC:
      dbi = *val;
      vp = DB_get_VC (dbi);
      AP_obj_add_vc (outBuf, vp); // make "D(x y z)" from db-vector
      break;

    default:
      TX_Error("*** SRC_src_isol_ato1 unsupported %d ***",typ);
      return -1;
  }

    // printf("ex SRC_src_isol_ato1 |%s|\n",outBuf);

  return 0;

}

/* UNUSED
//================================================================
  int SRC_fmt_sub (char *outBuf, int outTyp, int inTyp, long dbi) {
//================================================================
/// format sourceObj (text) for compound-DB-obj

  int       irc, iSeg, iSubSeg, sSel, ccvtypSel;
  int       subtypSel, iaSeg[3], iaNr;
  double    lpar;
  char      fmt[16];
  ObjGX     *cv1;


  // printf("SRC_fmt_sub %d %ld\n",inTyp,dbi);

  irc = 0;


  //========================================================
  L_wr_pt:      // write "P(..)"  fuer LN|AC|Cv|Plg|...   nicht CCV
    // printf("  L_wr_pt\n");
  // Output: f. LN|AC|Cv:  P(L21 lpar)
  //        od f. Plg: P(S21 MOD(1) lpar)   od P(S21 lpar)
  strcpy(fmt, "---ox");
  // copy the typeSpecificCharacter -> fmt[2]
  fmt[2] = AP_typChar_typ (outTyp);
    // printf(" fmt=|%s|\n",fmt);

  // if((outTyp == Typ_VC)) .. strcpy(fmt, "pPPoix")...
  // goto L_write;


  //========================================================
  L_write:     // write "P(selObj  MOD(segwNr) offset)
  // AP_srcPt_selObj (outBuf, inTyp, dbi, fmt);

// Output:
//   outBuf      zB "P(S25 MOD(2))"

// Input:
//   inTyp        typ of sel. Object (Typ_LN|Typ_CI|Typ_CV(Elli,Bspl,CCV)
//   dbi        DB-indextyp of sel. Object (eg.10)
//   oNam        name of sel. Object (eg."L10")
//   fmt         OutputFormatDescription;  MUST NOT BE CONST !

// write "P(selObj  MOD(segwNr) offset)
// write P(L21 MOD(1)) od P(L21 lpar) od  P(L21 MOD(1) lpar);
// fmt[0-1] definieren eine zusaetzliche ResolvFunktion:
// 0.char  what to resolve;
//         p (Point) or
//         l (Line)
//         v (Vector)                          NOT YET IMPLEMENTED
// 1.char  typ of input for ResolFunction:
//         o  obj (L,C,S)
//         p  Polygon
//         P  CCV-Polygon
//         -  dont know (change later to o|p|P)
// fmt[2-6] beschreiben das OutputFormat:
// 2.char     'P|L|D|-(nix)|c(ccvtypSel)  --> "P("
// 3.char     'o' = ObjName zB "S20"
// 4.char     'x' = unknown (prepare Format dynamic)
// 4/5/6.char 'i' = iSeg als MOD(<iSeg>) od
//            'I' = iSubSeg als MOD(<iSubSeg>) od
//            'p' = lpar ausgeben

// see IE_cad_sel1

  subtypSel = 0;  // Typ_Error
  ccvtypSel = 0;
  iSeg      = -1;
  iSubSeg   = -1;


  // wenn eine Curve selektiert wurde, noch den Subtyp -> subtypSel
  // CCV kommt schon als CCV, aber Polygon u. Bspl kommen als Typ_CV.
  if(inTyp == Typ_CV) {
    cv1 = DB_GetCurv (dbi);
    subtypSel = cv1->typ;
    // inTyp = subtypSel;
      // printf(" Curve: subtypSel=%d\n",subtypSel);
  }


  // den subTyp suchen - fuer L|C|Cv ..
  if((inTyp == Typ_LN)        ||
     (inTyp == Typ_CI)        ||
     (inTyp == Typ_CV))            {
     // (inTyp == Typ_CVPOL)     ||
     // (inTyp == Typ_CVPOL2)    ||
     // (inTyp == Typ_CVBSP)     ||
     // (inTyp == Typ_CVRBSP)    ||
     // (inTyp == Typ_CVELL)     ||
     // (inTyp == Typ_CVTRM)     ||
     // (inTyp == Typ_CVCLOT))      {

    // // den dem Cursor naechsten vertex holen ..
    // GL_SelVert__ (&pt1);
      // DEB_dump_obj__ (Typ_PT, &pt1, "sel.Vert:");

    // das Polygon der selektierten CCV holen
    // den selektieren Punkt auf der Curve feststellen (segmentNr und Abstand)
    // Segmentnummer und Abstand in iSeg und einen Laengsparameter umwandeln
    ccvtypSel = UT3D_segpar_dboSel (&sSel, iaSeg, &iaNr, &lpar, inTyp, dbi);
      // printf("L0: ccvtypSel=%d sSel=%d iSeg=%d lpar=%f\n",
             // ccvtypSel,sSel,iSeg,lpar);
    if(ccvtypSel < 0) return -1;
    iSeg = iaSeg[0];
    if(iaNr > 1) iSubSeg = iaSeg[1];
  }
  // printf("L1: inTyp=%d subtypSel=%d ccvtypSel=%d iSeg=%d iSubSeg=%d lpar=%f\n",
         // inTyp,subtypSel,ccvtypSel,iSeg,iSubSeg,lpar);
  // printf(" fmt=|%s|\n",fmt);


  //========================================================
  // L_write:     // write "P(selObj  MOD(segwNr) offset)
    // printf("  L_write |%s| iSeg=%d iSubSeg=%d lpar=%f\n",
           // fmt,iSeg,iSubSeg,lpar);


  // create sourceText from Format.
  AP_src_sel_fmt (outBuf, sSel, inTyp, dbi, subtypSel, ccvtypSel,
                  iSeg, iSubSeg, lpar, fmt);



  L_exit:
    // printf("ex SRC_fmt_sub %d |%s|\n",irc,outBuf);

  return irc;

}
*/

/* UNUSED
//================================================================
  int AP_src_sel_fmt (char *outBuf, int sSel, int typSel, long indSel,
                      int subtypSel, int ccvtypSel,
                      int iSeg, int iSubSeg, double lpar, char *fmt) {
//================================================================
/// \code
/// format sourceObj (text) for selected compound-obj
/// 
/// Output:
///   outBuf   sourceText
/// Input:
///   sSel    -1  no limitPoint (Startpt|Endpt|plgPoint) selected
///          >=0  Startpt|Endpt|plgPoint selected;
///   typSel      typ of selected obj
///   indSel      DB-ind of selected obj
///   subtypSel   type of curve if typSel=Typ_CV
///   ccvtypSel   typ of selected obj inside CCV if typSel=Typ_CVTRM
///   iSeg        segmentNr in Polygon or in CCV
///   iSubSeg     segmentNr of Polygon inside CCV
///   lpar        Curveparameter 0-1 if sSel<0
///   fmt         outputformat:  zB "poPox" f PT from LN
///     0.char  -
///         unused
///     1.char  -
///         unused
///     Characters 2-6 beschreiben das OutputFormat:
///     2.char     'P|L|D|'
///         P = P()
///         L = L()
///         D = D()  (Vector)
///         c = typ of ccvtypSel (ccvtypSel replaces this char)
///     3.char     'o' = Obj (cbuf) zB "S20"
///     4.char     'x' = prepare Format
///     4/5/6.char 'i' = iSeg als MOD(<iSeg>) od
///                'I' = iSubSeg als MOD(<iSubSeg>) od
///                'p' = lpar
/// \endcode


  int   irc, i2;
  char  *p2, auxBuf[32];


  // printf("AP_src_sel_fmt sSel=%d typSel=%d |%s|\n",sSel,typSel,fmt);
  // printf("  subtypSel=%d ccvtypSel=%d\n",subtypSel,ccvtypSel);
  // printf("  iSeg=%d iSubSeg=%d lpar=%f\n",iSeg,iSubSeg,lpar);


  //----------------------------------------------------------------
  // 2-6 beschreiben das OutputFormat
  // fix dynamic outFormat 'c'
  if(fmt[2] == 'c') {    // c=ccvtypSel
    if(ccvtypSel > 0) i2 = ccvtypSel;
    else              i2 = typSel;
    fmt[2] = AP_typChar_typ(i2);   // L od C od S
  }


  // test if last char is 'x'
  // if(fmt[strlen(fmt) - 1] != 'x') goto L_wr_out;
  p2 = strchr(&fmt[3], 'x');
  if(p2 == NULL) goto L_wr_out;


  // for D|L lPar is not useful
  if(fmt[2] != 'P') sSel = -1;


  // for pointOutput only: fix dynamic format "x" to "p|ip|i|iI"
  // point or segment
  if(fmt[2] == 'P') {        // P()
    if(sSel < 0) {               // no point selectes, "p|ip"
      if((subtypSel != Typ_CVTRM) &&
         (subtypSel != Typ_CVLNA))   strcat(p2, "p");
      else                           strcat(p2, "ip");

    } else {                      // point was selected; '"|iI"
      if(subtypSel != Typ_CVTRM)  strcat(p2, "i");
      else                        strcat(p2, "iI");
    }


  // for L|D-Output only: fix dynamic format "x" to "i|iI"
  } else {
    if(subtypSel == Typ_CVTRM) {
      // plg in CCV needs "iI", all others "i"
      if(ccvtypSel == Typ_CVPOL) {
        strcat(p2, "iI");
      } else {
        strcat(p2, "i");
      }
    } else {
      // Plg needs "i"; all others ""
      if(subtypSel == Typ_CVPOL) {
        strcat(p2, "i");
      }
    }
  }


    // printf(" fmt fixed |%s|\n",fmt);



  //----------------------------------------------------------------
  // add outFormat eg 'P('
  L_wr_out:
  if(fmt[2] != '-') {
    outBuf[0] = fmt[2];
    outBuf[1] = '(';
    outBuf[2] = '\0';
  } else {
    outBuf[0] = '\0';
  }


  if(fmt[3] == 'o') {
    irc = APED_oid_dbo__ (auxBuf, typSel, indSel);
    strcat(outBuf, auxBuf);
  }


  // add data i|I|p
  for(i2=4; i2<7; ++i2) {
    if(fmt[i2] == '\0') break;

    if(fmt[i2] == 'i') {
      sprintf(auxBuf, " MOD(%d)",iSeg);
      strcat(outBuf, auxBuf);

    } else if(fmt[i2] == 'I') {
      sprintf(auxBuf, " MOD(%d)",iSubSeg);
      strcat(outBuf, auxBuf);

    } else if(fmt[i2] == 'p') {
      strcat(outBuf, " ");
      UTX_add_fl_u (outBuf, lpar);
    }
  }

  if(fmt[2] != '-') {
    strcat(outBuf, ")");
  }
  // printf(" outBuf out |%s|\n",outBuf);

  return 0;

}
*/
/*
//====================================================================
  int AP_src_selObj (char *outBuf, int typReq, int sTyp, long sInd) {
//====================================================================
/// \code
/// AP_src_selObj      create sourceObj (text) for selected DB-obj
/// Input:
///   sTyp     inputtyp: Typ_LN|Typ_CI|Typ_CV(Elli,Bspl)|Typ_CVTRM
///   sInd     DB-index
/// Output:
///   outBuf   resulting modelcode
///   Retod    0=OK; -1=Error
/// 
/// see also IE_cad_sel1 :3438
///
/// DO NOT USE: use new SRC_dbo
/// \endcode

  // int       irc, subtypSel, ccvtypSel, iSeg, sSel;

  int       irc, i2, iSeg, iSubSeg, sSel, ccvtypSel;
  int       subtypSel, iaSeg[3], iaNr;
  double    lpar, d1;
  char      auxBuf[32], fmt[16], *ps;
  ObjGX     *cv1, oxo;


  printf("AP_src_selObj typReq=%d sTyp=%d sInd=%d\n",typReq,sTyp,sInd);


  if(sTyp != Typ_TmpPT)
    APED_oid_dbo__ (outBuf, sTyp, sInd);



  //----------------------------------------------------------------
  if(sTyp == Typ_TmpPT) {
    if(typReq == Typ_PT) {
      UI_GR_get_actPos_ (&ps);
      strcpy(outBuf, ps);
      goto L_exit;

    } else return -1;




  //----------------------------------------------------------------
  } else if(sTyp == Typ_PT) {                        // selected: PT
    if(typReq == Typ_PT) goto L_exit;    // PT > PT
    if(typReq == Typ_VC)  return -1;     // PT > VC = Err
    if(typReq == Typ_VAR) return -1;     // PT > Var = Err
    return -1;



  //----------------------------------------------------------------
  } else if(sTyp == Typ_LN) {                        // selected: LN
    if(typReq == Typ_VAR) return -1;   // LN > Var = Err
    goto L_wr_pt;



  //----------------------------------------------------------------
  } else if(sTyp == Typ_CI) {                        // selected: AC|CI
    if(typReq == Typ_VAR) goto L_exit;    // AC > Var = radius
    goto L_wr_pt;




  //----------------------------------------------------------------
  } else if(sTyp == Typ_CV) {                        // selected: CV
    if(typReq == Typ_VAR) return -1;   // CV > Var = Err
    goto L_wr_pt;
  }



  //========================================================
  L_wr_pt:      // write "P(..)"  fuer LN|AC|Cv|Plg|...   nicht CCV

  irc = SRC_fmt_sub (outBuf, typReq, sTyp, sInd);



  L_exit:

    printf("ex AP_src_selObj |%s|\n",outBuf);

  return 0;

}


//================================================================
  int AP_src_parPt_selSur (char *cBuf, Point *pti, long iSur) {
//================================================================
/// \code
/// AP_src_parPt_selSur    create sourceCode for parametric-point from surface
/// 
/// Input:
///   iSur          DB-index of sur
/// Output:
///   cBuf          eg "P(A# <uPar> <vPar>)"
/// 
/// see UT3D_segpar_dboSel AP_src_sel_fmt
/// \endcode

  int       irc;
  Point     pt2;
  // Point2    pt2;
  ObjGX     *ox1p;


  // printf("AP_src_parPt_selSur %d\n",iSur);


  // get sur
  ox1p = DB_GetSur (iSur, 0);


  // get get 2D-parameters of 3D-point pt1
  irc = SUR_pt2_prjptsur (&pt2, pti, ox1p);
  if(irc < 0) return irc;


  // create "P(A# <uPar> <vPar>)"
  sprintf(cBuf, "P(A%ld ", iSur);
  UTX_add_fl_u (cBuf, pt2.x);
  strcat(cBuf, " ");
  UTX_add_fl_u (cBuf, pt2.y);
  strcat(cBuf, ")");

    // printf("ex AP_src_parPt_selSur |%s|\n",cBuf);

  return 0;

}
*/


//========================================================================
  int SRC_src_pt_dbo (char *so, int sSiz,
                      int oTyp, Point *pti, int iTyp, long iDbi) {
//========================================================================
/// \code
/// SRC_src_pt_dbo    create obj (src) from point and DB-obj (curve, surface)
///   create src-obj of type <oTyp> from selected obj <iTyp,iDbi> and
///   select-position <pti>
/// Input:
///   oTyp            type of outputobj; Typ_PT|Typ_VC|Typ_LN|Typ_CI
///                   Typ_goGeo1 - get L|C|S according to selected obj
///   pti             point on obj; get last selPos if NULL.
///   iTyp,iDbi       object (Ln,Ac,Curv,Surf) 
///   sSiz            size of so (nr of chars)
/// Output:
///   so              output-object (sourceCode of point on obj)
///   RetCod:
///     >0            typ of created object (if oTyp=Typ_goGeo1)
///                   TODO: typ of basic-curve for trimmed-curve sele_ck_subCurv
///      0            output of nearest selected point; not parametric ..
///     -1            Error
///     -2            cannot create oTyp from iTyp,iDbi
///
/// Examples:
/// outBuf                    outTyp     inTyp
/// P(<x> <y> <z>)            Typ_PT     Typ_TmpPT
/// P(<dbo> MOD(#) <par1>)    Typ_PT     CCV with more than one components
/// P(<dbo> <par1>)           Typ_PT     single trimmed curve
///
///
/// see IE_cad_sel1           if(typSel == Typ_VAR) {           :3527
/// see SRC_dbo
/// see AP_src_parPt_selSur
/// see AP_src_sel_fmt
/// see AP_stru_2_txt (creates definition-line)
/// \endcode


// was SRC_LnAc_ptDbo
// TODO:
// soll ein irc = uncomplete liefern
// zB fuer L = P    // soll: L = P P
// zB fuer D = P    // soll: D = P P
// andere Beispiele:
// ANG = L L
// Example: LinPat Vector



  int       irc, i1, i2, iNr;
  ObjAto    ato;
  CurvCCV   *ccva;
  void      *iObj;
  char      o2[OBJ_SIZ_MAX];
  Point     pts;


  // printf("-------------------------------------------------------------- \n");
  // printf("SRC_src_pt_dbo siz=%d oTyp=%d iTyp=%d iDbi=%ld\n",sSiz,oTyp,iTyp,iDbi);


  // f pti == NULL: get last selected position
  if(!pti) {
    // sele_get_pos__ (&pts);
    sele_get_pos_vtx (&pts);
    pti = &pts;
  }
    // DEB_dump_obj__(Typ_PT, pti, " SRC_src_pt_dbo-pti");


  // get tempSpc for ato (with max 6 records)
  ATO_getSpc_tmp__ (&ato, 6);


  //---------------------------------------------------------------
  if(iTyp == Typ_PT) {                // selected: PT
    ATO_ato_expr_add (&ato, iTyp, (double)iDbi, 0); // add PT
    goto L_encode;
  }

  if(iTyp == Typ_LN) { 
    if(oTyp == Typ_CI) return -2;
  }


  //---------------------------------------------------------------
  // get selected obj
  // get objData of DB-obj (iTyp,iDbi)
  irc = UTO_obj_dbo (&iObj, &iNr, &iTyp, iDbi);
    // printf(" src_pt_dbo-irc=%d iTyp=%d iNr=%d\n",irc,iTyp,iNr);
  if(irc < 0) {
    TX_Print ("**** cannot analyze DB-obj");
    return -1;
  }
    // DEB_dump_obj__ (iTyp, iObj, " SRC_src_pt_dbo-iObj");


  //----------------------------------------------------------------
  if(iTyp == Typ_SOL) {                // selected: SUR
    printf("*** get parameteric point on solid is not yet implemented ***\n");
    // see also Typ_TmpPT,
    ATO_ato_expr_add (&ato, Typ_Val, pti->x, 0);
    ATO_ato_expr_add (&ato, Typ_Val, pti->y, 0);
    ATO_ato_expr_add (&ato, Typ_Val, pti->z, 0);
    goto L_encode;
  }


  //----------------------------------------------------------------
  // add primary obj to atomicObjs
  ATO_ato_expr_add (&ato, iTyp, (double)iDbi, 0);



  //----------------------------------------------------------------
  if(iTyp == Typ_SUR) {                // selected: SUR
    // irc = AP_src_parPt_selSur (outBuf, pti, dbi);

    if(oTyp == Typ_VC) return -2;
    if(oTyp == Typ_LN) return -2;
    if(oTyp == Typ_CI) return -2;

//     // get 2D-parameters of 3D-point pti in pts
//     irc = SUR_pt2_prjptsur (&pts, pti, iObj);
//     if(irc < 0) {
//       printf("*** get parameteric point on this surfTyp is not yet implemented ***\n");
      // so = get vertex from selected position
      ATO_clear__ (&ato);
      ATO_ato_expr_add (&ato, Typ_Val, pti->x, 0);
      ATO_ato_expr_add (&ato, Typ_Val, pti->y, 0);
      ATO_ato_expr_add (&ato, Typ_Val, pti->z, 0);
      goto L_encode;
//     }
//     ATO_ato_expr_add (&ato, Typ_Val, pts.x, 0);
//     ATO_ato_expr_add (&ato, Typ_Val, pts.y, 0);
//     goto L_encode;


  //----------------------------------------------------------------
  // resolv CCV
  } else if(iTyp == Typ_CVTRM) {
      // printf(" Typ_CVTRM-siz=%d\n",iNr);
    ccva = iObj;

    if(iNr == 1) {
      // only one trimmedCurve; eg "S(S20)" - make "S21"
      if((oTyp == Typ_goGeo1)||(oTyp == Typ_CV)) {
        APED_oid_dbo__ (so, Typ_CV, iDbi);
        goto L_exit;
      }
    }

    // loop tru CCV
    i2 = 0;   // do not output segmenNr (SEG) if only one obj in CCV
    for(i1=0; i1<iNr; ++i1) {
        // DEB_dump_obj__ (Typ_CVTRM, &ccva[i1], " ccv[%d]",i1);
      iTyp = ccva[i1].typ;
      // get o2=basicCurve from trimmedCurve
      irc = UTO_cv_cvtrm (&iTyp, o2, NULL, &ccva[i1]);
      if(irc < 0) return -1;
      // get parametric position of point on obj o2
      if(iNr > 1) i2 = i1 + 1;  // set segmenNr (SEG)
      irc = ATO_ato_obj_pt (&ato, oTyp, i2, iTyp, o2, pti);
        // printf(" ato_obj_pt irc=%d seg-%d\n",irc,i1+1);
      if(irc == 0) break;
      if(irc == -2) { oTyp = -2; so[0] = 0; goto L_exit;} // no conversion possible
    }
    if(irc < 0) {
      // no segment identified ..
      printf("***** ex SRC_src_pt_dbo E-3\n");
      return -3;
    }

      // ATO_dump__ (&ato, " rc_pt_dbo-5");
      
    // L|C -> S(contour MOD(segNr))    but keep P,D
    if(oTyp == Typ_goGeo1) oTyp = Typ_CV;
    



  //----------------------------------------------------------------
  } else {
    // curve - not CCV
    // get all atomicObjs for selection;
    // get parametric position of point on obj
    irc = ATO_ato_obj_pt (&ato, oTyp, 0, iTyp, iObj, pti);
      // ATO_dump__ (&ato, " rc_pt_dbo-6");
    if(irc < 0) {
      printf(" SRC_src_pt_dbo E-1\n");
      return -1;
    }
  }



  //----------------------------------------------------------------
  // for (oTyp==Typ_goGeo1) fix oTyp
  if(oTyp == Typ_goGeo1) {

    // get the selected subObj of a CCV
      // printf(" iTyp=%d\n",iTyp);
    if((iTyp == Typ_LN)||(iTyp == Typ_CVPOL)) {
      oTyp = Typ_CV; // Typ_LN;

    } else if(iTyp == Typ_CI) {
      oTyp = Typ_CI;
    // } else return -2;

    } else {
      oTyp =  AP_typDB_typ (iTyp);
    }
  }



  //----------------------------------------------------------------
  L_encode:
    // printf(" L_encode: oTyp=%d\n",oTyp);
    // ATO_dump__ (&ato, " L_encode:");

  // get sourceObj for outTyp from atomicObjs in ato
  irc = SRC_src_ato (so, sSiz, oTyp, &ato);
  if(irc < 0) return -1;


  L_exit:

    // printf("  ex-SRC_src_pt_dbo %d |%s|---------------------------\n",oTyp,so);

  return oTyp;


}

/*
//====================================================================
  int APED_oid_dbo__ (char *buf, int typ, long ind) {
//====================================================================
/// \code
/// make name from typ and DB-index  (APED_oid_dbo_all)
/// see also AP_typDB_typ AP_cre_defHdr
/// \endcode

// FuncNam should be APED_oid_dbo_all

  char   *p1;

  // printf("APED_oid_dbo__ %d %d\n",typ,ind);


  if(       (typ == Typ_PT)    ||
            (typ == Typ_PT2)   ||
            (typ == Typ_TmpPT))      {
    sprintf(buf, "P%ld", ind);
    

  } else if((typ == Typ_LN)    ||
            (typ == Typ_LN2))        {
    sprintf(buf, "L%ld", ind);
    

  } else if((typ == Typ_CI)    ||
            (typ == Typ_CI2))        {
    sprintf(buf, "C%ld", ind);
    

  } else if(typ == Typ_PLN) {
    if(ind >= 0) sprintf(buf, "R%ld", ind);
    else {
      if     (ind == DB_PLX_IND) sprintf(buf, "RX");
      else if(ind == DB_PLY_IND) sprintf(buf, "RY");
      else if(ind == DB_PLZ_IND) sprintf(buf, "RZ");
    }
    

  } else if(typ == Typ_VC) {
    if(ind >= 0) sprintf(buf, "D%ld", ind);
    else {
      if     (ind == DB_VCX_IND) sprintf(buf, "DX");
      else if(ind == DB_VCY_IND) sprintf(buf, "DY");
      else if(ind == DB_VCZ_IND) sprintf(buf, "DZ");
      else if(ind == DB_VCIX_IND) sprintf(buf, "DIX");
      else if(ind == DB_VCIY_IND) sprintf(buf, "DIY");
      else if(ind == DB_VCIZ_IND) sprintf(buf, "DIZ");
      else sprintf(buf, "D%ld", ind);
    }


  } else if((typ == Typ_CV)      ||
            (typ == Typ_CVPOL)   ||
            (typ == Typ_CVBSP)   ||
            (typ == Typ_CVRBSP)  ||
            (typ == Typ_CVELL)   ||
            (typ == Typ_CVTRM))      {
    sprintf(buf, "S%ld", ind);
    

  } else if((typ == Typ_SUR)      ||
            (typ == Typ_SURRU)    ||
            (typ == Typ_SURRV)    ||
            (typ == Typ_SURCIR)   ||
            (typ == Typ_SURSTRIP) ||
            (typ == Typ_SURBSP)   ||
            (typ == Typ_SURRBSP)  ||
            (typ == Typ_SURPTAB)  ||
            (typ == Typ_SURPMSH))    {
    sprintf(buf, "A%ld", ind);
    

  } else if((typ == Typ_SOL)   ||
            (typ == Typ_CON)   ||
            (typ == Typ_TOR))        {
    sprintf(buf, "B%ld", ind);
    

  } else if((typ == Typ_Note)  ||
            (typ == Typ_ATXT)  ||
            (typ == Typ_GTXT)  ||
            (typ == Typ_Dimen) ||
            (typ == Typ_Tag))         {
    sprintf(buf, "N%ld", ind);
    

  } else if((typ == Typ_Model) ||
            (typ == Typ_Mock))        {
    sprintf(buf, "M%ld", ind);


  } else if(typ == Typ_Val) {
    //sprintf (buf,"%f,%f",pt1.x,pt1.y);
    //strcpy(buf, ",");
    buf[0] = '\0';
    return -1;
    // UTX_add_fl_u2 (buf, pt1.x,pt1.y);
    

  } else if(typ == Typ_VAR) {  // hat hoechste Select-Prioritaet in GL
    sprintf(buf, "V%ld", ind);


  } else if(typ == Typ_Tra) {  // Transformation
    sprintf(buf, "T%ld", ind);


  } else if(typ == Typ_Activ) {  // Activity
    sprintf(buf, "I%ld", ind);


  } else if(typ == Typ_goGeom) {  // undefined
    sprintf(buf, "U%ld", ind);



  } else if(typ == Typ_SymB) {  // temporary-SymBmp
    sprintf(buf, "Sym%ld", ind);


  } else if(typ == Typ_APPOBJ) {  // ApplicationObj
    sprintf(buf, "App%ld", ind);


  } else {
  
    strcpy(buf, "-");
    p1 = DB_Char_Typ (typ);
    // TX_Error("APED_oid_dbo__: Objekttyp %d gefunden",typ);
    TX_Print("Err APED_oid_dbo__ obj %s %d %d",p1,ind,typ);
    return -1;
    
  }
  
  // printf("ex APED_oid_dbo__ |%s| %d\n",buf,strlen(buf)); // nur Testausg.

  return 1;

}
*/
// EOF
