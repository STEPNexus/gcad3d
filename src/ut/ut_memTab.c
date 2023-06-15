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
  ..

-----------------------------------------------------
Modifications:
2015-08-03 MemTab_del - delete all records following <ipos>. RF.
2014-01-27 MemTab_add new mode 4. RF.
2009-01-05 aus ut_mem.c extahiert; UME_mTab -> MemTab RF.
2008-03-21 UME_reall_add,UME_realloc,UME_reall_save: spcOff zu. RF.
2003-05-30 Erste Version aus ut_string.c. RF.

-----------------------------------------------------
*/
#ifdef globTag
void MEMTAB(){}
#endif
/*!
\file  ../ut/ut_memTab.c
\brief fixed-length-records in memory: add,insert,delete, realloc. .. MemTab_ 

- necessary includes:
#include "../ut/ut_memTab.h"              // MemTab

=====================================================
List_functions_start:

MemTab_ini__          init memory-table (malloc)
MemTab_ini_fixed      init memory-table with fixed memoryspace; no realloc, no free.
MemTab_ini_UME        init MemTab inside Memspc (UME_malloc or UME_init)
MemTab_ini_temp       get memspace for rTot records                          INLINE
MemTab_ini_add        init, malloc, add data into MemTab
MemTab_free           free memspace from MemTab_ini_temp                     INLINE
MemTab_add            save/check/reserve in memSpc; malloc/realloc if necessary.
MemTab_sav            save objDat to memSpc; malloc/realloc if necessary.
MemTab_uniq_sav       if data is uniq only: save data with MemTab_add (.. 1, 0);
MemTab_reserve        reserve space to memSpc; malloc/realloc if necessary.
MemTab_check          check free space to memSpc; malloc/realloc if necessary.
MemTab_ins            insert records in MemTab
MemTab_del            delete records in MemTab
MemTab_mod            modify records 
MemTab_load           returns datablock and its nr of records
MemTab_get            retrieve (copy out) records 
MemTab_free           free MemTab-structure.
MemTab_wrf            write MemTab to file
MemTab_rdf            read MemTab from file

MEMTAB__              get data-record from index                     INLINE
MEMTAB_DAT            get data-record complete                       INLINE
MEMTAB_IND            get index (pointer to next free = nr of used)  INLINE
MEMTAB_RESET_IND      reset index (nr of used = 0)                   INLINE
MEMTAB_RMAX           get max. nr of records                         INLINE
MEMTAB_RSIZ           get record size                                INLINE
MEMTAB_RFREE          get nr of free records                         INLINE
MEMTAB_POS            get position of next data-record               INLINE
MEMTAB_IND_POS        get record-index from position                 INLINE
MEMTAB_IS_EMPTY       test if MemTab.data == NULL                    INLINE
MEMTAB_SET_NULL       set MemTab = _MEMTAB_NUL;                      INLINE
MEMTAB_CLEAR          reset  (memTab->rNr = 0;)                      INLINE

MemTab_dump
MemTab_disp_tdyn      display MemTab dynamic        alias GR_tDyn_mtb > GR_set_mtb
MemTab_disp_temp      display MemTab temporary      alias GR_temp_mtb > GR_set_mtb

List_functions_end:
=====================================================

MemTab_int_..         ../ut/ut_memTab1.c      MemTab(Int)
MemTab_IndTab_..      ../ut/ut_memTab1.c      MemTab(IndTab)
ITAB_..               ../ut/itab.c            MemTab(IntTab)
OXMT_..               ../APP/ut_ogxt.c        OgxTab  (MemTab(ObjGX) + Memspc)
  TODO: add ../ut/itab.c  ->  ../ut/ut_memTab1.c als MemTab_IndInt_..
=====================================================

see also:
../ut/ut_umem.c     (MEM_CAN[NOT]_..
INF_MemTab
Examples / Testprogs: see ../APP/Test_MEM.c  - plugin Test_MEM.mak
INF_MEM__

- external functionreferences:
UTI_round_i2b
UTI_round_b2i
MEM_del_nrec
MEM_ins_nrec

*/


// #include "../xa/MS_Def0.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>


