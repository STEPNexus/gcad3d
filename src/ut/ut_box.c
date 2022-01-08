//**************************************************************************
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
  if (UT2D_Box_is_empty) UT2D_Box_set_mdlSiz   2 functions ...
  ..

-----------------------------------------------------
Modifications:
2004-06-11 neu aus ut_geo.c RF.

-----------------------------------------------------
*/
/*!
\file  ../ut/ut_box.c
\brief  2D-bounding-box-, 3D-bounding-box-functions
\code

Box = axis-parallel box from 2 points:
 p1 = low x,y,z,
 p2 = high x,y,z.

=====================================================
List_functions_start:

---------------- 2D ------------------------
UT2D_ckBoxinBox1          check if 2 boxes overlap
UT2D_ckBoxinBox2          check if Box2 is complete in Box1
UT2D_pt_ck_inBox          check if point is inside box
UT2D_pt_ck_inBoxTol       check if point p is inside box p1-p2

UT2D_box_ini0             init box
UT2D_box_extend           Box mit point vergroessern
UT2D_box_obj              make box of 2 bin.objs
UT2D_box_2pt__            make box of 2 points
UT2D_box_2pt_tol          make box of 2 points + tol
UT2D_box_ci2              box 2D-circ
UT2D_box_inpt             2D-bounding-box for indexed-points
UT2D_rect_inpt3           bbox for indexed point-array  <<<
UT2D_rect_pts             bounding rectangle <-- 2D-points
UT2D_rect_pta3            bounding rectangle <-- 3D-points

UT2D_box_4pt_bp           get 4 points of bbox on backplane

UT2D_box_addTol           add tolerance to box (enlarge box)
BBX2__ext_ptLL            extend lower left point of 2D-box
BBX2__ext_ptUR            extend upper right point of 2D-box
BBX2__nbbx                get box around boxes

UT2D_box_dump

---------------- 3D ------------------------
UT3D_ckBoxinBox1          check if 2 boxes overlap
UT3D_pt_ck_inBoxTol       check if point p is inside box p1-p2
UT3D_ck_ptInBox           check if point px is inside box p1-p2
UT3D_box_ck_intLnLim      check if limited line intersects bbox
UT3D_box_ck_intLnUnl      check if unlim. line intersects bbox
UT3D_box_ck_intpl         check intersect. Plane / Box (estimate!)
UT3D_box_ck_empty         test if box is empty

UT3D_box_ix_npt           get bounding-box for n points and find extreme points
UT3D_box_pts              bounding box <-- points
UT3D_box_2pt__            make box of 2 points
UT3D_box_2pt_tol          make box of 2 points + tol
UT3D_box_tria             make box from triangle

UT3D_box_ini0             init box
UT3D_box_extend           Box mit point vergroessern
see also UT3D_box_ox
UT3D_box_addTol           add tolerance to box (enlarge box)

UT3D_box_mdl__            get box for model
UT3D_box_ln               box Line 
UT3D_box_ci
UT3D_box_mdr              UU
UT3D_box_ox 
UT3D_box_obja
BBX__pMsh_dbi             BBox for surface from points from dbi
UT3D_box_Sphere
UT3D_box_Conus
UT3D_box_Torus
// UT3D_box_Prism
UT3D_box_CurvClot
UT3D_box_GText
BBX_def__                 get default-modelbox (size=modelSize)

UT3D_ptvc_intbox          intersect point/vector with box
UT3D_ch_lnbox             check line-Intersection with axis-parallel box (u3d.c)
UT3D_ln_intbox            relimit line inside box

UT3D_cv_boxxy             load rect.points from xmin/xmax ymin/ymax in z=0


Liste_Funktionen_Ende:
=====================================================
- see also:
UT3D_box_ox                Box mit obj vergroessern
GR_tDyn_box__               disp boundingBox

\endcode *//*----------------------------------------



=====================================================

*/

#ifdef _MSC_VER
#include "../xa/MS_Def0.h"
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>                         // for ...


#include "../ut/ut_geo.h"
#include "../ut/ut_cast.h"                // INT_PTR
#include "../ut/ut_ox_base.h"             // OGX_SET_INDEX
#include "../ut/ut_memTab.h"           // MemTab_..
#include "../ut/ut_itmsh.h"            // MSHIG_EDGLN_.. typedef_MemTab.. Fac3


#include "../gr/ut_DL.h"               // DL_IS_HIDDEN
#include "../gr/ut_gr.h"               // GR_gtx_ckBlockWidth
#include "../db/ut_DB.h"               //






//=============== extern glob. vars ======================
// ex ../ci/NC_Main.c
extern double  APT_ModSiz;

// ex ../xa/xa.c:
extern Plane     WC_sur_act;            // Constr.Plane
extern double     AP_txsiz;       // Notes-Defaultsize
extern char AP_modact_nam[128];   // name of the active submodel; def="" (main)




//=====================================================================
  int UT2D_box_4pt_bp (Point2 *p2a, Point *pmin, Point *pmax, int bp) {
//=====================================================================
// UT2D_box_4pt_bp            get 4 points of bbox on backplane

  int     irc = 0;


  //----------------------------------------------------------------
  if(bp == BCKPLN_XY) {

    p2a[0].x = pmin->x;
    p2a[0].y = pmin->y;

    p2a[1].x = pmax->x;
    p2a[1].y = pmin->y;

    p2a[2].x = pmax->x;
    p2a[2].y = pmax->y;

    p2a[3].x = pmin->x;
    p2a[3].y = pmax->y;


  //----------------------------------------------------------------
  } else if(bp == BCKPLN_XZ) {

    p2a[0].x = pmin->x;
    p2a[0].y = pmin->z;

    p2a[1].x = pmax->x;
    p2a[1].y = pmin->z;

    p2a[2].x = pmax->x;
    p2a[2].y = pmax->z;

    p2a[3].x = pmin->x;
    p2a[3].y = pmax->z;


  //----------------------------------------------------------------
  } else {
    TX_Error("UT2D_box_4pt_bp only bp XY and XZ");
    irc = -1;
  }


  return irc;

}


//================================================================
  int BBX_def__ (Point *pb1, Point *pb2) {
//================================================================
// BBX_def__                 get default-modelbox (size=modelSize)

  double d1;

  d1 = APT_ModSiz / 2.;

  pb1->x =  -d1;
  pb1->y =  -d1;
  pb1->z =  -d1;

  pb2->x =  d1;
  pb2->y =  d1;
  pb2->z =  d1;

  return 0;

}


//================================================================
  int BBX2__ext_ptLL (BBox2 *bbx, Point2 *pt1) {
//================================================================
// extend lower left point of 2D-box 
// see also UT2D_box_extend


  if(pt1->x < bbx->pb1.x) bbx->pb1.x = pt1->x;
  if(pt1->y < bbx->pb1.y) bbx->pb1.y = pt1->y;


  return 0;

}


//================================================================
  int BBX2__ext_ptUR (BBox2 *bbx, Point2 *pt1) {
//================================================================
// extend upper right point of 2D-box 
// see also UT2D_box_extend


  if(pt1->x > bbx->pb2.x) bbx->pb2.x = pt1->x;
  if(pt1->y > bbx->pb2.y) bbx->pb2.y = pt1->y;

  return 0;

}


//================================================================
  int BBX2__nbbx (BBox2 *bbxO, BBox2 *bbxa, int bbxNr) {
//================================================================
// get box around boxes

  int   i1;

  printf("BBX2__nbbx %d\n",bbxNr);

  // copy 1. box
  *bbxO = bbxa[0];

  for(i1=1; i1<bbxNr; ++i1) {
      // DEB_dump_obj__ (Typ_BBox2, &bbxa[i1], "bbx2[%d]",i1);
    BBX2__ext_ptLL (bbxO, &bbxa[i1].pb1);
    BBX2__ext_ptUR (bbxO, &bbxa[i1].pb2);
  }


    // GR_tDyn_box2__ (&bbxO->pb1, &bbxO->pb2, Typ_Att_hili);

  return 0;

}


//================================================================
  int UT2D_box_addTol (Point2 *pb1, Point2 *pb2, double tol) {
//================================================================
/// \code
/// UT2D_box_addTol            add tolerance to box
/// \endcode

  pb1->x -= tol;
  pb2->x += tol;

  pb1->y -= tol;
  pb2->y += tol;

  return 0;

}


//================================================================
    UT2D_box_inpt (Point2 *pmin, Point2 *pmax,
                   int ptNr, int *ipa, Point2 *pta) {
//================================================================
// UT2D_box_inpt3            2D-bounding-box for indexed-points
// see UT2D_rect_pts

  int     i1;
  Point2  *px;


  // printf("UT2D_box_inpt3 %d\n",ptNr);

  *pmin = pta[ipa[0]];  // UT2D_pt_pt3 (&pta[ipa[0]]);
  *pmax = *pmin;

  for (i1=1; i1<ptNr; ++i1) {
    px = &pta[ipa[i1]];
    if (px->x < pmin->x) pmin->x = px->x;
    if (px->y < pmin->y) pmin->y = px->y;
    if (px->x > pmax->x) pmax->x = px->x;
    if (px->y > pmax->y) pmax->y = px->y;
  }

    // printf(" ex-box_inpt %f %f - %f %f\n",pmin->x,pmin->y,pmax->x,pmax->y);

  return 0;

}


//====================================================================
  int UT2D_box_ini0 (Point2 *pt1, Point2 *pt2) {
//====================================================================
/// UT3D_box_ini0         init box


  pt1->x =  UT_VAL_MAX;
  pt2->x = -UT_VAL_MAX;

  pt1->y =  UT_VAL_MAX;
  pt2->y = -UT_VAL_MAX;

  return 0;

}


//================================================================
  int UT3D_box_ck_empty (Point *pb1, Point *pb2) {
//================================================================
/// \code
/// test if box is empty
/// returns 0 = no, box has values
///         1 = yes box is empty (pb1 and pb2 == UT3D_PT_NUL)
/// \endcode


  if((pb1->x + pb1->y + pb1->z) != 0.) return 0;
  if((pb2->x + pb2->y + pb2->z) != 0.) return 0;

  return 1;

}


//====================================================================
  int UT2D_box_extend (Point2 *pb1, Point2 *pb2, Point2 *pt1) {
//====================================================================
/// \code
/// Box mit point vergroessern.
/// init Box zB mit UT3D_box_ini0
/// \endcode


    // DEB_dump_obj__(Typ_PT2, pt1, "UT2D_box_extend");
    // DEB_dump_obj__(Typ_PT2, pb1, " _ext pb1i=");
    // DEB_dump_obj__(Typ_PT2, pb2, " _ext pb2i=");


  if(pt1->x < pb1->x) pb1->x = pt1->x;
  if(pt1->x > pb2->x) pb2->x = pt1->x;

  if(pt1->y < pb1->y) pb1->y = pt1->y;
  if(pt1->y > pb2->y) pb2->y = pt1->y;

    // DEB_dump_obj__(Typ_PT2, pb1, " _ext pb1o=");
    // DEB_dump_obj__(Typ_PT2, pb2, " _ext pb2o=");

  return 0;

}


//====================================================================
  int UT3D_box_ini0 (Point *pt1, Point *pt2) {
//====================================================================
/// UT3D_box_ini0         init box


  pt1->x =  UT_VAL_MAX;
  pt1->y =  UT_VAL_MAX;
  pt1->z =  UT_VAL_MAX;

  pt2->x = -UT_VAL_MAX;
  pt2->y = -UT_VAL_MAX;
  pt2->z = -UT_VAL_MAX;


  return 0;

}


//================================================================
  int UT2D_box_ci2 (Point2 *pb1, Point2 *pb2, Circ2 *ci1) {
//================================================================
// UT2D_box_ci2              box 2D-circ

  double   a1, a2, d1;
  Point2   ptx;


  // DEB_dump_obj__ (Typ_CI2, ci1, "UT2D_box_ci2");
  // GR_Disp_ci2 (ci1, Typ_Att_blue);


  // set d1 = fabs(rad)
  d1 = fabs(ci1->rad);

  // ck 360-deg
  if(!UT2D_ck_ci360(ci1)) {
    pb1->x = ci1->pc.x - d1;
    pb1->y = ci1->pc.y - d1;
    pb2->x = ci1->pc.x + d1;
    pb2->y = ci1->pc.y + d1;
    goto L_exit;
  }

  // init box with p1,p2
  UT2D_box_2pt__ (pb1, pb2, &ci1->p1, &ci1->p2);

  // get a1,a2 = angles at startPt and endPt; 0 < a1 < a2 < 720 deg
  a1 = UT2D_angr_ptpt (&ci1->pc, &ci1->p1);
  UT2D_ANGR_ADD_4PI (a1, a2, ci1->ango);
    // printf(" box_ci2-a1 = %f a2 = %f\n",a1,a2);

  if(a2 < a1) MEM_swap_2db (&a1, &a2);



  //----------------------------------------------------------------
  // test all 90 deg from 0 to 720 deg
  L_nxt_0:
  if(a1 > RAD_90) goto L_nxt_5;

  // test if circ goes tru 0 deg; yes = add dx
  if(!DLIM3(0., a1, a2)) {
      // printf(" box_ci2-0 add\n");
    ptx.x = ci1->pc.x + d1;
    ptx.y = ci1->pc.y;
    UT2D_box_extend (pb1, pb2, &ptx);
  }

  // test if circ goes tru 90 deg; yes = add dy
  if(!DLIM3(RAD_90, a1, a2)) {
      // printf(" box_ci2-90 add\n");
    ptx.x = ci1->pc.x;
    ptx.y = ci1->pc.y + d1;
    UT2D_box_extend (pb1, pb2, &ptx);
  }

  // test if circ goes tru 180 deg; yes = add -dx
  L_nxt_5:
  if(a2 < RAD_180) goto L_exit;
  if(!DLIM3(RAD_180, a1, a2)) {
      // printf(" box_ci2-180 add\n");
    ptx.x = ci1->pc.x - d1;
    ptx.y = ci1->pc.y;
    UT2D_box_extend (pb1, pb2, &ptx);
  }

  // test if circ goes tru 270 deg; yes = add -dy
  if(!DLIM3(RAD_270, a1, a2)) {
      // printf(" box_ci2-270 add\n");
    ptx.x = ci1->pc.x;
    ptx.y = ci1->pc.y - d1;
    UT2D_box_extend (pb1, pb2, &ptx);
  }

  // test also 2pi -> 4Pi
  L_nxt_9:
  if(a2 >= RAD_360) {
    a1 -= RAD_360;
    a2 -= RAD_360;
    goto L_nxt_0;
  }

  L_exit:
    // TESTBLOCK
    // GR_tDyn_box2__ (pb1, pb2, Typ_Att_dash__);
    // END TESTBLOCK

  return 0;

}


