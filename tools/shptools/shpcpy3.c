/*********************************************************************
*
*  $Id: shpcpy3.c,v 1.1 2006/09/28 15:27:29 woodbri Exp $
*
*  shpcpy
*  Copyright 2001, Stephen Woodbridge, All rights Reserved
*  Redistribition without written permission strictly prohibited
*
**********************************************************************
*
*  shpcpy.c
*
*  $Log: shpcpy3.c,v $
*  Revision 1.1  2006/09/28 15:27:29  woodbri
*  Adding shpcpy2.c shpcpy3.c shpsplit.c
*
*  Revision 1.2  2004/09/16 20:53:32  swoodbridge
*  Changes to support the fact that debian libshp-dev places the shapefile in a
*  different location than the source vendor.
*
*  Revision 1.1.1.1  2004/03/05 19:00:28  swoodbridge
*  Initial import of imaptools.com software LICENSED to where2getit.com.
*  For more information or questions regarding the license contact:
*  Stephen Woodbridge
*  978-251-4502
*  978-853-8336 (c)
*  woodbri@imaptools.com
*  woodbri@swoodbridge.com
*
*  Revision 1.1  2002/03/18 16:23:52  woodbri
*  Initial revision
*
*
*  2002-02-12 sew, created
*
**********************************************************************
*
*  Utility to copy objects from one shapefile to a new shapefile
*  The <string> is prepended to field LOCATION
*
**********************************************************************/

#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBIAN_LIBSHP
#include <shapefil.h>
#else
#include <libshp/shapefil.h>
#endif


#ifdef DMALLOC
#include "dmalloc.h"
#endif

// Debug print out
#define DBGOUT(s) (s)
//#define DBGOUT(s)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

extern int DEBUG;
int DEBUG  = 0;


void Usage()
{
  printf("shpcpy <src base name> <dest base name> <string> [<list of objects>]\n");
  exit(0);
}

int main( int argc, char ** argv )
{
  DBFHandle     dIN, dOUT;
  SHPHandle     sIN, sOUT;
  SHPObject    *sobj;
  DBFFieldType  fType;
  char         *fin;
  char         *fout;
  char         *pString;
  char          fldname[12];
  char          buff[256];
  int           nEnt, sType, fw, fd;
  int           i, j, rec, n;
  int           iLOC = -1;
  double        zX[2] = {0.0, 0.0};
  double        zY[2] = {0.0, 0.0};
  double        sMin[4];
  double        sMax[4];

  if (argc < 4) Usage();

  fin  = argv[1];
  fout = argv[2];
  pString = argv[3];
  argc -= 4;
  argv += 4;

  if (!(dIN  = DBFOpen(fin, "rb"))) {
    printf("Failed to open dbf %s for read\n", fin);
    exit(1);
  }

  if (!(sIN  = SHPOpen(fin, "rb"))) {
    printf("Failed to open shp %s for read\n", fin);
    exit(1);
  }

  SHPGetInfo(sIN, &nEnt, &sType, sMin, sMax);

  if (!(dOUT = DBFCreate(fout))) {
    printf("Failed to create dbf %s\n", fout);
    exit(1);
  }

  if (!(sOUT = SHPCreate(fout, sType))) {
    printf("Failed to create shp %s\n", fout);
    exit(1);
  }

  for (i=0; i<DBFGetFieldCount(dIN); i++) {
    fType = DBFGetFieldInfo(dIN, i, fldname, &fw, &fd);
    DBFAddField(dOUT, fldname, fType, fw, fd);
    if (!strcasecmp(fldname, "LOCATION")) {
      iLOC = i;
    }
  }

  if (argc > 0)
    for (i=0; i<argc; i++) {
      rec = strtol(argv[i], NULL, 10);
      if (rec >= nEnt) continue;
      sobj = SHPReadObject(sIN, rec);
      if (sobj->nSHPType < 1 || sobj->nVertices < 1) { 
        SHPDestroyObject(sobj);
#ifdef NULLTOPOINT
        sobj = SHPCreateSimpleObject(sType, 1, zX, zY, NULL);
        sobj->dfXMin = sMin[0];
        sobj->dfYMin = sMin[1];
        sobj->dfZMin = sMin[2];
        sobj->dfMMin = sMin[3];
        sobj->dfXMax = sMax[0];
        sobj->dfYMax = sMax[1];
        sobj->dfZMax = sMax[2];
        sobj->dfMMax = sMax[3];
#else
        continue;
#endif
      }
      n = SHPWriteObject(sOUT, -1, sobj);
      SHPDestroyObject(sobj);
      for (j=0; j<DBFGetFieldCount(dIN); j++) {
        fType = DBFGetFieldInfo(dIN, j, NULL, NULL, NULL);
        switch (fType) {
          case FTString:
            if (j == iLOC) {
              strcpy(buff, pString);
	      strcat(buff, DBFReadStringAttribute( dIN, rec, j));
              DBFWriteStringAttribute( dOUT, n, j, buff);
            } else {
              DBFWriteStringAttribute( dOUT, n, j, DBFReadStringAttribute( dIN, rec, j));
	    }
            break;
          case FTInteger:
            DBFWriteIntegerAttribute(dOUT, n, j, DBFReadIntegerAttribute(dIN, rec, j));
            break;
          case FTDouble:
            DBFWriteDoubleAttribute( dOUT, n, j, DBFReadDoubleAttribute( dIN, rec, j));
            break;
        }
      }
    }
  else
    for (i=0; i<nEnt; i++) {
      rec = i;
      sobj = SHPReadObject(sIN, rec);
      if (sobj->nSHPType < 1 || sobj->nVertices < 1) {
        SHPDestroyObject(sobj);
#ifdef NULLTOPOINT
        sobj = SHPCreateSimpleObject(sType, 1, zX, zY, NULL);
        sobj->dfXMin = sMin[0];
        sobj->dfYMin = sMin[1];
        sobj->dfZMin = sMin[2];
        sobj->dfMMin = sMin[3];
        sobj->dfXMax = sMax[0];
        sobj->dfYMax = sMax[1];
        sobj->dfZMax = sMax[2];
        sobj->dfMMax = sMax[3];
#else
        continue;
#endif
      }
      n = SHPWriteObject(sOUT, -1, sobj);
      SHPDestroyObject(sobj);
      for (j=0; j<DBFGetFieldCount(dIN); j++) {
        fType = DBFGetFieldInfo(dIN, j, NULL, NULL, NULL);
        switch (fType) {
          case FTString:
            if (j == iLOC) {
              strcpy(buff, pString);
	      strcat(buff, DBFReadStringAttribute( dIN, rec, j));
              DBFWriteStringAttribute( dOUT, n, j, buff);
            } else {
              DBFWriteStringAttribute( dOUT, n, j, DBFReadStringAttribute( dIN, rec, j));
	    }
            break;
          case FTInteger:
            DBFWriteIntegerAttribute(dOUT, n, j, DBFReadIntegerAttribute(dIN, rec, j));
            break;
          case FTDouble:
            DBFWriteDoubleAttribute( dOUT, n, j, DBFReadDoubleAttribute( dIN, rec, j));
            break;
        }
      }
    }

  for (j=0; j<4; j++) {
    sOUT->adBoundsMin[j] = sMin[j];
    sOUT->adBoundsMax[j] = sMax[j];
  }

  DBFClose(dIN);
  DBFClose(dOUT);
  SHPClose(sIN);
  SHPClose(sOUT);
}