#include "../ut/ut_types.h"            // INT_8 - UINT_64
#include "../ut/AP_types.h"
#include "../ut/ut_memTab.h"
#include "../ut/ut_uti.h"              // UTI_round_32up
#include "../ut/ut_cast.h"             // INT__PTR
#include "../ut/ut_types.h"            // UINT_8_MAX
#include "../ut/ut_mem.h"              // MEM_*
#include "../ut/ut_umem.h"                // Memspc - only for MemTab_ini_UME

#include "../xa/xa_msg.h"              // MSG_* ERR_*

MemTab MEMTAB_NUL = _MEMTAB_NUL;




//================================================================================
  int MemTab_ini_fixed (MemTab *memTab, void *data, int dSiz, int rSiz, int typ) {
//================================================================================
/// MemTab_ini_fixed          init memory-table with fixed memoryspace;
///   - memspace expands automaticall; must be freed.
///   - for expandable fixed memspace use MemTab_ini_temp
///   - see INF_MemTab
/// \code
/// INPUT:
///   data    memspc to connect
///   dSiz    size of data in bytes
///   rSiz    recordsize in bytes
///   typ     info; Typ_PT=Point Typ_Int4=int Typ_Float8=double Typ_Int1=char;
///
/// Examples:
//   MemTab(Point) mtpa = _MEMTAB_NUL;
//   Point pta[200];
//   MemTab_ini_fixed (&mtpa, pta, sizeof(pta), sizeof(Point), Typ_PT);
// 
//   
//   MemTab(Point) mtpa = _MEMTAB_NUL;
//   MemTab_ini_fixed (&mtpa, MEM_alloc_tmp (SPC_MAX_STK), SPC_MAX_STK,
//                   sizeof(Point), Typ_PT);
//  
//  
///  int lNrMax=100, mSizb;
///  Line *lna;
///  MemTab(Line) mln = _MEMTAB_NUL;
///  mSizb = lNrMax * sizeof(Line);
///  lna = (Line*) MEM_alloc_tmp (mSizb);
///  MemTab_ini_fixed (&mln, lna, mSizb, sizeof(Line), Typ_LN);

/// \endcode

  // printf("MemTab_ini__ rSiz=%d typ=%d incSiz=%d\n",rSiz,typ,incSiz);


  memTab->data   = data;
  memTab->tSiz   = dSiz;
  memTab->rMax   = dSiz / rSiz;

  memTab->rNr    = 0;
  memTab->rSiz   = rSiz;
  // memTab->incSiz = UINT_8_MAX;
  memTab->typ    = typ;
  memTab->spcTyp = MEMTYP_STACK_EXPND;     // fixed; CANNOT-reallocate


  // printf("ex MemTab_ini__ rSiz=%d incSiz=%d\n",memTab->rSiz,memTab->incSiz);

  return 0;

}


//=========================================================================
  int MemTab_ini__ (void *mtbv, int rSiz, int typ, int incSiz) {
//=========================================================================
// MemTab_ini__           init memory-table; if already in use: clear.
//   - MUST BE INITIALIZED WITH _MEMTAB_NUL
//   - prepare for permanent memoryspace (malloc) ; must be freed with MemTab_free
//   - get space (malloc) with MemTab_add; does expand automatically;
// use MemTab for a list of records;
// use Memspc for a different types of records in the same memoryblock.
// 
// INPUT:
//   rSiz    recordsize in bytes
//   typ     info; Typ_PT=Point Typ_Int4=int Typ_Float8=double Typ_Int1=char;
//   incSiz  nr of records to increment memTab;
//           if Memspc is too small: add UTI_round_b2i(incSiz * rSiz)
//           0 -> 0; 1 -> 2; 2 -> 4; 3 -> 8; 4 -> 16; 5 -> 32; 8=256; 10=1024 ..
//           10: UTI_round_b2i(10) = 1024 records to add when reallocating
//
// Examples:
//   MemTab(int) mtbIa1 = _MEMTAB_NUL;
//   MemTab_ini__ (&mtbIa1, sizeof(int), Typ_Int4, 1000);
//   MemTab_add  (..);   // add records; reallocates if necessary
//   MemTab_free (&mtbIa1);
//
//

// SEE TODO ON TOP ..

  MemTab *memTab = mtbv;


  // printf("MemTab_ini__ rSiz=%d typ=%d incSiz=%d\n",rSiz,typ,incSiz);

  if(memTab->data == NULL) {
    memTab->rMax   = 0;
    memTab->spcTyp = MEMTYP_ALLOC__;  // else already tempspace ?
  }
 
  memTab->rNr    = 0;
  memTab->rSiz   = rSiz;
  memTab->incSiz = UTI_round_i2b(incSiz);
  memTab->typ    = typ;
  //                       keep tSiz = 0


    // printf("ex-MemTab_ini__ rSiz=%d incSiz=%d\n",memTab->rSiz,memTab->incSiz);

  return 0;

}