//===========================================================================
  int UT2D_box_2pt__ (Point2 *pb1, Point2 *pb2, Point2 *pt1, Point2 *pt2) {
//===========================================================================
/// UT2D_box_2pt__           make box of 2 points


  // DEB_dump_obj__ (Typ_PT, pt1, "UT3D_box_2pt\n");
  // DEB_dump_obj__ (Typ_PT, pt2, "");


  if(pt1->x < pt2->x) {
    pb1->x = pt1->x;
    pb2->x = pt2->x;
  } else {
    pb1->x = pt2->x;
    pb2->x = pt1->x;
  }

  if(pt1->y < pt2->y) {
    pb1->y = pt1->y;
    pb2->y = pt2->y;
  } else {
    pb1->y = pt2->y;
    pb2->y = pt1->y;
  }

  // GR_tDyn_box__(pb1, pb2, 2);

  return 0;

}


//==============================================================================
  int UT2D_box_obj (Point2 *pb1, Point2 *pb2, int typ, void *obj, double *tol) {
//==============================================================================
// UT2D_box_obj                  make box of 2 bin.objs


  if(typ == Typ_LN2) {
    UT2D_box_2pt_tol (pb1, pb2, &((Line2*)obj)->p1, &((Line2*)obj)->p2, *tol);

  } else if(typ == Typ_CI2) {
    UT2D_box_ci2 (pb1, pb2, (Circ2*)obj);
    UT2D_box_addTol (pb1, pb2, *tol);

  } else {
    TX_Error("UT2D_box_obj E1 %d",typ);
    return -1;
  }

    // TESTBLOCK
    // GR_tDyn_box2__ (pb1, pb2, Typ_Att_dash__);
    // END TESTBLOCK


  return 0;

}


//===========================================================================
  int UT2D_box_2pt_tol (Point2 *pb1, Point2 *pb2, Point2 *pt1, Point2 *pt2,
                       double tol) {
//===========================================================================
/// UT2D_box_2pt_tol         make box of 2 points + tol


  // DEB_dump_obj__ (Typ_PT, pt1, "UT3D_box_2pt\n");
  // DEB_dump_obj__ (Typ_PT, pt2, "");


  if(pt1->x < pt2->x) {
    pb1->x = pt1->x - tol;
    pb2->x = pt2->x + tol;
  } else {
    pb1->x = pt2->x - tol;
    pb2->x = pt1->x + tol;
  }

  if(pt1->y < pt2->y) {
    pb1->y = pt1->y - tol;
    pb2->y = pt2->y + tol;
  } else {
    pb1->y = pt2->y - tol;
    pb2->y = pt1->y + tol;
  }

  // GR_tDyn_box__(pb1, pb2, 2);

  return 0;

}


//================================================================
  int UT2D_box_dump (Point2 *p1, Point2 *p2, char *txt, ...) {
//================================================================
// see DEB_dump__ Typ_PT2

  char       cbuf[256];
  va_list    va;


  va_start(va,txt);
  vsprintf(cbuf,txt,va);
  va_end(va);

  printf("UT2D_box_dump %s\n",cbuf);

  printf("(Point2) %9.3f,%9.3f\n",p1->x,p1->y);
  printf("(Point2) %9.3f,%9.3f\n",p2->x,p2->y);




  return 0;

}


//=======================================================================
  int UT2D_ckBoxinBox1 (Point2 *p1, Point2 *p2, Point2 *p3, Point2 *p4) {
//=======================================================================
/// \code
/// check if 2 boxes overlap
/// p1-p2 sind Box1, p3-p4 Box2.
/// ACHTUNG: p1 muss links unter p2 liegen; p3 links unter p4.
/// RC  0:   NO, boxes do not overlap
/// RC  1:   yes boxes overlap.
/// \endcode


  // printf(" UT2D_ckBoxinBox1\n");
  // printf("    b1=%f,%f  - %f,%f\n",p1->x,p1->y,p2->x,p2->y);
  // printf("    b2=%f,%f  - %f,%f\n",p3->x,p3->y,p4->x,p4->y);



  if(p3->x > p2->x) return 0;  // b2 ganz rechts
  if(p4->x < p1->x) return 0;  // b2 ganz links

  if(p3->y > p2->y) return 0;  // b2 ueber b1
  if(p4->y < p1->y) return 0;  // b2 unter b1

  return 1;  // irgendwo overlap

}


//=======================================================================
  int UT2D_ckBoxinBox2 (Point2 *p1, Point2 *p2,
                        Point2 *p3, Point2 *p4, double tol) {
//=======================================================================
/// \code
/// check if Box2 is complete in Box1
/// p1-p2 sind Box1, p3-p4 Box2.
/// ACHTUNG: p1 muss links unter p2 liegen; p3 links unter p4.
/// RC -1:   NO, boxes overlap
/// RC  1:   yes, Box2 is complete inside Box1
/// \endcode


  // printf(" UT2D_ckBoxinBox2\n");
  // printf("    b1=%f,%f  - %f,%f\n",p1->x,p1->y,p2->x,p2->y);
  // printf("    b2=%f,%f  - %f,%f\n",p3->x,p3->y,p4->x,p4->y);


  if(p3->x < p1->x + tol) return -1;   // overlap links
  if(p4->x > p2->x - tol) return -1;   // overlap rechts

  if(p4->y > p2->y - tol) return -1;   // overlap oben
  if(p3->y < p1->y + tol) return -1;   // overlap unten

  return 1;   // komplett innerhalb

}



//=======================================================================
  int UT2D_pt_ck_inBox (Point2 *p1, Point2 *p2, Point2 *p) {
//=======================================================================
/// \code
/// UT2D_pt_ck_inBox             check if point p is inside box p1-p2
/// RC = 0:   YES, p is inside box p1 / p2
/// RC = 1:   NO,  p is outside box p1 / p2
/// \endcode


  if(UTP_db_ck_in2db (p->x, p1->x, p2->x) != 0) return 1;

  if(UTP_db_ck_in2db (p->y, p1->y, p2->y) != 0) return 1;

  return 0;

}


//======================================================================
  int UT2D_rect_pts (Point2 *pmin, Point2 *pmax, int nump, Point2 *ptab) {
//======================================================================
// UT2D_rect_pts        Author: Thomas Backmeister       3.6.2003
// 
/// \code
/// UT2D_rect_pts        bounding rectangle <-- 2D-points
/// 
/// Computation of a minimal axis-parallel rectangle which contains all
/// given 2D-points.
/// The rectangle is given by the endpoints of its diagonal.
/// If all points ly on a line parallel x-, y- or z- axis, the rectangle
/// will be a line segment.
/// 
/// IN:
///   int nump     ... number of 2D-points
///   Point2 *ptab ... 2D-points
/// OUT:
///   Point2 *pmin ... endpoint 1 of rectangle diagonal (lower left)
///   Point2 *pmax ... endpoint 2 of rectangle diagonal (upper right)
/// Returncodes:
///   0 = OK
///  -1 = input error
/// \endcode


  int i1;


  // printf("UT2D_rect_pts ptNr=%d\n",nump);


  if (nump < 1) {
    printf("UT2D_rect_pts: input error\n");
    return -1;
  }

  *pmin = ptab[0];
  *pmax = ptab[0];

  for (i1=1; i1<nump; ++i1) {
    // printf(" pt[%d]=%f,%f\n",i1,ptab[i1].x,ptab[i1].y);
    if (ptab[i1].x < pmin->x) pmin->x = ptab[i1].x;
    if (ptab[i1].y < pmin->y) pmin->y = ptab[i1].y;
    if (ptab[i1].x > pmax->x) pmax->x = ptab[i1].x;
    if (ptab[i1].y > pmax->y) pmax->y = ptab[i1].y;
  }


  // printf("ex UT2D_rect_pts: %9.3f,%9.3f - %9.3f,%9.3f\n",
         // pmin->x,pmin->y,pmax->x,pmax->y);

  return 0;
}


//=================================================================
  int UT2D_rect_pta3 (Point2 *p1, Point2 *p2, Point *pa, int pNr) {
//=================================================================
/// UT2D_rect_pta3           bounding rectangle <-- 3D-points

  int    i1;


  *p1 = UT2D_pt_pt3 (&pa[0]);
  *p2 = UT2D_pt_pt3 (&pa[1]);

  for(i1=2; i1 < pNr; ++i1) {
    UT2D_box_extend (p1, p2, (Point2*)&pa[i1]);
  }

  // printf("ex UT2D_rect_pta3 %f %f / %f %f\n",p1->x,p1->y,p2->x,p2->y);

  return 0;

}


//=======================================================================
  int UT3D_ckBoxinBox1 (Point *p1, Point *p2, Point *p3, Point *p4) {
//=======================================================================
/// \code
/// check if 2 boxes overlap
/// p1-p2 sind Box1, p3-p4 Box2.
/// ACHTUNG: p1 muss links unter p2 liegen; p3 links unter p4.
/// Toleranzen: schon zur Box dazuaddieren.
/// RC -1:   NO, boxes do not overlap
/// RC  1:   yes boxes overlap.
/// \endcode


  // printf(" UT2D_ckBoxinBox1\n");
  // printf("    b1=%f,%f  - %f,%f\n",p1->x,p1->y,p2->x,p2->y);
  // printf("    b2=%f,%f  - %f,%f\n",p3->x,p3->y,p4->x,p4->y);



  if(p3->x > p2->x) return -1;  // b2 ganz rechts
  if(p4->x < p1->x) return -1;  // b2 ganz links

  if(p3->y > p2->y) return -1;  // b2 ueber b1
  if(p4->y < p1->y) return -1;  // b2 unter b1

  if(p3->z > p2->z) return -1;  // b2 ueber b1
  if(p4->z < p1->z) return -1;  // b2 unter b1

  return 1;  // irgendwo overlap

}


//=======================================================================
  int UT2D_pt_ck_inBoxTol (Point2 *p1, Point2 *p2, Point2 *p, double tol) {
//=======================================================================
/// \code
/// UT2D_pt_ck_inBoxTol          check if point p is inside box p1-p2
/// RC = 0:   YES, p is inside box p1 / p2
/// RC = 1:   NO,  p is outside box p1 / p2
/// \endcode


  if(UTP_db_ck_in2dbTol (p->x, p1->x, p2->x, tol) != 0) return 1;

  if(UTP_db_ck_in2dbTol (p->y, p1->y, p2->y, tol) != 0) return 1;

  return 0;

}


//======================================================================
  int UT3D_box_ix_npt (Point *pmin, Point *pmax, int *ia,
                       int nump, Point *ptab) {
//======================================================================
/// UT3D_box_ix_npt     get bounding-box for n points and find extreme points
/// IN:
///   int nump    ... number of points
///   Point *ptab ... points
/// OUT:
///   Point *pmin ... endpoint 1 of box diagonal
///   Point *pmax ... endpoint 2 of box diagonal
///   int   ia[6]     indices of xmin,ymin,zmin,xmax,ymax,zmax-points
///   Returncodes:
///     0 = OK
///    -1 = input error
///
/// UT3D_box_ini0 not necessary
/// \endcode
  
  int   i1;
  
  
  if (nump < 1) {
    printf("UT3D_box_ix_npt: input error\n");
    return -1;
  }
    
  *pmin = ptab[0];
  *pmax = ptab[0];

  for (i1=0; i1<6; ++i1) ia[i1] = 0;

  for (i1=1; i1<nump; ++i1) {
    if (ptab[i1].x < pmin->x) {pmin->x = ptab[i1].x; ia[0] = i1;}
    if (ptab[i1].y < pmin->y) {pmin->y = ptab[i1].y; ia[1] = i1;}
    if (ptab[i1].z < pmin->z) {pmin->z = ptab[i1].z; ia[2] = i1;}
    if (ptab[i1].x > pmax->x) {pmax->x = ptab[i1].x; ia[3] = i1;}
    if (ptab[i1].y > pmax->y) {pmax->y = ptab[i1].y; ia[4] = i1;}
    if (ptab[i1].z > pmax->z) {pmax->z = ptab[i1].z; ia[5] = i1;}
  }

    // printf("ex UT3D_box_ix_npt min-x:%d y:%d z:%d\n",ia[0],ia[1],ia[2]);
    // printf("   UT3D_box_ix_npt max-x:%d y:%d z:%d\n",ia[3],ia[4],ia[5]);

  return 0;

}


//======================================================================
  int UT3D_box_pts (Point *pmin, Point *pmax, int nump, Point *ptab) {
//======================================================================
// UT3D_box_pts         Author: Thomas Backmeister       23.5.2003
// 
/// \code
/// UT3D_box_pts         bounding box <-- points
/// 
/// Computation of a minimal axis-parallel box which contains all given points.
/// The box is given by the endpoints of its diagonal.
/// If all points ly in a plane parallel x,y-plane, the box will be a rectangle.
/// 
/// IN:
///   int nump    ... number of points
///   Point *ptab ... points
/// OUT:
///   Point *pmin ... endpoint 1 of box diagonal
///   Point *pmax ... endpoint 2 of box diagonal
/// Returncodes:
///   0 = OK
///  -1 = input error
/// \endcode


  int i1;

  //printf("UT3D_box_pts: bounding box <-- points\n");

  if (nump < 1) {
    printf("UT3D_box_pts: input error\n");
    return -1;
  }

  *pmin = ptab[0];
  *pmax = ptab[0];
  for (i1=1; i1<nump; ++i1) {
    if (ptab[i1].x < pmin->x) pmin->x = ptab[i1].x;
    if (ptab[i1].y < pmin->y) pmin->y = ptab[i1].y;
    if (ptab[i1].z < pmin->z) pmin->z = ptab[i1].z;
    if (ptab[i1].x > pmax->x) pmax->x = ptab[i1].x;
    if (ptab[i1].y > pmax->y) pmax->y = ptab[i1].y;
    if (ptab[i1].z > pmax->z) pmax->z = ptab[i1].z;
  }

  return 0;
}