//=========================================================================
  int MemTab_ini_add (void *pmtb, int typ, int rSiz, int rNr, void *data) {
//=========================================================================
// MemTab_ini_add    init, malloc, add data into MemTab


  long    l1;

  // printf("MemTab_ini_add \n");

  MemTab_ini__ (pmtb, rSiz, typ, rNr + 10);

  MemTab_add (pmtb, &l1, data, rNr, 0);

  return 0;

}


//===============================================================================
  int MemTab_ini_UME (Memspc *mSpc, MemTab **mtb, int rSiz, int typ, int rNr) {
//===============================================================================
// MemTab_ini_UME              make MemTab indside Memspc
//   MemTab CANNOT REALLOCATE.
// Input
//   mSpc       Memspc
//   rSiz       recordsize in bytes
//   typ        info; Typ_PT=Point Typ_Int4=int Typ_Float8=double Typ_Int1=char;
//   rNr        nr of records to occupy; -1 = complete space of mSpc;
// Output
//   mtb        pointer to MemTab mtb inside Memspc mSpc

// MemTab_add


  int       sizTot, sizDat, rMax;
  MemTab    mtx = _MEMTAB_NUL;


  // printf("MemTab_ini_UME rSiz=%d typ=%d rNr=%d\n",rSiz,typ,rNr);

  sizTot = UME_ck_free (mSpc) - sizeof(MemTab);

  if(rNr > 0) {
    sizDat = rSiz * rNr;
    if(sizDat > sizTot)
      return MSG_ERROR (ERR_EOM, "MemTab_ini_UME - not enough space");
  } else {
    rNr = sizTot / rSiz;
    if(rNr >sizTot) --rNr;
    sizDat = rSiz * rNr;
  }
    // printf(" sizTot=%d sizDat=%d rNr=%d\n",sizTot,sizDat,rNr);


  mtx.typ  = typ;
  mtx.rMax = rNr;
  mtx.rSiz = rSiz;
  mtx.tSiz = sizDat;
  // set spcTyp = MEMTYP_STACK__ - realloc - NO,    malloc - NO;
  mtx.spcTyp = MEMTYP_STACK__;
  mtx.data   = (char*)mSpc->next + sizeof(MemTab);
    // MemTab_dump (&mtx, "MemTab-mtx");


  // copy out pointer to MemTab mtx -> mSpc
  *mtb = mSpc->next;

  // copy MemTab-header into mSpc
  memcpy (mSpc->next, &mtx, sizeof(MemTab));

  // occupy space in Memspc mSpc
  mSpc->next = (char*)mtx.data + sizDat;
  // mSpc->next += sizeof(MemTab) + sizDat;


    // UME_dump (mSpc, "Memspc-mSpc");
    // MemTab_dump (*mtb, "MemTab-mtb");

  return 0;

}