//===========================================================================
  int UT3D_box_2pt__ (Point *pb1, Point *pb2, Point *pt1, Point *pt2) {
//===========================================================================
/// UT3D_box_2pt__         make box of 2 points


  // DEB_dump_obj__ (Typ_PT, pt1, "UT3D_box_2pt\n");
  // DEB_dump_obj__ (Typ_PT, pt2, "  ");


  // if((fabs(pt1->x) > UT_VAL_MAX) ||
     // (fabs(pt1->y) > UT_VAL_MAX) ||
     // (fabs(pt1->z) > UT_VAL_MAX)) {
  // if(isnan(pt1->x)) {
    // printf("***** UT3D_box_2pt__ I001\n");
    // return 0;
  // }


  if(pt1->x < pt2->x) {
    pb1->x = pt1->x;
    pb2->x = pt2->x;
  } else {
    pb1->x = pt2->x;
    pb2->x = pt1->x;
  }

  if(pt1->y < pt2->y) {
    pb1->y = pt1->y;
    pb2->y = pt2->y;
  } else {
    pb1->y = pt2->y;
    pb2->y = pt1->y;
  }

  if(pt1->z < pt2->z) {
    pb1->z = pt1->z;
    pb2->z = pt2->z;
  } else {
    pb1->z = pt2->z;
    pb2->z = pt1->z;
  }

  // GR_tDyn_box__(pb1, pb2, 2);

  return 0;

}


//======================================================================
  int UT3D_box_tria (Point *pb1, Point *pb2, Triang *tr, double tol) {
//======================================================================
/// UT3D_box_tria             make box from triangle


  UT3D_box_2pt__ (pb1, pb2, tr->pa[0], tr->pa[1]);  // make box of p1,p2
  UT3D_box_extend (pb1, pb2, tr->pa[2]);          // Box mit p3 vergroessern.
  if(tol > 0.)
  UT3D_box_addTol (pb1, pb2, tol);

    // GR_tDyn_box__(pb1, pb2, 8);

  return 0;

}


//===========================================================================
  int UT3D_box_2pt_tol (Point *pb1, Point *pb2, Point *pt1, Point *pt2,
                       double tol) {
//===========================================================================
/// UT3D_box_2pt_tol         make box of 2 points + tol


  // DEB_dump_obj__ (Typ_PT, pt1, "UT3D_box_2pt\n");
  // DEB_dump_obj__ (Typ_PT, pt2, "");


  if(pt1->x < pt2->x) {
    pb1->x = pt1->x - tol;
    pb2->x = pt2->x + tol;
  } else {
    pb1->x = pt2->x - tol;
    pb2->x = pt1->x + tol;
  }

  if(pt1->y < pt2->y) {
    pb1->y = pt1->y - tol;
    pb2->y = pt2->y + tol;
  } else {
    pb1->y = pt2->y - tol;
    pb2->y = pt1->y + tol;
  }

  if(pt1->z < pt2->z) {
    pb1->z = pt1->z - tol;
    pb2->z = pt2->z + tol;
  } else {
    pb1->z = pt2->z - tol;
    pb2->z = pt1->z + tol;
  }

  // GR_tDyn_box__(pb1, pb2, 2);

  return 0;

}


//====================================================================
  int UT3D_box_extend (Point* pb1, Point* pb2, Point* pt1) {
//====================================================================
/// \code
/// Box mit point vergroessern.
/// init Box zB mit UT3D_box_ini0
/// \endcode


    // DEB_dump_obj__(Typ_PT, pt1, "UT3D_box_extend");
    // DEB_dump_obj__(Typ_PT, pb1, " _ext pb1i=");
    // DEB_dump_obj__(Typ_PT, pb2, " _ext pb2i=");


  if(pt1->x < pb1->x) pb1->x = pt1->x;
  if(pt1->x > pb2->x) pb2->x = pt1->x;

  if(pt1->y < pb1->y) pb1->y = pt1->y;
  if(pt1->y > pb2->y) pb2->y = pt1->y;

  if(pt1->z < pb1->z) pb1->z = pt1->z;
  if(pt1->z > pb2->z) pb2->z = pt1->z;


    // DEB_dump_obj__(Typ_PT, pb1, " _ext pb1o=");
    // DEB_dump_obj__(Typ_PT, pb2, " _ext pb2o=");

  return 0;

}


//================================================================
  int UT3D_box_addTol (Point *pb1, Point *pb2, double tol) {
//================================================================
/// \code
/// UT3D_box_addTol           add tolerance to box
/// \endcode
// ganz am Ende aufaddieren ist schneller

  pb1->x -= tol;
  pb2->x += tol;

  pb1->y -= tol;
  pb2->y += tol;

  pb1->z -= tol;
  pb2->z += tol;


  return 0;

}


//================================================================
  int UT3D_ck_ptInBox (Point *p1, Point *p2, Point *px) {
//================================================================
/// \code
/// UT3D_ck_ptInBox          check if point px is inside box p1-p2
/// box should have tolerance added; use UT3D_box_addTol
/// RC = 0:   YES, p is inside box p1 / p2
/// RC = 1:   NO,  p is outside box p1 / p2
/// \endcode

  if(px->x < p1->x) return 1;
  if(px->x > p2->x) return 1;

  if(px->y < p1->y) return 1;
  if(px->y > p2->y) return 1;

  if(px->z < p1->z) return 1;
  if(px->z > p2->z) return 1;

  return 0;

}


//=======================================================================
  int UT3D_pt_ck_inBoxTol (Point *p1, Point *p2, Point *p, double tol) {
//=======================================================================
/// \code
/// UT3D_pt_ck_inBoxTol          check if point p is inside box p1-p2
/// RC = 0:   YES, p is inside box p1 / p2
/// RC = 1:   NO,  p is outside box p1 / p2
/// create boxpoints p1,p2 with UT3D_box_2pt()
/// \endcode


  // DEB_dump_obj__ (Typ_PT, p, "UT3D_pt_ck_inBoxTol %f pt\n",tol);
  // DEB_dump_obj__ (Typ_PT, p1, "  ",tol);
  // DEB_dump_obj__ (Typ_PT, p2, "  ",tol);


  if(UTP_db_ck_in2dbTol (p->x, p1->x, p2->x, tol) != 0) return 1;

  if(UTP_db_ck_in2dbTol (p->y, p1->y, p2->y, tol) != 0) return 1;

  if(UTP_db_ck_in2dbTol (p->z, p1->z, p2->z, tol) != 0) return 1;

  return 0;

}


//=============================================================================
  int UT3D_box_ck_intLnUnl (Point *pls, Vector *vl, Point *pmin, Point *pmax) {
//=============================================================================
// UT3D_box_ck_intLnUnl            check if unlim. line intersects bbox
//
// IN:
//   pmin,pmax   box
//   pls,vl      unlim line
// OUT:
//   retCode     0 no intersection
//               1 intersection
//

  int       irc, i1, ii1, ii2;
  Point2    p2l, p2a[4];
  Vector2   v2;


  // DEB_dump_obj__ (Typ_PT, pls, "UT3D_box_ck_intLnUnl pls");
  // DEB_dump_obj__ (Typ_VC, vl, "  vl");
  // DEB_dump_obj__ (Typ_PT, pmin, "  pmin");
  // DEB_dump_obj__ (Typ_PT, pmax, "  pmax");
  // GR_tDyn_box__ (pmin, pmax, Typ_Att_Symb);
  // GR_tDyn_vc__ (vl, pls, ATT_COL_T_BLACK, 0);


  //----------------------------------------------------------------
  // test backplane XY = BCKPLN_XY
  // get p2-v2 line on BCKPLN_XY
  UT2D_pt_tra_pt3_bp (&p2l, pls, BCKPLN_XY);
  UT2D_vc_tra_vc3_bp (&v2, vl, BCKPLN_XY);

  // get pa = points of bbox in BCKPLN_XY
  UT2D_box_4pt_bp (p2a, pmin, pmax, BCKPLN_XY);

  // check if pa-points on same side of line p2-v2
  ii1 = UT2D_sid_ptvc__ (&p2a[0], &p2l, &v2);
    // printf(" intLnUnl-XY ii1=%d\n",ii1);
  if(!ii1) goto L_ck_Z;
  for(i1=1; i1<4; ++i1) {
    ii2 = UT2D_sid_ptvc__ (&p2a[i1], &p2l, &v2);
      // printf(" intLnUnl-XY i1=%d ii2=%d\n",i1,ii2);
    if(ii2 != ii1) goto L_ck_Z;
  }

  goto L_exit_no;


  //----------------------------------------------------------------
  // test backplane XZ = BCKPLN_XZ
  L_ck_Z:
  // get p2-v2 line on BCKPLN_XZ
  UT2D_pt_tra_pt3_bp (&p2l, pls, BCKPLN_XZ);
  UT2D_vc_tra_vc3_bp (&v2, vl, BCKPLN_XZ);

  // get pa = points of bbox in BCKPLN_XZ
  UT2D_box_4pt_bp (p2a, pmin, pmax, BCKPLN_XZ);

  // check if pa-points on same side of line p2-v2
  ii1 = UT2D_sid_ptvc__ (&p2a[0], &p2l, &v2);
    // printf(" intLnUnl-XZ ii1=%d\n",ii1);
  if(!ii1) goto L_exit_yes;
  for(i1=1; i1<4; ++i1) {
    ii2 = UT2D_sid_ptvc__ (&p2a[i1], &p2l, &v2);
      // printf(" intLnUnl-XZ i1=%d ii2=%d\n",i1,ii2);
    if(ii2 != ii1) goto L_exit_yes;
  }

  goto L_exit_no;


  //----------------------------------------------------------------
  L_exit_yes:
    irc = 1;          // line intersects box
    goto L_exit__;

  //----------------------------------------------------------------
  L_exit_no:
    irc = 0;           // no intersection

  //----------------------------------------------------------------
  L_exit__:
    // printf("ex-UT3D_box_ck_intLnUnl %d\n",irc);
  return irc;

}


//======================================================================
  int UT3D_box_ck_intLnLim (Line *ln, Point *pmin, Point *pmax, double tol) {
//======================================================================
// UT3D_box_ck_intLnLim        Author: Thomas Backmeister       27.5.2003
// 
/// \code
/// UT3D_box_ck_intLnLim        check line-X with axis-parallel box
/// 
/// Check if a line intersects an axis-parallel box.
/// 
/// IN:
///   Line *ln    ... line
///   Point *pmin ... endpoint 1 of box diagonal
///   Point *pmax ... endpoint 2 of box diagonal
///   double tol  ... tolerance for line intersecting box (>0)
/// OUT:
/// Returncodes:
///   0 = the line does not intersect the box
///   1 = the line intersects the box
/// \endcode



  int rc, i1;
  double a, b, c, d, h, a1, b1, vh;
  Vector vl, vp1, vp2;


  // printf("UT3D_box_ck_intLnLim: ============================== tol=%f\n",tol);
  // DEB_dump_obj__ (Typ_LN, ln, "ln:");
  // DEB_dump_obj__ (Typ_PT, pmin, "pmin:");
  // DEB_dump_obj__ (Typ_PT, pmax, "pmax:");

  // init
  rc = 1;

  // line vector
  UT3D_vc_ln (&vl, ln);

  // 2 vectors
  UT3D_vc_2pt (&vp1, &(ln->p1), pmin);
  UT3D_vc_2pt (&vp2, &(ln->p1), pmax);

  // check X of line and box
  // - - - - - - - - - - - -
  a = UT_VAL_MIN;
  b = UT_VAL_MAX;
  a1 = vp1.dx;
  b1 = vp2.dx;
  vh = vl.dx;
  for (i1=0; i1<3; ++i1) {
    if (i1 == 1) {
      a1 = vp1.dy;
      b1 = vp2.dy;
      vh = vl.dy;
    }
    else if (i1 == 2) {
      a1 = vp1.dz;
      b1 = vp2.dz;
      vh = vl.dz;
    }
    if (UTP_compdb0 (vh, UT_TOL_min2)) {
      // 0 not in interval [a1,b1] -> no line-box X
      if (b1 < -tol || a1 > tol) {
        rc = 0;
        break;
      }
    }
    else {
      // intersection of [a,b] with [c,d] empty -> no line-box X
      c = a1 / vh;
      d = b1 / vh;
      if (d < c) {
        h = c;
        c = d;
        d = h;
      }
      if (c > a) a = c;
      if (d < b) b = d;
      if (a > (b + tol)) {
        rc = 0;
        break;
      }
    }
  }

  // printf("ex UT3D_box_ck_intLnLim rc=%d (0=no intersect, 1=yes)\n",rc);


  //================================================================
  // TEST ONLY: test if p1=-1008.122, -142.314,    0.000 is inside box
  // { Point pt1={-1008.122, -142.314,    0.000};
    // if(!UT3D_pt_ck_inBoxTol(pmin,pmax,&pt1,tol)) {
      // printf("-------------- rc=%d\n",rc);
      // DEB_dump_obj__ (Typ_PT, pmin, " pmin");
      // DEB_dump_obj__ (Typ_PT, pmax, " pmax");
      // GR_tDyn_box__(pmin, pmax, 0);
    // }
  // }
  //================================================================


  return rc;
}


//===================================================================
  int UT3D_box_ck_intpl (Plane *pln, Point *p1, Point *p2, double tol) {
//===================================================================
/// \code
/// UT3D_box_ck_intpl               check intersect. Plane / Box (estimate!)
/// ACHTUNG: dzt nur rasche Abschaetzung !!!
/// Returncodes:
///   0 = plane does not intersect the box
///   1 = plane intersects the box
/// \endcode

// erweitern: test mit dem Vektor Boxcenter -> Plane, welche Seitenflaeche der
// Box geschnitten wird.
// oder: es muss auf mindestens einer der 3 Hauptachsen (/Y/Z) der vektorTeil
// der Vektor Boxcenter->Plane groesser sein als vom Vektor Boxcenter->BoxCorner.
// 

  int    irc;
  double d1, d2;
  Point  pbc, ppc;
  // Vector vc1, vc2;


  // den Boxmittelpunkt auf die Plane projizieren
  UT3D_pt_mid2pt (&pbc, p1, p2);
  // GR_tDyn_symB__ (&pbc, SYM_TRI_S, 0);

  // UT3D_pt_projptpl (&ppc, pln, &pbc);
  UT3D_pt_intptvcpl_ (&ppc, pln, &pbc, &pln->vz);
  // GR_tDyn_symB__ (&ppc, SYM_TRI_S, 0);

  // UT3D_vc_2pt (&vc1, &pbc, p1);
  // UT3D_vc_2pt (&vc2, &pbc, &ppc);
  // DEB_dump_obj__ (Typ_VC, &vc1, "vc1");
  // DEB_dump_obj__ (Typ_VC, &vc2, "vc2");

  // BoxSiz = Abst**2 von BoxCenter / BoxEckpunkt
  d1 = UT3D_lenq_PtPt (&pbc, p1) + (tol * tol);
  // Abstand = Abst**2 von BoxCenter / Plane
  d2 = UT3D_lenq_PtPt (&pbc, &ppc);

  if(d1 < d2) irc = 0; // BoxSiz < Abstand = no intersect
  else        irc = 1; // yes

  // printf("ex UT3D_box_ck_intpl %d %f %f\n",irc,d1,d2);

  return irc;

}