//=========================================================================
  int MemTab_add (void *mtbv, long *spcOff,
                  void* objDat, int recNr, int mode) {
//=========================================================================
/// \code
/// MemTab_add          save/check/reserve in memSpc; malloc/realloc if necessary.
/// struct must be initialized with MemTab_ini__; free it with MemTab_free.
/// Fixed Recordsizes only !
/// 
/// Input:
///   memTab        where to store data; must have type according to objDat.
///   objDat        data to be saved in memSpc (NULL if mode=1 or 2)
///   recNr         nr of records to be allocated / saved
///   mode          0=save (get space, save data, incr. counter)
///                 1=reserve (get space and add <recNr> to internal counter)
///                 2=check (now MT has at least <recNr> free records)
///                 4=save; but create <recNr> copies of record <objDat>
/// Output:
///   mtbv->rNr   nr of records already in use (including objDat)
///   spcOff        displacement-offset; 0: no reallocate.
/// RetCode 
///   0             OK
///   1             OK after reallocate
///  -1             outOfMemory-Error.
///  -2             expand memory not allowed
/// 
/// mtbv:
///   mtbv->rMax  nr of records already allocated
///   mtbv->rNr   nr of records already in use
///   mtbv->rSiz  recordsize in bytes
/// 
/// Ablauf:
///   if (recNr > 0)  get space for <recNr> records.
///   else            get space for <recNrFree> records.
///   if (objDat == NULL)  do not save, do malloc/realloc.
///   recNrFree =  UTI_round_b2i(mtbv->incSiz)
///
/// Examples:
/// // typedef_MemTab(Point);
/// MemTab(Point)    pa2D = _MEMTAB_NUL;
/// ips = MEMTAB_IND (&ts1->pa2D);                // get next free position
/// irc = MemTab_add (&pa2D, &ld, NULL, ptNr, 1); // reserve pNr points in pa2D
/// pa = MEMTAB_DAT (&ts1->pa2D);                 // get datablock of points
/// irc = PRCV_get_dbo_1 (&pa[ips]);              // get points & save in pa2D
/// \endcode


  int    i1, irc=0;
  long   newRecNrMax, iAddr, iAddNr, tSiz;
  void   *oldPos, *newPos;
  char   *oPos, *cp1;
  MemTab *memTab = mtbv;



  // printf("MemTab_add max=%d used=%d siz=%d nr=%d\n",
          // memTab->rMax, memTab->rNr, memTab->rSiz, recNr);
  // DEB_dump_obj__ (Typ_MemTab, memTab, " _add-in");


  *spcOff = 0;

  //----------------------------------------------------------------
  // test init
  if(memTab->rSiz < 1) {
    // 0 = uninitialized
    TX_Error("MemTab_sav E001 (uninitialized)");
    return -2;
  }

  if(memTab->data == NULL) {
    oldPos       = NULL;
    memTab->rNr  = 0;             // used records 
    memTab->rMax = 0;             // actual size
  } else {
    oldPos = memTab->data;
  }


  //----------------------------------------------------------------
  // test if enough space ..
  iAddr = memTab->rNr + recNr;
  // if(MEM_CAN_ALLOC(memTab->spcTyp)) iAddr += 1;  // can realloc: keep 1 records free
  if(iAddr <= memTab->rMax) goto L_save;



  //================================================================
  // not enough space ..
    // DEB_dump_obj__ (Typ_MemTab, memTab, "\nMemTab_add mode=%d iAddr=%ld",
                    // mode,iAddr);
    // printf("\nMemTab_add-rMax=%d rNr=%d recNr=%d incSiz=%d iAddr=%ld\n",
           // memTab->rMax, memTab->rNr, memTab->incSiz, recNr, iAddr);
    // AP_debug__  ("MemTab_add-1");


  // compute newRecNrMax
  if(memTab->incSiz) iAddNr = UTI_round_b2i (memTab->incSiz);
  else               iAddNr = UTI_round_32up (iAddr);           //(recNr);

  newRecNrMax = memTab->rMax;
  while(newRecNrMax < iAddr) newRecNrMax += iAddNr;
    // printf(" newRecNrMax=%ld iAddNr=%ld iAddr=%ld\n",newRecNrMax,iAddNr,iAddr);


  // test if memspc can be reallocated
  if(MEM_CAN_ALLOC(memTab->spcTyp)) goto L_reall;

  // test if memspc can be copied
  if(MEM_CAN_EXPND(memTab->spcTyp)) goto L_copy;

  // not expandable; error -2             expand memory not allowed
  TX_Error("MemTab_add expand memory not allowed");
  return -2;



  //----------------------------------------------------------------
  L_reall:
    // printf(" L_reall:\n");
  // realloc memspace
  tSiz = newRecNrMax * memTab->rSiz;
  if(memTab->data) memTab->data = realloc (memTab->data, tSiz);
  else             memTab->data = (void*) malloc (tSiz);
  if(memTab->data == NULL) {
    TX_Error("MemTab_add E1 EOM");
    return -1;
  }

  memTab->rMax = newRecNrMax;
  memTab->tSiz = tSiz;

  // compute displacement-offset
  if(oldPos) *spcOff = (char*)memTab->data - (char*)oldPos;
  goto L_save;



  //----------------------------------------------------------------
  L_copy:
  // malloc new memspace, copy memspace, do not free  source.
    // printf(" L_copy:\n");
  tSiz = newRecNrMax * memTab->rSiz;
  newPos = malloc (tSiz);
  if(newPos == NULL) {
    TX_Error("MemTab_add E2 EOM");
    return -1;
  }

  // copy memspace
  if(memTab->rNr)
    memcpy(newPos, oldPos, memTab->rNr * memTab->rSiz);
  
  memTab->data = newPos;
  memTab->rMax = newRecNrMax;
  memTab->tSiz = tSiz;

  // compute displacement-offset
  if(oldPos) *spcOff = (char*)memTab->data - (char*)oldPos;
  // goto L_save;



  //================================================================
  L_save:
  // if((objDat)&&(recNr > 0)) {
  // mode 0=save, 1=reserve, 2=check
  if(mode == 0) {
    oPos = (char*)memTab->data;
    memcpy(&oPos[memTab->rNr * memTab->rSiz], objDat, recNr * memTab->rSiz);

  } else if(mode == 4) {
    // create <recNr> copies of record <objDat>      // 2014-01-27
      // printf(" m4: rNr=%d rSiz=%d\n",memTab->rNr,memTab->rSiz);
    oPos = (char*)memTab->data;
    cp1 = &oPos[memTab->rNr * memTab->rSiz];
    for(i1=0; i1<recNr; ++i1) {
      memcpy(cp1, objDat, memTab->rSiz);
      cp1 += memTab->rSiz;
    }
  }

  if(mode != 2) {
    // for 0|1|4
    memTab->rNr += recNr;
  }

  //----------------------------------------------------------------
  L_exit:

    // printf("ex-UME_alloc_add max=%d used=%d\n",memTab->rMax,memTab->rNr);
    // DEB_dump_obj__ (Typ_MemTab, memTab, "ex-UME_alloc_add");

  return irc;

}