//=============================================================================
  int UT3D_cv_boxxy (Point* pb1,double x1,double x2,double y1,double y2) {
//=============================================================================
/// UT3D_cv_boxxy          load rect.points from xmin/xmax ymin/ymax in z=0

// Quadrat aus xmin/xmax ymin/ymax in z=0 setzen.
// 5 punkte !


  // printf("UT3D_cv_boxxy %f %f %f %f\n",x1,x2,y1,y2);


  pb1[0].x = x1;   pb1[0].y = y1;  pb1[0].z = 0.;
  pb1[1].x = x2;   pb1[1].y = y1;  pb1[1].z = 0.;
  pb1[2].x = x2;   pb1[2].y = y2;  pb1[2].z = 0.;
  pb1[3].x = x1;   pb1[3].y = y2;  pb1[3].z = 0.;

  pb1[4] = pb1[0];

  return 0;

}


//================================================================
  int UT3D_ln_intbox (Line *ln1, Point *bp1, Point *bp2) {
//================================================================
/// \code
/// UT3D_ln_intbox            relimit line inside box
/// Line an Raumbox begrenzen
/// \endcode


  int    i1, i2;
  Vector vl;

  // DEB_dump_obj__ (Typ_LN, ln1, "UT3D_ln_intbox\n");
  // DEB_dump_obj__ (Typ_PT, bp1, "  bp1");
  // DEB_dump_obj__ (Typ_PT, bp2, "  bp2");


  i1 = UT3D_pt_ck_inBoxTol (bp1, bp2, &ln1->p1, UT_TOL_pt);
  // printf(" i1=%d\n",i1);

  if(i1 == 1) {  // p1 is outsize; replace p1
    UT3D_vc_2pt (&vl, &ln1->p1, &ln1->p2);

    // p1 mit dem Schnittpunkt ersetzen
    UT3D_ptvc_intbox (&ln1->p1, &vl, bp1, bp2);
  }


  i2 = UT3D_pt_ck_inBoxTol (bp1, bp2, &ln1->p2, UT_TOL_pt);
  // printf(" i2=%d\n",i2);

  if(i2 == 1) {  // p1 is outsize; replace p1
    UT3D_vc_2pt (&vl, &ln1->p2, &ln1->p1);

    // p2 mit dem Schnittpunkt ersetzen
    UT3D_ptvc_intbox (&ln1->p2, &vl, bp1, bp2);
  }


  // DEB_dump_obj__ (Typ_LN, ln1, "ex UT3D_ln_intbox\n");

  return 0;

}


//======================================================================
  int UT3D_ptvc_intbox (Point *pl, Vector *vl, Point *bp1, Point *bp2) {
//======================================================================
/// \code
/// UT3D_ptvc_intbox               intersect point/vector with box
/// 
/// Input:
///   pl ist ausserhalb box. vl zeigt in die box hinein.
/// Output:
///   pl ist der Schnittpunkt mit der am naechsten liegenden Boxplane.
/// \endcode

  int   ix, iy, iz;
  Point px, py, pz;


  // printf("UT3D_ptvc_intbox\n");
  // DEB_dump_obj__ (Typ_PT, pl, "  pl");
  // DEB_dump_obj__ (Typ_VC, vl, "  vl");


  // mit einer X-Plane, einer Y-Plane und einer Z-Plane schneiden


  // Schnitt mit einer X-Plane
  // welche X-Plane liegt naeher an pl->x
  if(pl->x < bp1->x) {       // darunter
    ix = 1;
    UT3D_pt_intptvcplx (&px, pl, vl, bp1->x);

  } else if(pl->x > bp2->x) {       // darueber
    ix = 1;
    UT3D_pt_intptvcplx (&px, pl, vl, bp2->x);

  } else {                          // dazwischen;
    ix = 0;
  }



  // Schnitt mit einer Y-Plane
  // welche Y-Plane liegt naeher an pl->x
  if(pl->y < bp1->y) {       // darunter
    iy = 1;
    UT3D_pt_intptvcply (&py, pl, vl, bp1->y);

  } else if(pl->y > bp2->y) {       // darueber
    iy = 1;
    UT3D_pt_intptvcply (&py, pl, vl, bp2->y);

  } else {                          // dazwischen;
    iy = 0;
  }


  // Schnitt mit einer Z-Plane
  // welche Z-Plane liegt naeher an pl->z
  if(pl->z < bp1->z) {       // darunter
    iz = 1;
    UT3D_pt_intptvcplz (&pz, pl, vl, bp1->z);

  } else if(pl->z > bp2->z) {       // darueber
    iz = 1;
    UT3D_pt_intptvcplz (&pz, pl, vl, bp2->z);

  } else {                          // dazwischen; kein Z-inters.Pt.
    iz = 0;
  }


  // if(px->x > AP_spc_p2->x) ..


  // nur einer der 3 Schnittpunkte befindet sich innerhalb der Box;
  // diesen wahlen ....
  if(ix > 0) {
    if(UT3D_pt_ck_inBoxTol (bp1, bp2, &px, UT_TOL_pt) == 0) {  // inside
      *pl = px;
      goto L_fertig;
    }
  }

  if(iy > 0) {
    if(UT3D_pt_ck_inBoxTol (bp1, bp2, &py, UT_TOL_pt) == 0) {  // inside
      *pl = py;
      goto L_fertig;
    }
  }

  if(iz > 0) {
    if(UT3D_pt_ck_inBoxTol (bp1, bp2, &pz, UT_TOL_pt) == 0) {  // inside
      *pl = pz;
      goto L_fertig;
    }
  }


  L_fertig:
  // DEB_dump_obj__ (Typ_PT, pl, "ex UT3D_ptvc_intbox");

  return 0;

}


//================================================================
  int UT3D_box_Torus (Point *pb1, Point *pb2, Torus *to1) {
//================================================================
// see GL_disp_tor

  double  r1;
  Circ    ci1;

  // DEB_dump_obj__ (Typ_TOR, to1, "UT3D_box_Torus");

  r1 = to1->r1 + to1->r2;

  // make circ from origin, z-axis, radius
  UT3D_ci_ptvcr (&ci1, &(to1->pl.po), &(to1->pl.vz), r1);
  UT3D_box_ci (pb1, pb2, &ci1);

  // TODO: move origin up and down


  return 0;

}


/*
//================================================================
  int UT3D_box_Prism (Point *pb1, Point *pb2, ObjGX *pr1) {
//================================================================

  DEB_dump_obj__ (Typ_PRI, pr1, "UT3D_box_Prism");


  // work both contours


// TODO: if pr1[1] == vector: translate points !

  return 0;

}
*/


//================================================================
  int UT3D_box_Conus (Point *pb1, Point *pb2, Conus *co1) {
//================================================================
// 2 points; cen + r-vx + r-vy + r-vz

  Circ      ci1;
  Point     po, p1, p2;


  // DEB_dump_obj__ (Typ_CON, co1, " UT3D_box_Conus ");


  // get bottom-circ
  po = co1->pl.po;
  UT3D_ci_ptvcr (&ci1, &po,  &co1->pl.vz,  co1->r1);
  UT3D_box_ci (&p1, &p2, &ci1);
  UT3D_box_extend (pb1, pb2, &p1);
  UT3D_box_extend (pb1, pb2, &p2);


  // get top-circ
  UT3D_pt_traptmultvc (&po, &po, &co1->pl.vz, co1->h);
  UT3D_ci_ptvcr (&ci1, &po,  &co1->pl.vz,  co1->r1);
  UT3D_box_ci (&p1, &p2, &ci1);
  UT3D_box_extend (pb1, pb2, &p1);
  UT3D_box_extend (pb1, pb2, &p2);


  return 0;

}


//================================================================
  int BBX__pMsh_dbi (Point *pb1, Point *pb2, long dbi) {
//================================================================
// BBX__pMsh_dbi             BBox for surface from points from dbi
// was GR_cv_pMesh_box

  int           i1, typ, ptNr;
  MemTab(Point) pTab = _MEMTAB_NUL;


  // printf("BBX__pMsh_dbi %ld\n",dbi);

  MemTab_ini__ (&pTab, sizeof(Point), Typ_PT, 10000);

  // load pTab from bin.file; mallocs Ptab !
  i1 = MSH_bload_pTab (&pTab, AP_modact_nam, dbi);
  if(i1 < 0) return -1;
  ptNr = pTab.rNr;
    // printf(" pMsh_dbi-ptNr=%d\n",ptNr);

  // get bounding box <-- points
  i1 = UT3D_box_pts (pb1, pb2, ptNr, pTab.data);

  MemTab_free (&pTab);

    // printf("::::::::::::::::::::::::: ex BBX__pMsh_dbi: ::::::::::: \n");
    // DEB_dump_obj__ (Typ_PT, &pb1, "pb1");
    // DEB_dump_obj__ (Typ_PT, &pb2, "pb2");

  return 0;

}


//================================================================
  int UT3D_box_Sphere (Point *pb1, Point *pb2, Sphere *sp1) {
//================================================================
// 2 points; cen + r-vx + r-vy + r-vz

  Point     p1, p2;

  // DEB_dump_obj__ (Typ_SPH, sp1, " UT3D_box_Sphere ");

  UT3D_pt_traptmultvc (&p1, &sp1->pc, &WC_sur_act.vx, sp1->rad)

  UT3D_pt_traptmultvc (&p2, &p1, &WC_sur_act.vy, sp1->rad)

  UT3D_box_extend (pb1, pb2, &p2);

  UT3D_pt_opp2pt (&p1, &sp1->pc, &p2);

  UT3D_box_extend (pb1, pb2, &p1);

  return 0;

}





//================================================================
  int UT3D_box_CurvClot (Point *p1, Point *p2, CurvClot *clt) {
//================================================================

  Point    ptx;


  UT3D_box_extend (p1, p2, &clt->stp);  // startpoint

  UT3D_ptvc_evparclot (&ptx, NULL, 1, clt, 1.);    // get ptx = endpoint
    // GR_tDyn_symB__ (&ptx, SYM_STAR_S, ATT_COL_RED);
  UT3D_box_extend (p1, p2, &ptx);

  return 0;

}


//================================================================
  int UT3D_box_AText (Point *p1, Point *p2, AText *tx1) {
//================================================================

  int     irc;

  // DEB_dump_obj__ (Typ_ATXT, tx1, "UT3D_box_AText");
  


  //================================================================
  if((tx1->aTyp == 0) ||       // 2D-Text
     (tx1->aTyp == 2))   {     // Tag

    // 2 = Tag = fixed size; cannot be used for next scale; ignore ..

    UT3D_box_extend (p1, p2, &tx1->p1);
    goto L_ldr;


  //================================================================
  } else if(tx1->aTyp == 1) {       // Image

    int    sx, sy, dx, dy, typ, pNr;
    long   dli;
    double dsx, dsy, dsz;
    Point  pll, pur;

    // get image-size sx, sy
    irc = GR_img_get_obj (&typ, &pll, &sx, &sy, &dx, &dy, tx1);
    if(irc < 0) return -1;
      // DEB_dump_obj__ (Typ_PT, &pll, "pll");
      // printf(" sx=%d sy=%d dx=%d dy=%d\n",sx,sy,dx,dy);
      // sx=100; sy=100; // TEST ONLY


    // pll = p1 + dx, dy
    // dsx += dx;
    // dsy += dy;
    // GL_Sk2Uk (&pll.x, &pll.y, &pll.z,  dsx, dsy, dsz);
      // GR_tDyn_symB__ (&pll, SYM_STAR_S, ATT_COL_RED);
      // DEB_dump_obj__(Typ_PT, &pll, " pll=");


    UT3D_box_extend (p1, p2, &pll);

    // change Textpoint --> Screencoords
    GL_Uk2Sk (&dsx, &dsy, &dsz, pll.x, pll.y, pll.z);
      // printf(" LL(ScrCoords) dsx=%f dsy=%f dsz=%f\n",dsx,dsy,dsz);

    // add size of box
    dsx += sx;
    dsy += sy;
      // printf(" UR(ScrCoords) dsx=%f dsy=%f\n",dsx,dsy);


    // change upper-right --> UserCoords
    GL_Sk2Uk (&pur.x, &pur.y, &pur.z,  dsx, dsy, dsz);
      // GR_tDyn_symB__ (&pur, SYM_STAR_S, ATT_COL_RED);
      // DEB_dump_obj__(Typ_PT, &pur, " pur=");
  
    UT3D_box_extend (p1, p2, &pur);
    goto L_ldr;


  //================================================================
  } else if(tx1->aTyp == 3) {       // LeaderLine + Balloon + 3D-Text

    int       sLen;
    double    rdc, cw, bw, scale;
    Point     ptc, pb1, pb2;

      // GR_tDyn_symB__ (&tx1->p1, SYM_STAR_S, ATT_COL_RED);

    // compute radius of balloon; see also GL_set_txt__
    scale = 1.;
    sLen  = tx1->xSiz;
    cw = GTX_chw_ (scale);
    bw = sLen * cw;
    rdc = (bw + cw) / 2.;
      // printf(" rdc=%lf\n",rdc);

    UT3D_pt_tra_pt_dx (&ptc, &tx1->p1, rdc);
      // DEB_dump_obj__(Typ_PT, &ptc, " ptc=");
      // GR_tDyn_symB__ (&ptc, SYM_STAR_S, ATT_COL_RED);

    UT3D_pt_tra_pt_dy (&pb1, &tx1->p1, -rdc);
    UT3D_box_extend (p1, p2, &pb1);

    UT3D_pt_opp2pt (&pb2, &tx1->p1, &pb1);
    UT3D_box_extend (p1, p2, &pb2);
    goto L_ldr;


  //================================================================
  } else if(tx1->aTyp == 4) {       // LeaderLine + 3D-Text

    int       sLen;
    double    cw, bw, scale;
    Point     pb2;

    UT3D_box_extend (p1, p2, &tx1->p1);
    UT3D_box_extend (p1, p2, &tx1->p2);

    // compute upper right pt of textblock
    scale = 1.;
    sLen  = tx1->xSiz;
    cw = GTX_chw_ (scale);
    bw = sLen * cw;

    UT3D_pt_tra_pt_dx (&pb2, &tx1->p1, bw);
      // DEB_dump_obj__(Typ_PT, &pb2, " pb2=");
      // GR_tDyn_symB__ (&pb2, SYM_STAR_S, ATT_COL_RED);

    UT3D_box_extend (p1, p2, &pb2);

 

  //================================================================
  } else {       // typ > 4  =  symbol
     //  5=Symbol SYM_STAR_S (Stern klein) 6=Symbol SYM_TRI_S (Dreieck klein)
     //  7=Symbol SYM_CIR_S (Kreis klein)  8=SYM_SQU_B (Viereck)
     //  9=Vector (normalized)            10=Vector (true length)

    UT3D_box_extend (p1, p2, &tx1->p1);


    if(tx1->aTyp == 10) {   // 10 = Vector (true length)
      // p2 is vector; add to p1
      Point     pb2;
      UT3D_pt_traptvc (&pb2, &tx1->p1, (Vector*)&tx1->p2);
      UT3D_box_extend (p1, p2, &pb2);
    }


  }


  //================================================================
  L_exit:

  return 0;


  //================================================================
  // use startpoint leaderline
  L_ldr:
    if(tx1->ltyp >= 0) UT3D_box_extend (p1, p2, &tx1->p2);
    goto L_exit;


}