//=========================================================================
  int MemTab_sav (MemTab *memTab, long *spcOff, void* objDat, int recNr) {
//=========================================================================
// MemTab_sav          save objDat to memSpc; malloc/realloc if necessary.

  return MemTab_add (memTab, spcOff, objDat, recNr, 0);

}


//====================================================================
    int MemTab_uniq_sav (MemTab *memTab, long *spcOff, void* objDat) {
//====================================================================
/// \code
/// MemTab_uniq_add     if data is uniq only: save data with MemTab_add (.. 1, 0);
/// API see MemTab_add
/// RetCode
///   2        objDat already exists
/// \endcode


  int     i1, ie;
  long    lsiz;
  char    *mdat;


  // printf("MemTab_uniq_sav %d %d\n",memTab->rNr,memTab->rSiz);


  ie = memTab->rNr;      // nr of records
  lsiz = memTab->rSiz;   // recordSize
  mdat =memTab->data;    // start of data

  for(i1=0; i1<ie; ++i1) {
    if(! memcmp(mdat, objDat, lsiz)) return 2;  // objDat already exists
    mdat += lsiz;
  }


  return MemTab_add (memTab, spcOff, objDat, 1, 0);

}


//================================================================
  int MemTab_reserve (MemTab *memTab, long *spcOff, int recNr) {
//================================================================
// MemTab_reserve      reserve space to memSpc; malloc/realloc if necessary.
// updates data-pointer;
// get first free position before with MEMTAB_IND

  return MemTab_add (memTab, spcOff, NULL, recNr, 1);

}

//================================================================
  int MemTab_check (MemTab *memTab, long *spcOff, int recNr) {
//================================================================
// MemTab_check        check free space to memSpc; malloc/realloc if necessary.
// does not update data-pointer


  // printf("MemTab_check recNr=%d\n",recNr);

  return MemTab_add (memTab, spcOff, NULL, recNr, 2);

}


//================================================================
  int  MemTab_load (void **oTab, int *iNr, MemTab *mt) {
//================================================================
/// \code
/// MemTab_load            returns datablock and its nr of records
/// use:
///   MemTab_load ((void**)&oTab, &iNr, (MemTab*)&delTab);
/// see also MemTab_get
/// \endcode


  *iNr  = mt->rNr;
  *oTab = mt->data;

  return 0;

}