//================================================================
  int UT3D_box_Dim3 (Point *p1, Point *p2, Dim3 *dim1) {
//================================================================
// APT_DrawDimen

  Point   *pt1;


  // DEB_dump_obj__ (Typ_Dim3, dim1, "UT3D_box_Dim3");

  pt1 = DB_get_PT (dim1->ip1);
  UT3D_box_extend (p1, p2, pt1);

  pt1 = DB_get_PT (dim1->ip2);
  UT3D_box_extend (p1, p2, pt1);

  pt1 = DB_get_PT (dim1->ipt);
  UT3D_box_extend (p1, p2, pt1);

  return 0;

}


//================================================================
  int UT3D_box_Dimen (Point *p1, Point *p2, Dimen *dim1) {
//================================================================
// TODO: project p3 onto lines from p1, p2 ..
// APT_DrawDimen

  Point   pt1;


  // DEB_dump_obj__ (Typ_Dimen, dim1, "UT3D_box_Dimen");

  pt1 = UT3D_pt_pt2 (&dim1->p1);
  UT3D_box_extend (p1, p2, &pt1);
  pt1 = UT3D_pt_pt2 (&dim1->p2);
  UT3D_box_extend (p1, p2, &pt1);

  if(dim1->p3.x != UT_DB_LEER) {
    pt1 = UT3D_pt_pt2 (&dim1->p1);
    UT3D_box_extend (p1, p2, &pt1);
  }

  return 0;

}


//================================================================
  int UT3D_box_GText (Point *p1, Point *p2, GText *tx1) {
//================================================================
// See:
// GTX_chw_
// GR_gtx_ckBlockWidth
// GTX_chh_


  int       cnr, lnr;
  double    cw, ch, bw, bh, tSiz, aa;
  Point     pt1, pt2;
  Vector    vp1, vp2;
  Point2    p21, p22, p23;


  // DEB_dump_obj__ (Typ_GTXT, tx1, "UT3D_box_GText");
  // GR_gxt_dump ();
  

  tSiz = tx1->size;
  if(tSiz < 0.001) tSiz = AP_txsiz;
    // printf(" tSiz=%lf\n",tSiz);
  cnr = tx1->xSiz;
  lnr = tx1->ySiz;

  ch =  GTX_chhl_ (tSiz);
  cw =  GTX_chw_ (tSiz);
    // printf(" cw=%lf ch=%lf\n",cw,ch);

  // ch =  GTX_chh_ (tSiz);
  bh = lnr * ch;
  bw = cnr * cw;
    // printf(" bw=%lf bh=%lf\n",bw,bh);


  if(lnr > 1) {
    // vp1 = vector (x = 0, y = - ch * (lnr - 1), z = 0)
    UT3D_vc_3db (&vp1, 0., -ch * (lnr - 1), 0.);
  }

  // vp2 = vector (x = cnr * cw, y = ch, z = 0)
  UT3D_vc_3db (&vp2, cnr * cw, ch, 0.);

  if(!UTP_comp_0(tx1->dir)) {
    aa = UT_RADIANS(tx1->dir) * -1.;
    // rotate vector vy
    if(lnr > 1) {
      vp1 = UT3D_vc_rotvc_angZ (&vp1, aa);
    }
    vp2 = UT3D_vc_rotvc_angZ (&vp2, aa);
  }

  // p1 = lower left of box
  if(lnr > 1) {
    UT3D_pt_traptvc (&pt1, &tx1->pt, &vp1);
  } else {
    pt1 = tx1->pt;
  }

  // p2 = upper right
  UT3D_pt_traptvc (&pt2, &tx1->pt, &vp2);


    // GR_tDyn_symB__ (&tx1->pt, SYM_STAR_S, ATT_COL_GREEN);
    // GR_tDyn_symB__ (&pt1, SYM_STAR_S, ATT_COL_RED);
    // GR_tDyn_symB__ (&pt2, SYM_STAR_S, ATT_COL_RED);
    // DEB_dump_obj__ (Typ_PT, &pt1, " _GText-pt1");
    // GR_tDyn_box__ (&pt1, &pt2, 9);


  UT3D_box_extend (p1, p2, &pt1);
  UT3D_box_extend (p1, p2, &pt2);

  return 0;

}


//================================================================
  int UT3D_box_mdr (Point *p1, Point *p2, ModelRef *mdr) {
//================================================================
// box ModelReference

  int        irc, imdb;
  Mat_4x3    m31;
  ModelBas   *mdb;
  

  // DEB_dump_obj__ (Typ_Model, mdr, "UT3D_box_mdr");

  // create transformation-matrix
  UT3D_m3_load_povxvz (m31, &mdr->po, &mdr->vx, &mdr->vz);
    // DEB_dump_obj__ (Typ_M4x3, m31, " m31 ");

  // get box of basicModel of ModelReference
  imdb = mdr->modNr;
  mdb = DB_get_ModBas (imdb);
    // DEB_dump_obj__ (Typ_SubModel, mdb, " basicModel:");

  // test if box is empty; 1=empty; 0=not
  irc = UT3D_box_ck_empty (&mdb->pb1, &mdb->pb2);
    // printf(" ck_empty = %d\n",irc);
  if(!irc) goto L_apply;

  // box is empty

  // store active DB
  DB_save__ ("");
  // load DB of a basicModel
  DB_load__ (mdb->mnam);
  // get box for subModel 
  UT3D_box_mdl__ (p1, p2, mdr->modNr, 0);
    // DEB_dump_obj__ (Typ_PT, p1, " p1:");
    // DEB_dump_obj__ (Typ_PT, p2, " p2:");
  // reload main-DB
  DB_load__ ("");

  // reload basicModel & store box 
  mdb = DB_get_ModBas (imdb);
  mdb->pb1 = *p1;
  mdb->pb2 = *p2;


  //----------------------------------------------------------------
  L_apply:   // apply box (scale/scale/rotate according to ModelRef)

  // transform the box-points
  *p1 = mdb->pb1;
  *p2 = mdb->pb2;
  UT3D_pt_tra_pt_m3 (p1, m31, p1);
  UT3D_pt_tra_pt_m3 (p2, m31, p2);

    // DEB_dump_obj__ (Typ_PT, p1, " ex box_mdr p1");
    // DEB_dump_obj__ (Typ_PT, p2, " ex box_mdr p2");

  return 0;

}
    
  

//====================================================================
  int UT3D_box_mdl__ (Point *PB1, Point *PB2, int iMdl, int mode) {
//====================================================================
// UT3D_box_mdl__                    get box for model <iMdl>.
// - loop tru dispList, update box for everey obj in dispList;
// Input:
//   iMdl      AP_modact_ibm;  -1=primary-model,
//              else index baseModel of model being created.
//   mode      0=all-objs, 1=group-only (only for primary-model)
// Output:
//   Retcod:   0 = OK;
//            -1 = DispList empty (no obj to display)

  int         i1, typ, bTyp, form, oNr;
  long        dlSiz, dli, dbi;
  void        *obj;
  Point       pb1, pb2;
  DL_Att      *dla;


  // printf("================================================= \n");
  // printf("UT3D_box_mdl__ iMdl=%d mode=%d\n",iMdl,mode);


  // init box
  UT3D_box_ini0 (&pb1, &pb2);

  // get nr of records in DispList
  // dlSiz = GL_Get_DLind ();
  dlSiz = DL_get__ (&dla);
    // printf(" dlSiz=%ld\n",dlSiz);
  if(dlSiz <= 0) return -1;



  // loop tru DispList
  for(dli=0; dli<dlSiz; ++dli) {
      // printf(" dli-nxt: %ld\n",dli);

    // skip dynam. objects
    // if(dla[dli].ind <= 0L) continue;

    // skip objects not in model <iMdl>:
    if((INT_16)dla[dli].modInd != iMdl) continue;

    // skip hidden objects
    if(DL_IS_HIDDEN(dla[dli])) continue;

    // skip invis. obj's
    if(dla[dli].unvis != 0) continue;

    // test group
    if(mode) {if(dla[dli].grp_1) continue;}


    typ = dla[dli].typ;
    dbi = dla[dli].ind;
      // printf(" _box_mdl dli=%ld typ=%d dbi=%ld\n",dli,typ,dbi);

    // skip dynam. objects (see APT_disp_SymB)
    // if(dbi < 0) continue;
    if(typ == Typ_dynSym) continue;

    // skip undisplayable objects
    // skip vectors
    if((typ == Typ_VAR)   ||
       (typ == Typ_VC)    ||
       (typ == Typ_Activ)) continue;


    // get basic typ from typ
    bTyp = AP_typDB_typ (typ);
      // printf(" typ_2_bastyp %d from %d\n",bTyp,typ);


    // get datastruct of dbo
    form = UTO__dbo (&obj, &oNr, bTyp, dbi);
    if(form <= 0) {
      printf("**** ERROR decode typ=%d dbi=%ld\n",typ,dbi);
      continue;
    }

    // have primary obj for A, B !

    // get box for obj
    UT3D_box_obja (&pb1, &pb2, form, oNr, obj);
     

      // TESTBLOCK
      // if(fabs(pb1.x) > UT_INT_MAX) AP_debug__ ("UT3D_box_mdl__");
      // if(fabs(pb1.y) > UT_INT_MAX) AP_debug__ ("UT3D_box_mdl__");
      // if(fabs(pb1.z) > UT_INT_MAX) AP_debug__ ("UT3D_box_mdl__");
      // END TESTBLOCK

  }



  //----------------------------------------------------------------
/*
  // man muesste nun den obersten und den untersten Z-Punkt entlang der
  // Sichtlinie auf die aktuelle Plane werfen
  pt2.x = uxmax; pt2.y = uymax; pt2.z = uzmax;
  pt1 = GL_GetViewPt (&pt2);
  UT3D_box_extend (&pb1, &pb2,(&pt1);

  pt2.x = uxmin; pt2.y = uymin; pt2.z = uzmin;
  pt1 = GL_GetViewPt (&pt2);
  UT3D_box_extend (&pb1, &pb2,(&pt1);
*/


  //----------------------------------------------------------------
    // TESTBLOCK
    // DEB_dump_obj__ (Typ_PT, &pb1, "pb1");
    // DEB_dump_obj__ (Typ_PT, &pb2, "pb2");
    // printf("::::::::::::::::::::::::: ex UT3D_box_mdl__: ::::::::::: \n");
    // END TESTBLOCK

  *PB1 = pb1;
  *PB2 = pb2;

  return 0;

}