//================================================================
  int MemTab_get (void* objDat, MemTab *mt, int ipos, int recNr) {
//================================================================
/// \code
/// MemTab_get          retrieve (copy out) records 
///   get <recNr> records starting with record <ipos>
/// 
/// Input:
///   ipos       1. record to replace in mt->data
///   recNr      nr of records to overwrite 
/// Output:
///   objDat     data out of memSpc
/// RetCodes:
///    -1        not yet allocated (ipos+recNr > mt->rNr)
/// \endcode

  char   *oPos;

  if(mt->rNr < (ipos + recNr)) return -1;

  oPos = (char*)mt->data;

  memcpy(objDat, &oPos[ipos * mt->rSiz], recNr * mt->rSiz);

  return 0;

}


//================================================================
  int MemTab_mod (MemTab *mt, void* objDat, int ipos, int recNr) {
//================================================================
/// \code
/// MemTab_mod               modify records 
/// modify <recNr> records starting with record <ipos>
/// 
/// Input:
///   objDat     data to be saved in memSpc
///   ipos       1. record to replace in mt->data
///   recNr      nr of records to overwrite 
/// RetCodes:
///    -1        not yet allocated (ipos+recNr > mt->rNr)
/// \endcode

  char   *oPos;

  if(mt->rNr < (ipos + recNr)) return -1;

  oPos = (char*)mt->data;

  memcpy(&oPos[ipos * mt->rSiz], objDat, recNr * mt->rSiz);

  return 0;

}


//================================================================
  int MemTab_del (MemTab *mt, int ipos, int delRecNr) {
//================================================================
/// \code
/// MemTab_del               delete records in MemTab
///   delete <delRecNr> records starting with record <ipos>
/// Input:
///   ipos       1. record to delete in mt->data
///   delRecNr   nr of records to delete 
///              -1 delete all records following <ipos> and including <ipos>
/// \endcode


  if(delRecNr < 0) {
    // delete all records including <ipos>
    mt->rNr = ipos;
    return 0;
  }

  return MEM_del_nrec (&mt->rNr, mt->data, ipos, delRecNr, mt->rSiz);

}


//================================================================
  int MemTab_ins (MemTab *mt, long *spcOff,
                    void* objDat, int ipos, int insRecNr) {
//================================================================
/// \code
/// MemTab_ins               insert records in MemTab
///   insert <insRecNr> records before record <ipos>
/// \endcode

  int      irc;


  // check space
  irc = MemTab_add (mt, spcOff, NULL, insRecNr, 2);
  if(irc < 0) return irc;

  // insert
  MEM_ins_nrec (&mt->rNr, mt->data, ipos, objDat, insRecNr, mt->rSiz);


  return 0;

}


/*
//================================================================
  int MEMTAB_CLEAR (MemTab *memTab) {
//================================================================
/// MEMTAB_CLEAR        reset  (memTab->rNr = 0;)

  if(memTab->data) {
    memTab->rNr = 0;
  }

  return 0;

}
*/


//================================================================
  int MemTab_free (void* mtbi) {                  // (MemTab *memTab)
//================================================================
/// MemTab_free        free MemTab-structure.

  MemTab *memTab;

  memTab = mtbi;     // casting ..


  // MemTab_dump (mtbi, "MemTab_free");
  // printf(" MEM_MUST_FREE = %d\n",MEM_MUST_FREE(memTab->spcTyp));


  if(MEM_MUST_FREE(memTab->spcTyp)) {
      // printf("MemTab_free-free\n");
    if(memTab->data) free (memTab->data);
  }
    memTab->data = NULL;
    memTab->rMax = 0;             // actual size
    memTab->rNr  = 0;
    memTab->tSiz = 0;
 

    // MemTab_dump (mtbi, "ex-MemTab_free");

  return 0;

}


//================================================================
  int MemTab_wrf (FILE *fp1, MemTab *mt) {
//================================================================
// write MemTab to file

  printf("MemTab_wrf rSiz=%d rNr=%d\n",mt->rSiz, mt->rNr);


  fwrite(mt, sizeof(MemTab), 1, fp1);

  if(mt->rNr > 0) fwrite(mt->data, mt->rSiz, mt->rNr, fp1);

  return 0;
}


//================================================================
  int MemTab_rdf (FILE *fp1, MemTab *mt1) {
//================================================================
// read MemTab from file
// Input:
//   mt1    new Memtab - load data from open file into mt1
//   mt2    old Memtab - has invalid data and rMax
// Output:
//   mt1

  int     ii, rNr;
  long    l1;


  // printf("MemTab_rdf rNr=%d rMax=%d rSiz=%d\n",
             // mt1->rNr,mt1->rMax,mt1->rSiz);

  // load mt1 = old-mt; has invalid data and rMax
  fread(mt1, sizeof(MemTab), 1, fp1);
     // printf("    -mt-in rNr=%d rMax=%d rSiz=%d\n",
             // mt1->rNr, mt1->rMax, mt1->rSiz);

  // clear Data, rMax, tSiz
  mt1->data   = NULL;
  mt1->rMax   = 0;
  // mt1->tSiz   = 0;


  // skip reading data if no data exists
  if(mt1->rNr <= 0) goto L_exit;

  // reserve space
  MemTab_add (mt1, &l1, NULL, mt1->rNr, 1);
    // printf(" MemTab_rdf-add %d records\n",mt1->rNr);
    // DEB_dump_obj__ (Typ_MemTab, mt1, " -mt-add");

  // get data
  fread(mt1->data, mt1->rSiz, mt1->rNr, fp1);

  L_exit:
    // DEB_dump_obj__ (Typ_MemTab, mt1, "ex-MemTab_rdf");
    // MemTab_dump (mt1, "ex-MemTab_rdf");
  return 0;
}


//================================================================
  int MemTab_dump (MemTab *memTab, char *info) {
//================================================================
// see MemTab_disp_tdyn

  int   irc, i1, ityp, isiz, *ia;
  long  *la;
  char  *data;
  void  *vp;


  printf("============ MemTab_dump %s =============\n", info);
    printf(" rMax=%d rNr=%d rSiz=%d\n",memTab->rMax,memTab->rNr,memTab->rSiz);


  if(!memTab)          { printf(".. undefined ..\n");     return -1;}
  if(memTab->rMax < 1) { printf(".. uninitialized ..\n"); return -1;}

  

  DEB_dump_obj__ (Typ_MemTab, memTab, "MemTab_dump");

  if(memTab->rNr < 1)  { printf(".. empty ..\n"); return -1;}

  ityp = memTab->typ;

  if(!ityp) {printf(".. objTyp undef ..\n"); return -1;}
  if(ityp == Typ_unknown) {printf(".. objTyp unknown ..\n"); return -1;}

  isiz = UTO_siz_stru (ityp);
    printf(" ityp=%d isiz=%d rNr=%d\n",ityp,isiz,memTab->rNr);



  //----------------------------------------------------------------
  // Typ_Int1
  if(ityp != Typ_Int1) goto L_I4;

  data = MEMTAB_DAT (memTab);
  for(i1=0; i1<memTab->rNr; ++i1) {
    vp = PTR_INT((int)data[i1]);
    irc = DEB_dump_obj__ (ityp, vp, "[%d]",i1);
    if(irc < 0) break;
  }
  return 0;




  //----------------------------------------------------------------
  L_I4:
  // Typ_Int4
  if(ityp != Typ_Int4) goto L_I8;

  ia = MEMTAB_DAT (memTab);
  for(i1=0; i1<memTab->rNr; ++i1) {
    vp = PTR_INT(ia[i1]);
      // printf(" i4[%d]=%ld\n",i1,(long)vp);
    irc = DEB_dump_obj__ (ityp, vp, "[%d]",i1);
    if(irc < 0) break;
  }
  return 0;



  //----------------------------------------------------------------
  L_I8:
  // Typ_Int8
  if(ityp != Typ_Int8) goto L_1;

  la = MEMTAB_DAT (memTab);
  for(i1=0; i1<memTab->rNr; ++i1) {
    vp = PTR_LONG(la[i1]);
      // printf(" i4[%d]=%ld\n",i1,(long)vp);
    irc = DEB_dump_obj__ (ityp, vp, "[%d]",i1);
    if(irc < 0) break;
  }
  return 0;


  //----------------------------------------------------------------
  L_1:
  // Typ_PT, Typ_IndTab ..
  data = (char*)memTab->data;

  for(i1=0; i1<memTab->rNr; ++i1) {
      // printf(" data=%d\n",*((int*)data));
    irc = DEB_dump_obj__ (ityp, data, "[%d]",i1);
    if(irc < 0) break;
    data += isiz;
  }

  printf("\n");






  return 0;

}


//======================== EOF ======================================*/