//================================================================
  int UT3D_box_obja (Point *pb1, Point *pb2,
                     int form, int oNr, void *obj) {
//================================================================
/// \code
/// UT3D_box_obja       get box for structs
/// Input:
///   form   type of struct of obj
///   oNr    nr of structs in obj
///   retCod 0=ok, 1=skipped obj
/// \endcode
// TODO: Prism Dzt in als (form=204 oNr=1)  MIST ! See myReadme.solids


  int         i1, ibm, iNr, sNr, frm1, typ0, typ1;
  void        *o1;
  double      scl;
  Point       *pta, p1, p2, ps1, ps2;
  Point2      *p2a;
  Vector      vcy;
  ObjGX       *ox1, *oxa;
  ModelBas    *mb;


  // printf("UT3D_box_obja form=%d oNr=%d\n",form, oNr);


    // TESTBLOCK
    // if(form == Typ_ObjGX) {
       // oxa = obj;
       // for(i1=0; i1<oNr; ++i1) {
         // DEB_dump_ox_0 (&oxa[i1], "_box_obja");
       // }
    // } else DEB_dump_obj__ (form, obj, "_box_obja");
    // if(typ0 == 80) AP_debug__ ("UT3D_box_obja");
    // END TESTBLOCK



  //----------------------------------------------------------------
  if((form == Typ_VAR) ||
     (form == Typ_VC))    {
    // skip vectors
    // printf("**** INF UT3D_box_obja skip %d\n",form);
    return 1;


  //----------------------------------------------------------------
  } else if(form == Typ_PT) {
    UT3D_box_extend (pb1, pb2, (Point*)obj);


  //----------------------------------------------------------------
  } else if(form == Typ_LN) {
    // UT3D_box_extend (pb1, pb2, &((Line*)obj)->p1);
    // UT3D_box_extend (pb1, pb2, &((Line*)obj)->p2);
    UT3D_box_ln (pb1, pb2, (Line*)obj);


  //----------------------------------------------------------------
  } else if(form == Typ_CI) {
    UT3D_box_ci (pb1, pb2, (Circ*)obj);


  //----------------------------------------------------------------
  // } else if(form == Typ_SymV) {
    // UT3D_box_ci (pb1, pb2, (Circ*)obj);


  //----------------------------------------------------------------
  // Typ_CV
  //----------------------------------------------------------------

  //----------------------------------------------------------------
  } else if(form == Typ_CVELL) {
    UT3D_box_elli (pb1, pb2, (CurvElli*)obj);

/*
  //-----------------------------------------------------------------
  } else if (typ1 == Typ_CVPOL2) {  // 18
    // p2Tab = cv1.data;
    pt2a = cv1->data;
    iNr  = cv1->siz;
    for(i1=0;i1<iNr; ++i1) {
      pt1.x = pt2a[i1].x;
      pt1.y = pt2a[i1].y;
      pt1.z = 0.;
      UT3D_box_extend (&pb1, &pb2, &pt1);
    }
*/

  //----------------------------------------------------------------
  } else if(form == Typ_CVPOL) {
    pta = ((CurvPoly*)obj)->cpTab;
    iNr = ((CurvPoly*)obj)->ptNr;
    for(i1=0;i1<iNr; ++i1)
      UT3D_box_extend (pb1, pb2, &pta[i1]);


  //----------------------------------------------------------------
  } else if(form == Typ_CVPOL2) {
    p2a = ((CurvPol2*)obj)->pTab;
    iNr = ((CurvPol2*)obj)->ptNr;
    for(i1=0;i1<iNr; ++i1) {
      p1 = UT3D_pt_pt2 (&p2a[i1]);
      UT3D_box_extend (pb1, pb2, &p1);
    }


  //----------------------------------------------------------------
  } else if(form == Typ_CVCLOT) {
    UT3D_box_CurvClot (pb1, pb2, (CurvClot*)obj);


  //----------------------------------------------------------------
  } else if(form == Typ_CVBSP) {
    pta = ((CurvBSpl*)obj)->cpTab;
    iNr = ((CurvBSpl*)obj)->ptNr;
    for(i1=0;i1<iNr; ++i1)
      UT3D_box_extend (pb1, pb2, &pta[i1]);


  //----------------------------------------------------------------
  } else if(form == Typ_CVRBSP) {
    pta = ((CurvRBSpl*)obj)->cpTab;
    iNr = ((CurvRBSpl*)obj)->ptNr;
    for(i1=0;i1<iNr; ++i1)
      UT3D_box_extend (pb1, pb2, &pta[i1]);



/*
  //-----------------------------------------------------------------
  } else if (typ1 == Typ_CVPSP3) {  // 11
    // cvpsp3 = cv1->data;
    cvpol3 = (polynom_d3*)cv1->data;
    iNr  = cv1->siz;
    for(i1=0;i1<iNr; ++i1) {
      pt1.x = cvpol3[i1].x.a;
      pt1.y = cvpol3[i1].y.a;
      pt1.z = cvpol3[i1].z.a;
      UT3D_box_extend (&pb1, &pb2, &pt1);
      // printf(" seg %d %f %f %f\n",i1,pt1.x,pt1.y,pt1.z);
    }
*/

  //-----------------------------------------------------------------
  } else if (form == Typ_CVTRM) {
    for(i1=0;i1<oNr; ++i1)
      UT3D_box_ccv (pb1, pb2, &((CurvCCV*)obj)[i1]);


  //----------------------------------------------------------------
  // Typ_PLN, Typ_SUR 40-79
  //----------------------------------------------------------------
  } else if (form == Typ_PLN) {
    UT3D_box_extend (pb1, pb2, &((Plane*)obj)->po);


  //----------------------------------------------------------------
  } else if(form == Typ_ObjGX) {
    typ0 = ((ObjGX*)obj)->typ;
       // printf(" _obja-ObjGX typ=%d form=%d\n",typ0,((ObjGX*)obj)->form);

    switch (typ0) {

      case Typ_SUR:
        return UT3D_box_surPln (pb1, pb2, (ObjGX*)obj); 

      case Typ_SUTP:
        return UT3D_box_surPln (pb1, pb2, (ObjGX*)obj); 

      case Typ_SURSWP:
        return UT3D_box_surSwp (pb1, pb2, (ObjGX*)obj);

      case Typ_SURBSP:
        return UT3D_box_surBsp (pb1, pb2, (ObjGX*)obj);

      case Typ_SURRV:
        return UT3D_box_surRV (pb1, pb2, (ObjGX*)obj);

      case Typ_SURRU:
        return UT3D_box_surRU (pb1, pb2, (ObjGX*)obj); 

      case Typ_SURCIR:
        return UT3D_box_surFan (pb1, pb2, (ObjGX*)obj);

      case Typ_SURSTRIP:
        return UT3D_box_surStripe (pb1, pb2, (ObjGX*)obj);

      case Typ_SURPTAB:        // group-of-points
      case Typ_SURPMSH:        // Mesh from PTAB
        // obj is a link to the A-index of the PTAB
        return UT3D_box_surPMsh (pb1, pb2, (ObjGX*)obj);

      default:
        // TX_Print("***** UT3D_box_obja TODO ObjGX typ %d",typ0);
        printf("***** UT3D_box_obja TODO ObjGX typ %d\n",typ0);
        return -1;
    }


    // resolve direct: Typ_SURCIR (points)
    oxa = obj;

    for(i1=0; i1<oNr; ++i1) {
      ox1 = &oxa[i1];
        // DEB_dump_obj__ (Typ_ObjGX, ox1, " _obja ox1");

      // must resolv Typ_Index; UT3D_box_obja cannot do it (typ missing)
      if(ox1->form == Typ_Index) {
        typ1 = ox1->typ;
        frm1 = UTO__dbo (&o1, &sNr, typ1, LONG_PTR(ox1->data));
        UT3D_box_obja (pb1, pb2, frm1, sNr, o1);
        continue;
 
      } else {
        UT3D_box_obja (pb1, pb2, ox1->form, ox1->siz, ox1->data);
      }
    }

    
/*
  //----------------------------------------------------------------
    UT3D_box_ox (&pb1, &pb2, ox1);  // siehe UT3D_npt_ox__
    // if(ox1->form == Typ_SUR) {
      // man sollte eine Box von der ersten Kontur holen;
      // die Boxpunkte UT3D_box_extend (&pb1, &pb2,
      // siehe GR_DrawSur/SUP_load_c .., UT3D_npt_ox__
      // UT3D_box_ox (&pb1, &pb2, ox1->data);  // siehe UT3D_npt_ox__
    // } else if (ox1->form == Typ_ObjGX) {
    // }

    if(oxi->form == Typ_SUR) {
      sus = oxi->data;
      // printf("    in SurStd: typ=%d ind=%d\n",sus->cvTyp,sus->cvID);

      if((sus->cvTyp == Typ_AC)||(sus->cvTyp == Typ_CI)) {
        cip1 = DB_get_CI (sus->cvID);
        UT3D_box_extend (pb1, pb2, &cip1->pc);

      } else if(sus->cvTyp == Typ_CV) {
        ox1 = DB_GetCurv (sus->cvID);
        UT3D_box_obj (pb1, pb2, ox1);  // recu!
      }
    }
*/

  //----------------------------------------------------------------
  // Typ_SOL   80-89
  //----------------------------------------------------------------
  } else if (form == Typ_CON) {
    return UT3D_box_Conus (pb1, pb2, (Conus*)obj);


  } else if (form == Typ_SPH) {
    return UT3D_box_Sphere (pb1, pb2, (Sphere*)obj);


  } else if (form == Typ_TOR) {
    return UT3D_box_Torus (pb1, pb2, (Torus*)obj);


  // } else if (form == Typ_PRI) {
    // return UT3D_box_Prism (pb1, pb2, obj);





  //----------------------------------------------------------------
  // Typ_Note   90-109
  //----------------------------------------------------------------
  } else if(form == Typ_ATXT) {
    return UT3D_box_AText (pb1, pb2, (AText*)obj);


  } else if(form == Typ_GTXT) {
    return UT3D_box_GText (pb1, pb2, (GText*)obj);


  } else if(form == Typ_Dimen) {
    return UT3D_box_Dimen (pb1, pb2, (Dimen*)obj);


  } else if(form == Typ_Dim3) {
    return UT3D_box_Dim3 (pb1, pb2, (Dim3*)obj);



  //----------------------------------------------------------------
  // model-reference = ditto = subModel (obj = ModelRef*)
  //----------------------------------------------------------------
  } else if (form == Typ_Model) {      // ditto, subModel
    // get mb = basic-model (struct ModelBas)
    ibm = ((ModelRef*)obj)->modNr;
    mb = DB_get_ModBas (ibm);
      // DEB_dump_obj__ (Typ_SubModel, mb, "_box_obja-mb-bm%d",ibm);

    // test if model is empty (UT3D_box_ini0); yes: exit.
    if(mb->pb1.x == UT_VAL_MAX) return 0;

    // support of internal-subModels (mb->typ MBTYP_INTERN)

    // Test if boxPoints pb1,pb2 is empty or not;
    // if(UT3D_box_mdl_ck_pb(mb)) {
    if(DB_isFree_PT(&mb->pb2)) {
      // box does not yet exist; get it (recursion ..)
      TX_Print ("***** UT3D_box_obja E001");
      // UT3D_box_mdl__ (&p1, &p2, ibm); - in Mod_load_sm !
    } else {
     // box already already existing; use it
     if(!UTP_comp2db (((ModelRef*)obj)->scl, 1., UT_TOL_min0)) {
       // scale points
       UT3D_pt_multpt (&ps1, &mb->pb1, ((ModelRef*)obj)->scl);
       UT3D_pt_multpt (&ps2, &mb->pb2, ((ModelRef*)obj)->scl);

     } else {
       //scale = 1.
       ps1 = mb->pb1;
       ps2 = mb->pb2;
     }
    }
      // DEB_dump_obj__ (Typ_PT, &ps1, "_box_obja-ps1-bm%d",ibm);
      // DEB_dump_obj__ (Typ_PT, &ps2, "_box_obja-ps2-0");

    // get y-axis for model-reference
    UT3D_vc_perp2vc (&vcy, &((ModelRef*)obj)->vz, &((ModelRef*)obj)->vx);
      // DEB_dump_obj__ (Typ_VC, &vcy, "_box_obja-vcy");
      // printf(" WC_sur_ind=%d\n",WC_sur_ind);

    // p1,p2 = translate points ps1,ps2 into active refSys WC_sur_act
    p1 = ((ModelRef*)obj)->po;
    p2 = ((ModelRef*)obj)->po;
      // DEB_dump_obj__ (Typ_PT, &p1, "_box_obja-p1-p2");
    UT3D_pt_add_3vc_3par (&p1,
                          &((ModelRef*)obj)->vx, &vcy, &((ModelRef*)obj)->vz,
                          ps1.x, ps1.y, ps1.z);
    UT3D_pt_add_3vc_3par (&p2,
                          &((ModelRef*)obj)->vx, &vcy, &((ModelRef*)obj)->vz,
                          ps2.x, ps2.y, ps2.z);
      // DEB_dump_obj__ (Typ_PT, &p1, "_box_obja-p1-3");
      // DEB_dump_obj__ (Typ_PT, &p2, "_box_obja-p2-3");

    UT3D_box_extend (pb1, pb2, &p1);
    UT3D_box_extend (pb1, pb2, &p2);



  //----------------------------------------------------------------
  } else {
     printf("**** UT3D_box_obja: Objekttyp not supported %d\n",form);
     return -1;
  }



  return 0;

}

//================================================================
  int UT3D_box_mdl_ck_pb (ModelBas* mb) {
//================================================================
/// \code
/// Test if boxPoints pb1,pb2 of basic model mb are empty or not;
///
/// Output:
///   retCod     0 = box already already existing (pb1, pb2 have values)
///              1 = undefined
/// \endcode


  Vector    vc1;


  // get vector of pb1,pb2
  UT3D_vc_2pt (&vc1, &mb->pb1, &mb->pb2);

  // test if vector == NULL
  return UT3D_compvc0 (&vc1, UT_TOL_min0);

}

 
//================================================================
  int UT3D_box_surPMsh (Point *pb1, Point *pb2, ObjGX *ox1) {
//================================================================
// Typ_SURPMSH see TSU_DrawSurPMsh

  int    typ;
  long   dbi;


  // DEB_dump_obj__ (Typ_ObjGX, ox1, "UT3D_box_surPMsh");

  OGX_GET_INDEX (&typ, &dbi, ox1);
    // printf(" surMsh-typ %d dbi %ld\n",typ,dbi);

  return BBX__pMsh_dbi (pb1, pb2, dbi);

}



//================================================================
  int UT3D_box_surFan (Point *pb1, Point *pb2, ObjGX *ox1) {
//================================================================
// UT3D_box_surFan          box from Typ_SURCIR = group of points
//     sus1 = ox1->data;
//     pNr = sus1->ptUNr * sus1->ptVNr;
//     // get spc for points
//     pta = (Point*) MEM_alloc_tmp (pNr * sizeof(Point));
//     // datablock pTab is group of ObjGX-points
//     iNr = 0;
//     i1 = UT3D_npt_obj (&iNr, pta, pNr,
//                        Typ_ObjGX, sus1->pTab, pNr, UT_DISP_cv);





  int          i1, typ, ptNr;
  long         dbi;
  ObjGX        *oxa;
  Point        *px;


  // DEB_dump_obj__ (Typ_ObjGX, ox1, "UT3D_box_surFan");

  oxa= ox1->data;
  // get nr of points
  ptNr = ox1->siz;

  for(i1=0; i1<ptNr; ++i1) {
    OGX_GET_INDEX (&typ, &dbi, &oxa[i1]);
    if(typ == Typ_PT) {
      px = DB_get_PT (dbi);
      UT3D_box_extend (pb1, pb2, px);
    } else printf("**** UT3D_box_surFan E001 %d\n",typ);
  }

  return 0;

}


//================================================================
  int UT3D_box_surStripe (Point *pb1, Point *pb2, ObjGX *ox1) {
//================================================================
// 
//     sus1 = ox1->data;
//     pNr = sus1->ptUNr * sus1->ptVNr;
//     // get spc for points
//     pta = (Point*) MEM_alloc_tmp (pNr * sizeof(Point));
//     // datablock pTab is group of ObjGX-points
//     iNr = 0;
//     i1 = UT3D_npt_obj (&iNr, pta, pNr,
//                        Typ_ObjGX, sus1->pTab, pNr, UT_DISP_cv);



  int          i1, i2, ii, typ;
  ObjGX        *oxa;
  SurStripe    *su1;
  Point        *px;

  su1 = ox1->data;

  // printf("UT3D_box_surStripe %d %d\n",su1->ptUNr, su1->ptVNr);

  // get nr of points
  ii = su1->ptUNr * su1->ptVNr;

  // resolv 
  oxa = su1->pTab;
  for(i1=0; i1<ii; ++i1) {
    // get obj from ObjGX
    typ = UTO_objDat_ox (&px, &i2, &oxa[i1]);
    if(typ == Typ_PT) UT3D_box_extend (pb1, pb2, px);
    else printf("**** UT3D_box_surStripe E001 %d\n",typ);
  }

  return 0;

}


//================================================================
  int UT3D_box_surRU (Point *pb1, Point *pb2, ObjGX *ox1) {
//================================================================
/// \code
/// box of ruled-surface.
/// Retcod:  0 = ok,
///          1 = 2 normal objects, make box of each obj.
/// \endcode


  int         i1, irc;
  Point       pta[2];
  Vector      *vc1;
  ObjGX       *oa;


  
  oa = ox1->data;
   
    // printf("UT3D_box_surRU %d %d\n",oa[0].typ,oa[1].typ);

  // SRU = 2 ObjGX-objects;
  // test if  2. obj is vec; no: meke box of both objs.
  if(oa[1].typ == Typ_VC) goto L_RU_vc;

  // 2 normal objects ..
  return UT3D_box_obja (pb1, pb2, Typ_ObjGX, 2, oa);


  L_RU_vc:
  // get start/endpoint for curve ox1[0]; then translate with vec ox1[1]
  // UTO_pt_ox  (&pta, oa, Ptyp_start|Ptyp_end);
  irc = UT3D_ptvcpar_std_obj (&pta[0], NULL, NULL, 0, Ptyp_start, Typ_ObjGX, oa);
  irc = UT3D_ptvcpar_std_obj (&pta[1], NULL, NULL, 0, Ptyp_end, Typ_ObjGX, oa);

  UT3D_box_extend (pb1, pb2, &pta[0]);
  UT3D_box_extend (pb1, pb2, &pta[1]);

  // get vc1 from ox1[1]
  UTO_objDat_ox (&vc1, &i1, &oa[1]);

  // translate with vec ox1[1]
  UT3D_pt_traptvc (&pta[0], &pta[0], vc1);
  UT3D_box_extend (pb1, pb2, &pta[0]);
  UT3D_pt_traptvc (&pta[1], &pta[1], vc1);
  UT3D_box_extend (pb1, pb2, &pta[1]);

  return 0;

}


//================================================================
  int UT3D_box_surRV (Point *pb1, Point *pb2, ObjGX *ox1) {
//================================================================
/// \code
/// box of revolved-surface.
/// \endcode

  int         i1, iNr;
  SurRev      *srv;
  ObjGX       o1;


  srv = ox1->data;

  o1 = DB_GetObjGX ((int)srv->typCov, srv->indCov);
  // typ2 = UTO_obj_get (&o1, &o1);

// TODO: rotate contourCurve ang1, ang2, trim with v0, v1;
//       or use boundarycurve of SurRev

  UT3D_box_obja (pb1, pb2, o1.form, 1, o1.data);

  return 0;

}


//================================================================
  int UT3D_box_surBsp (Point *pb1, Point *pb2, ObjGX *ox1) {
//================================================================
/// \code
/// box of Bspl-surface.
/// \endcode

  int         i1, iNr;
  SurBSpl     *sub1;


  // check controlPoints
  sub1 = ox1->data;
  iNr = sub1->ptUNr * sub1->ptVNr;

  for(i1=0; i1<iNr; ++i1) {
    UT3D_box_extend (pb1, pb2, &sub1->cpTab[i1]);
  }

  return 0;

}

//================================================================
  int UT3D_box_surSwp (Point *pb1, Point *pb2, ObjGX *ox1) {
//================================================================
/// \code
/// box of SweepSurf
/// TODO: use points from creation (Tess_sur__ or Tess_sSym__)
/// \endcode

  int       typ, ii;
  long      dbi;
  void      *obj;
  SurSwp    *su1;


  // DEB_dump_obj__ (Typ_ObjGX, ox1, "UT3D_box_surSwp");

  // get path, box
  su1 = (SurSwp*)ox1->data;
  typ = su1->typPath;
  dbi = su1->indPath;
  UTO_obj_dbo (&obj, &ii, &typ, dbi);
  UT3D_box_obja (pb1, pb2, typ, ii, obj);


  // get cov, box
  typ = su1->typCov;
  dbi = su1->indCov;
  UTO_obj_dbo (&obj, &ii, &typ, dbi);
  UT3D_box_obja (pb1, pb2, typ, ii, obj);

  return 0;

}


//================================================================
  int UT3D_box_surPln (Point *pb1, Point *pb2, ObjGX *ox1) {
//================================================================
/// \code
/// box of planar surface.
/// \endcode


  int         sNr, frm1, typ1;
  void        *o1;
  ObjGX       *oxa, *ox2;


  // DEB_dump_obj__ (Typ_ObjGX, ox1, "UT3D_box_surPln");

  oxa = ox1->data;

  // do not use supporting surf|body for surface; (ox[0])
  //   supporting surf can be typ0=Typ_SUR; typ1=Typ_Typ value=Typ_SURPLN
  //   use only outer boundary,                   (ox[1])
  //   and ignore inner boundaries.               (ox[2]-)
  ox2 = &oxa[1];
  
  typ1 = ox2->typ;
  frm1 = UTO__dbo (&o1, &sNr, typ1, LONG_PTR(ox2->data));
  UT3D_box_obja (pb1, pb2, frm1, sNr, o1);

  return 0;

}



//================================================================
  int UT3D_box_SurStd (Point *pb1, Point *pb2, SurStd *sus) {
//================================================================
/// \code
/// Box mit obj vergroessern; Box muss auf Maximum initialisiert sein !
/// siehe DL_ReScale_pt_ini
/// uses UT3D_box_extend
/// \endcode


  DEB_dump_obj__ (Typ_SUR, sus, "UT3D_box_SurStd");  fflush (stdout);

  return 0;

}


//================================================================
  int UT3D_box_ccv (Point *pb1, Point *pb2, CurvCCV *ccv) {
//================================================================
/// \code
/// Box mit obj vergroessern; Box muss auf Maximum initialisiert sein !
/// siehe DL_ReScale_pt_ini
/// uses UT3D_box_extend
/// \endcode


  int     typ1;
  char    obj1[OBJ_SIZ_MAX];


  // DEB_dump_obj__ (Typ_CVTRM, ccv, "UT3D_box_ccv");  fflush (stdout);

  // make a normal struct of trimmed-curve
  UTO_cv_cvtrm (&typ1, obj1, NULL, ccv);
    // DEB_dump_obj__ (typ1, obj1, " ccv-obj1");


  // resolv subCurve
  UT3D_box_obja (pb1, pb2, typ1, 1, obj1);

  return 0;

}


/*
//================================================================
  int UT3D_box_ox (Point *pb1, Point *pb2, ObjGX *oxi) {
//================================================================
/// \code
/// Box mit obj vergroessern; Box muss auf Maximum initialisiert sein !
/// siehe DL_ReScale_pt_ini
/// uses UT3D_box_extend
/// \endcode

  int       i1;
  Circ      *cip1;
  SurStd    *sus;
  ObjGX     *ox1, *ox2, *oTab;
  void      *xu;
  CurvCCV   *ccva, *ccv1;


  printf("UT3D_box_ox typ=%d form=%d siz=%d\n",oxi->typ,oxi->form,oxi->siz);


  //----------------------------------------------------------------
  if(oxi->typ == Typ_LN) {
    UTO_objDat_ox (&xu, &i1, oxi);
    UT3D_box_extend (pb1, pb2, &((Line*)xu)->p1);
    UT3D_box_extend (pb1, pb2, &((Line*)xu)->p2);
    // see also UT3D_box_model



  //----------------------------------------------------------------
  // man sollte hier SUR-Obj via UTO_objx_get holen (damit DB_Get-Calls weg!)
  } else if(oxi->typ == Typ_SUR) {

    if(oxi->form == Typ_SUR) {
      sus = oxi->data;
      // printf("    in SurStd: typ=%d ind=%d\n",sus->cvTyp,sus->cvID);

      if(sus->cvTyp == Typ_CI) {
        cip1 = DB_get_CI (sus->cvID);
        UT3D_box_extend (pb1, pb2, &cip1->pc);
        // see also UT3D_box_model


      } else if(sus->cvTyp == Typ_CV) {
        ox1 = DB_GetCurv (sus->cvID);
        UT3D_box_ox (pb1, pb2, ox1);  // recu!
        // see also UT3D_box_model
      }
    }


  //----------------------------------------------------------------
  } else if(oxi->typ == Typ_CVTRM) {
    // oTab  = oxi->data;
    ccva = oxi->data;
    for(i1=0; i1<oxi->siz; ++i1) {
      ccv1 = &ccva[i1];
        printf(" cv[%d] - dbi=%ld ip0=%ld ip1=%ld\n", i1,
               ccv1->dbi, ccv1->ip0, ccv1->ip1);
      // get basic-curve
      // get box from dbi-obj
        // ox1 = &oTab[i1];
        // UT3D_box_ox (pb1, pb2, ox1);  // recu!
    }

  // } else if(oxi->typ == Typ_CVBSP) {

  }




/
  // int       i1, cvNr;
  // CurvCCV   *ccva, *cv1;
  // cvNr = ccv->siz;
  // for(i1=0; i1<cvNr;  ++i1) {
  // }
/

  return 0;

}
*/


//================================================================
  int UT3D_box_elli (Point *p1, Point *p2, CurvElli *el1) {
//================================================================
/// \code
/// box ellipse
///  360-deg: 4 points; p1 = cen + r-vx + r-vy
///                     p2 = cen - r-vx - r-vy
///  else  check p1, p2 and all points at 0,90,180,270-deg.
/// \endcode

  int      i1;
  Point    px;

  i1 = UT3D_ck_el360 (el1);
  if (i1) goto L_elSeg;


  //----------------------------------------------------------------
  // 360-deg-ellipse
  UT3D_box_extend (p1, p2, &el1->p1);

  UT3D_pt_opp2pt(&px, &el1->pc, &el1->p1);
  UT3D_box_extend (p1, p2, &px);

  UT3D_pt_traptvc (&px, &el1->pc, &el1->vb);
  UT3D_box_extend (p1, p2, &px);

  UT3D_pt_opp2pt(&px, &el1->pc, &px);
  UT3D_box_extend (p1, p2, &px);

  goto L_exit;


  //----------------------------------------------------------------
  L_elSeg:
  UT3D_box_extend (p1, p2, &el1->p1);
  UT3D_box_extend (p1, p2, &el1->p2);
  // TODO: 0,90,180,270 deg if included
    // printf("***** UT3D_box_elli TODO \n");


  //----------------------------------------------------------------
  L_exit:
  return 0;

}


//================================================================
  int UT3D_box_ln (Point *p1, Point *p2, Line *ln1) {
//================================================================
/// \code
/// UT3D_box_ln           box Line 
///   unlimited lines: only definitionpoint
/// if UNL:   midpoint;
/// if UNL1:  startpt
/// if UNL1:  endpt
/// \endcode


  // Vector   vl1, vl2;
  Point    px;


  // DEB_dump_obj__ (Typ_LN, ln1, "UT3D_box_ln");


  switch(ln1->typ) {

    case 0:     // both sides limited
      UT3D_box_extend (p1, p2, &ln1->p1);
      UT3D_box_extend (p1, p2, &ln1->p2);
      break;

    case 1:     // p1 = limitpoint
      UT3D_box_extend (p1, p2, &ln1->p1);
      // UNL1:  add 1 to startpt
      // UT3D_vc_ln (&vl1, ln1);
      // UT3D_vc_div_d (&vl2, &vl1, UT_DISP_ln);
      // // px = ln1->p1 + vl2
      // px = ln1->p1;
      // UT3D_pt_add_vc__ (&px, &vl2);
      // UT3D_box_extend (p1, p2, &px);
      break;

    case 2:     // p2 = limitpoint
      UT3D_box_extend (p1, p2, &ln1->p2);
      break;

    case 3:     // 1 both sides unlimited
      // UNL: add +0.5 /-0.5 to midpoint;
      UT3D_pt_mid2pt (&px, &ln1->p1, &ln1->p2);
      UT3D_box_extend (p1, p2, &px);
      break;
  }




  //----------------------------------------------------------------
  L_exit:
    // DEB_dump_obj__ (Typ_PT, p1, "-p1");
    // DEB_dump_obj__ (Typ_PT, p2, "-p2");

  return 0;

}


//================================================================
  int UT3D_box_ci (Point *p1, Point *p2, Circ *ci1) {
//================================================================
/// \code
/// box CIRC 
///  360-deg: 2 points; p1 = cen + r-vx + r-vy
///                     p2 = cen - r-vx - r-vy
///  else  check p1, p2 and all points at 0,90,180,270-deg.
/// \endcode


  int      irc;
  double   rda;
  Vector   vx, vy;
  Point    px;


  // DEB_dump_obj__ (Typ_CI, ci1, "UT3D_box_ci");



  // get vx, vy from vz  
  UT3D_vc_perp2vc (&vx, &WC_sur_act.vy, &ci1->vz);
  UT3D_vc_perp2vc (&vy, &ci1->vz, &WC_sur_act.vx);
    // DEB_dump_obj__ (Typ_VC, &vx, "-vx");
    // DEB_dump_obj__ (Typ_VC, &vy, "-vy");

  // multiply with radius
  rda = fabs(ci1->rad);
    // printf(" rda=%f\n",rda);

  UT3D_vc_multvc (&vx, &vx, rda);
  UT3D_vc_multvc (&vy, &vy, rda);
    // DEB_dump_obj__ (Typ_VC, &vx, "-vx");
    // DEB_dump_obj__ (Typ_VC, &vy, "-vy");


  // check 360 deg
  if(UT3D_ck_ci360(ci1))  goto L_arc;


  //----------------------------------------------------------------
  // 360 deg:

  // add vx & vy
  UT3D_pt_traptvc (&px, &ci1->pc, &vx);
  UT3D_pt_traptvc (&px, &px, &vy);
  UT3D_box_extend (p1, p2, &px);


  // add -vx & -vy
  UT3D_pt_opp2pt(&px, &ci1->pc, &px);
  UT3D_box_extend (p1, p2, &px);


  goto L_exit;


  //----------------------------------------------------------------
  L_arc:
  // check p1, p2 and all points at 0,90,180,270-deg.
  UT3D_box_extend (p1, p2, &ci1->p1);
  UT3D_box_extend (p1, p2, &ci1->p2);

  //----------------
  // point at 0 deg
  UT3D_pt_traptvc (&px, &ci1->pc, &vx);
  // test if point px is in arc
  irc = UT3D_pt_ck_inCirc (ci1, &px, UT_TOL_pt);
    // printf(" 0-deg:%d\n",irc);
  if(irc == 1) UT3D_box_extend (p1, p2, &px);

// TODO: faster if add 3 times pi/2 to the 0-deg-angle and test with ango ..

  //----------------
  // point at 90 deg
  UT3D_pt_traptvc (&px, &ci1->pc, &vy);
  // test if point px is in arc
  irc = UT3D_pt_ck_inCirc (ci1, &px, UT_TOL_pt);
    // printf(" 90-deg:%d\n",irc);
  if(irc == 1) UT3D_box_extend (p1, p2, &px);

  //----------------
  // point at 180 deg
  UT3D_pt_traptivc (&px, &ci1->pc, &vx);
  // test if point px is in arc
  irc = UT3D_pt_ck_inCirc (ci1, &px, UT_TOL_pt);
    // printf(" 180-deg:%d\n",irc);
  if(irc == 1) UT3D_box_extend (p1, p2, &px);

  //----------------
  // point at 270 deg
  UT3D_pt_traptivc (&px, &ci1->pc, &vy);
  // test if point px is in arc
  irc = UT3D_pt_ck_inCirc (ci1, &px, UT_TOL_pt);
    // printf(" 270-deg:%d\n",irc);
  if(irc == 1) UT3D_box_extend (p1, p2, &px);




  //----------------------------------------------------------------
  L_exit:
    // DEB_dump_obj__ (Typ_PT, p1, "-p1");
    // DEB_dump_obj__ (Typ_PT, p2, "-p2");

  return 0;

}


/*
//====================================================================
  int UT3D_box_model (Point *PB1, Point *PB2, int iMdl) {
//====================================================================
// get box for model <iMdl>.
//   Retcod:   0 = OK;
//            -1 = DispList empty (no obj to display)





  // double d1, d2;
  // long   l1, dlSiz, dli, dbi, *ia;
  // int    i1, i2, typ, typ1, att, iNr, ckb_view_stat;

  // Point       pb1, pb2, pt1, pt2, *pta;
  // Point2      p21, p22, p23, *pt2a;
  // Plane       pl1;
  // Mat_4x3     m1;

  int         i1, typ, typ1, iNr, pNr, *ia;
  long        dlSiz, dli, dbi, *la;
  double      d1, d2;
  Point2      p21, p22, p23;
  Point       p1, p2, pb1, pb2, *pt1, *pta;
  Line        *ln1;
  Circ        *ci1;

  CurvElli    *el1;
  ObjGX       *cv1, *oTab;
  ObjGX       *ox1, o1;
  // ObjGX       *suc1;
  SurStripe   *sus1;
  SurBSpl     *sub1;
  SurRev      *srv;
  // CurvPol3    *cvpsp3;
  CurvBSpl    *cvpspl;
  polynom_d3  *cvpol3;
  GText       *tx1;
  Dimen       *dim1;
  ModelRef    *mr1;
  DL_Att      *dla;



  printf("================================================= \n");
  printf("UT3D_box_model %d\n",iMdl);


  // init box
  UT3D_box_ini0 (&pb1, &pb2);

  // get nr of records in DispList
  // dlSiz = GL_Get_DLind ();
  dlSiz = DL_get__ (&dla);
  if(dlSiz <= 0) return -1;
    printf(" dlSiz=%ld\n",dlSiz);


  // loop tru DispList
  for(dli=0; dli<dlSiz; ++dli) {

    // skip objects not in model <iMdl>:
    if((INT_16)dla[dli].modInd != iMdl) continue;

    // skip hidden objects
    if(DL_IS_HIDDEN(dla[dli])) continue;

    // skip invis. obj's
    if(dla[dli].unvis != 0) continue;

    typ = dla[dli].typ;
    dbi = dla[dli].ind;
      printf(" box_actMod dli=%ld typ=%d dbi=%ld\n",dli,typ,dbi);

    // get basic typ from typ
    typ = AP_typDB_typ (typ);



      //----------------------------------------------------------------
      if (typ == Typ_PT) {
        // if(dbi >= dyn_pt_max) continue;
        pt1 = DB_get_PT (dbi);
        UT3D_box_extend (&pb1, &pb2, pt1);


      //----------------------------------------------------------------
      } else if (typ == Typ_LN) {
        // if(dbi >= dyn_ln_max) continue;
        ln1 = DB_get_LN (dbi);
        UT3D_box_extend (&pb1, &pb2, &ln1->p1);
        UT3D_box_extend (&pb1, &pb2, &ln1->p2);



      //----------------------------------------------------------------
      } else if (typ == Typ_CI) {
        ci1 = DB_get_CI (dbi);
        UT3D_box_ci (&p1, &p2, ci1);
        UT3D_box_extend (&pb1, &pb2, &p1);
        UT3D_box_extend (&pb1, &pb2, &p2);

/
      //----------------------------------------------------------------
      } else if (typ == Typ_SymV) {
        if(dbi > 0) {
          pt1 = DB_get_PT (dbi);
          UT3D_box_extend (&pb1, &pb2, pt1);
        }




      } else if (typ == Typ_PLN) {
        // if(dbi >= dyn_ci_max) continue;
        // DB_GetRef (&pl1, m1, dbi);
        DB_GetRef (&pl1, dbi);
        UT3D_box_extend (&pb1, &pb2, &pl1.po);
/


      //-----------------------------------------------------------------
      } else if (typ == Typ_Model) {      // ditto, subModel
        mr1 = DB_get_ModRef (dbi);
        UT3D_box_mdr (&p1, &p2, mr1);
        UT3D_box_extend (&pb1, &pb2, &p1);
        UT3D_box_extend (&pb1, &pb2, &p2);



      //================================================================
      } else if (typ == Typ_CV) {
      //================================================================
        cv1 = DB_GetCurv (dbi);
        typ1 = cv1->typ;
          printf("........Typ_CV = %d ........\n",typ1);


        //-----------------------------------------------------------------
        if(typ1 == Typ_CVPOL) {   // 7
          pta = ((CurvPoly*)cv1->data)->cpTab;
          iNr = ((CurvPoly*)cv1->data)->ptNr;
          for(i1=0;i1<iNr; ++i1) 
            UT3D_box_extend (&pb1, &pb2, &pta[i1]);


        //-----------------------------------------------------------------
        } else if(typ1 == Typ_CVCLOT) {
          UT3D_box_CurvClot (&p1, &p2, ox1->data);
          UT3D_box_extend (&pb1, &pb2, &p1);
          UT3D_box_extend (&pb1, &pb2, &p2);


/
        //-----------------------------------------------------------------
        } else if(typ1 == Typ_CVELL) {
          el1 = cv1->data;
          UT3D_box_extend (&pb1, &pb2, &el1->p1);
          // p1=p2: Vollkreis; auch vis-vis.
          if(UT3D_comp2pt(&el1->p1, &el1->p2, UT_TOL_pt)) {
            UT3D_pt_opp2pt(&pt1, &el1->pc, &el1->p1);
            UT3D_box_extend (&pb1, &pb2, &pt1);
          } else {
            UT3D_box_extend (&pb1, &pb2, &el1->p2);
          }



        //-----------------------------------------------------------------
        } else if (typ1 == Typ_CVPOL2) {  // 18
          // p2Tab = cv1.data;
          pt2a = cv1->data;
          iNr  = cv1->siz;
          for(i1=0;i1<iNr; ++i1) {
            pt1.x = pt2a[i1].x;
            pt1.y = pt2a[i1].y;
            pt1.z = 0.;
            UT3D_box_extend (&pb1, &pb2, &pt1);
          }
/

        //-----------------------------------------------------------------
        } else if (typ1 == Typ_CVBSP) {  // 9
          cvpspl = cv1->data;
          pta = cvpspl->cpTab;
          iNr = cvpspl->ptNr;
          for(i1=0;i1<iNr; ++i1)
            UT3D_box_extend (&pb1, &pb2, &pta[i1]);


/
        //-----------------------------------------------------------------
        } else if (typ1 == Typ_CVPSP3) {  // 11
          // cvpsp3 = cv1->data;
          cvpol3 = (polynom_d3*)cv1->data;
          iNr  = cv1->siz;
          for(i1=0;i1<iNr; ++i1) {
            pt1.x = cvpol3[i1].x.a;
            pt1.y = cvpol3[i1].y.a;
            pt1.z = cvpol3[i1].z.a;
            UT3D_box_extend (&pb1, &pb2, &pt1);
            // printf(" seg %d %f %f %f\n",i1,pt1.x,pt1.y,pt1.z);
          }
/

        //-----------------------------------------------------------------
        } else if (typ1 == Typ_CVTRM) {
          ox1 = DB_GetCurv (dbi);
          UT3D_box_ox (&pb1, &pb2, ox1);


        //-----------------------------------------------------------------
        } else {
          printf("**** DL_ReScale: unused Curvetyp %d\n",typ1);

        }




      //================================================================
      } else if (typ == Typ_Note) {
      //================================================================
        ox1 = DB_GetGTxt (dbi);
        typ1 = ox1->form;
          printf("........Typ_Note = %d ........\n",typ1);

      //-----------------------------------------------------------------
        if(typ1 == Typ_Dimen) {
          dim1 = ox1->data;
          p1 = UT3D_pt_pt2 (&dim1->p1);
          UT3D_box_extend (&pb1, &pb2, &p1);
          p1 = UT3D_pt_pt2 (&dim1->p2);
          UT3D_box_extend (&pb1, &pb2, &p1);
          if(dim1->p3.x != UT_DB_LEER) {
            p1 = UT3D_pt_pt2 (&dim1->p3);
            UT3D_box_extend (&pb1, &pb2, &p1);
          }

/
      //-----------------------------------------------------------------
        } else if(typ1 == Typ_ATXT) {
???
/

      //-----------------------------------------------------------------
        } else if(typ1 == Typ_GTXT) {
          UT3D_box_GText (&p1, &p2, ox1->data);
          UT3D_box_extend (&pb1, &pb2, &p1);
          UT3D_box_extend (&pb1, &pb2, &p2);


        //----------------------------------------------------------------
        } else {
          printf("**** DL_ReScale: unused Note-typ %d\n",typ1);

        }



      //================================================================
      } else if (typ == Typ_SUR) {
      //================================================================
        ox1 = DB_GetSur (dbi, 1);
          printf("  box_actMod-2 typ=%d form=%d\n",ox1->typ,ox1->form);


        //----------------------------------------------------------------
        if(ox1->typ == Typ_SUR) {
          UT3D_box_ox (&pb1, &pb2, ox1);  // siehe UT3D_npt_ox__
          // if(ox1->form == Typ_SUR) {
            // man sollte eine Box von der ersten Kontur holen;
            // die Boxpunkte UT3D_box_extend (&pb1, &pb2,
            // siehe GR_DrawSur/SUP_load_c .., UT3D_npt_ox__
            // UT3D_box_ox (&pb1, &pb2, ox1->data);  // siehe UT3D_npt_ox__
          // } else if (ox1->form == Typ_ObjGX) {
          // }


        //----------------------------------------------------------------
        } else if(ox1->typ == Typ_SURBSP) {
          // check controlPoints
          sub1 = ox1->data;
          iNr = sub1->ptUNr * sub1->ptVNr;
          for(i1=0; i1<iNr; ++i1) {
            UT3D_box_extend (&pb1, &pb2, &sub1->cpTab[i1]);
          }


        //----------------------------------------------------------------
        // } else if(ox1->typ == Typ_SURRU) {
          // oTab = ox1->data;
          // DL_ReScaleObj(&oTab[0]);
          // DL_ReScaleObj(&oTab[1]);


        //----------------------------------------------------------------
        // } else if(ox1->typ == Typ_SURRV) {
          // srv = ox1->data;
          // o1 = DB_GetObjGX ((int)srv->typCov, srv->indCov);
          // typ2 = UTO_obj_get (&o1, &o1);


        //----------------------------------------------------------------
        } else if(ox1->typ == Typ_SURSTRIP) {
          // get nr of points
          sus1 = ox1->data;
          pNr = sus1->ptUNr * sus1->ptVNr;
          // get spc for points
          pta = (Point*) MEM_alloc_tmp (pNr * sizeof(Point));
          // datablock pTab is group of ObjGX-points
          iNr = 0;
          i1 = UT3D_npt_obj (&iNr, pta, pNr,
                             Typ_ObjGX, sus1->pTab, pNr, UT_DISP_cv);
          if(i1 < 0) {
            TX_Error("SURCIR E_STRIP %d %d %d",i1,ox1->form,ox1->siz);
            return -1;
          }
          for(i1=0; i1<iNr; ++i1) UT3D_box_extend (&pb1, &pb2, &pta[i1]);


        //----------------------------------------------------------------
        } else if(ox1->typ == Typ_SURCIR) {
            // printf("  SURCIR typ=%d form=%d iNr=%d\n",ox1->typ,ox1->form,iNr);
          // get all points of RCIR > pTab
          pNr = ox1->siz;
          pta = (Point*) MEM_alloc_tmp (pNr * sizeof(Point));
          iNr = 0;
          i1 = UT3D_cv_scir__ (&iNr, pta, pNr, ox1);
          if(i1 < 0) {
            TX_Error("SURCIR E_CIR %d %d %d",i1,ox1->form,ox1->siz);
            return -1;
          }
          for(i1=0; i1<iNr; ++i1) UT3D_box_extend (&pb1, &pb2, &pta[i1]);


        //----------------------------------------------------------------
        } else {
          printf("***** DL_ReScale: nicht bearb. SUR-typ %d\n",typ);
        }




      //-----------------------------------------------------------------
      } else {
        printf("**** DL_ReScale: nicht bearb. Objekttyp %d\n",typ);
      }
    }



  //----------------------------------------------------------------
/
  // man muesste nun den obersten und den untersten Z-Punkt entlang der
  // Sichtlinie auf die aktuelle Plane werfen
  pt2.x = uxmax; pt2.y = uymax; pt2.z = uzmax;
  pt1 = GL_GetViewPt (&pt2);
  UT3D_box_extend (&pb1, &pb2,(&pt1);

  pt2.x = uxmin; pt2.y = uymin; pt2.z = uzmin;
  pt1 = GL_GetViewPt (&pt2);
  UT3D_box_extend (&pb1, &pb2,(&pt1);
/

    printf("::::::::::::::::::::::::: ex UT3D_box_model: ::::::::::: \n");
    DEB_dump_obj__ (Typ_PT, &pb1, "pb1");
    DEB_dump_obj__ (Typ_PT, &pb2, "pb2");

  *PB1 = pb1;
  *PB2 = pb2;

  return;

}
*/

//====================== EOF ===========================
