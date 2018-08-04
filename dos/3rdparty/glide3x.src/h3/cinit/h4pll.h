/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   h4pll.h
**
** Description: Avenger PLL table and values.
**
** $History: h4pll.h $
** 
** *****************  Version 2  *****************
** User: Michael      Date: 1/08/99    Time: 1:51p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 1  *****************
** User: Andrew       Date: 11/18/98   Time: 12:27p
** Created in $/devel/h3/Win95/dx/minivdd
** Pll Table for Avenger
** 
** *****************  Version 8  *****************
** User: Pault        Date: 11/16/98   Time: 3:30p
** Updated in $/devel/h4/cinit
** This version of the table has m=k=1 and is based on the same
** calculations we have been using in the lab. This will not work for
** AvengerOEM.
** 
** *****************  Version 7  *****************
** User: Pault        Date: 11/15/98   Time: 4:17p
** Updated in $/devel/h4/cinit
** Added missing pllTable definition at the bottom of the file.
** 
** *****************  Version 6  *****************
** User: Pault        Date: 11/15/98   Time: 2:00p
** Updated in $/devel/h4/cinit
** Temporary table to work with Avenger. This table has k=1. But m values
** are not wedged to 24 below 141 MHz, so this table with not work with
** AvengerOEM.
** 
** *****************  Version 5  *****************
** User: Pault        Date: 11/05/98   Time: 2:07p
** Updated in $/devel/h4/cinit
** The table now goes up to 220 MHz for Avenger. Also, all of the entries
** up through 141 MHz have the M factor set at 24 so the entries can be
** used with Avenger OEM.
** 
** *****************  Version 3  *****************
** User: Artg         Date: 8/27/98    Time: 11:08a
** Updated in $/devel/h3/Win95/dx/minivdd
** added guard ifdef for redundant plltable define.
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

//
// generated by gen_plltable_at.pl at Sun Nov 15 14:16:18 1998
//

#define MIN_PLL_FREQ 30
#define MAX_PLL_FREQ 120
#define MAX_H4_OEM_PLL_FREQ 141
#define MAX_H4_PLL_FREQ 220

FxU32 _h4pllTable[] = {
//	   pllCtrl        n   m   k actual(MHz)
//	   -------      --- --- --- -----------
	0x00000B05,  //  11   1   1 31.022723
	0x00000B05,  //  11   1   1 31.022723
	0x00000B05,  //  11   1   1 31.022723
	0x00000C05,  //  12   1   1 33.409087
	0x00000C05,  //  12   1   1 33.409087
	0x00000D05,  //  13   1   1 35.795450
	0x00000D05,  //  13   1   1 35.795450
	0x00000E05,  //  14   1   1 38.181813
	0x00000E05,  //  14   1   1 38.181813
	0x00000E05,  //  14   1   1 38.181813
	0x00000F05,  //  15   1   1 40.568177
	0x00000F05,  //  15   1   1 40.568177
	0x00001005,  //  16   1   1 42.954540
	0x00001005,  //  16   1   1 42.954540
	0x00001005,  //  16   1   1 42.954540
	0x00001105,  //  17   1   1 45.340903
	0x00001105,  //  17   1   1 45.340903
	0x00001205,  //  18   1   1 47.727267
	0x00001205,  //  18   1   1 47.727267
	0x00001305,  //  19   1   1 50.113630
	0x00001305,  //  19   1   1 50.113630
	0x00001305,  //  19   1   1 50.113630
	0x00001405,  //  20   1   1 52.499993
	0x00001405,  //  20   1   1 52.499993
	0x00001505,  //  21   1   1 54.886357
	0x00001505,  //  21   1   1 54.886357
	0x00001505,  //  21   1   1 54.886357
	0x00001605,  //  22   1   1 57.272720
	0x00001605,  //  22   1   1 57.272720
	0x00001705,  //  23   1   1 59.659083
	0x00001705,  //  23   1   1 59.659083
	0x00001805,  //  24   1   1 62.045447
	0x00001805,  //  24   1   1 62.045447
	0x00001805,  //  24   1   1 62.045447
	0x00001905,  //  25   1   1 64.431810
	0x00001905,  //  25   1   1 64.431810
	0x00001A05,  //  26   1   1 66.818173
	0x00001A05,  //  26   1   1 66.818173
	0x00001A05,  //  26   1   1 66.818173
	0x00001B05,  //  27   1   1 69.204537
	0x00001B05,  //  27   1   1 69.204537
	0x00001C05,  //  28   1   1 71.590900
	0x00001C05,  //  28   1   1 71.590900
	0x00001D05,  //  29   1   1 73.977263
	0x00001D05,  //  29   1   1 73.977263
	0x00001D05,  //  29   1   1 73.977263
	0x00001E05,  //  30   1   1 76.363627
	0x00001E05,  //  30   1   1 76.363627
	0x00001F05,  //  31   1   1 78.749990
	0x00001F05,  //  31   1   1 78.749990
	0x00002005,  //  32   1   1 81.136353
	0x00002005,  //  32   1   1 81.136353
	0x00002005,  //  32   1   1 81.136353
	0x00002105,  //  33   1   1 83.522717
	0x00002105,  //  33   1   1 83.522717
	0x00002205,  //  34   1   1 85.909080
	0x00002205,  //  34   1   1 85.909080
	0x00002205,  //  34   1   1 85.909080
	0x00002305,  //  35   1   1 88.295443
	0x00002305,  //  35   1   1 88.295443
	0x00002405,  //  36   1   1 90.681807
	0x00002405,  //  36   1   1 90.681807
	0x00002505,  //  37   1   1 93.068170
	0x00002505,  //  37   1   1 93.068170
	0x00002505,  //  37   1   1 93.068170
	0x00002605,  //  38   1   1 95.454533
	0x00002605,  //  38   1   1 95.454533
	0x00002705,  //  39   1   1 97.840897
	0x00002705,  //  39   1   1 97.840897
	0x00002705,  //  39   1   1 97.840897
	0x00002805,  //  40   1   1 100.227260
	0x00002805,  //  40   1   1 100.227260
	0x00002905,  //  41   1   1 102.613623
	0x00002905,  //  41   1   1 102.613623
	0x00002A05,  //  42   1   1 104.999987
	0x00002A05,  //  42   1   1 104.999987
	0x00002A05,  //  42   1   1 104.999987
	0x00002B05,  //  43   1   1 107.386350
	0x00002B05,  //  43   1   1 107.386350
	0x00002C05,  //  44   1   1 109.772713
	0x00002C05,  //  44   1   1 109.772713
	0x00002D05,  //  45   1   1 112.159077
	0x00002D05,  //  45   1   1 112.159077
	0x00002D05,  //  45   1   1 112.159077
	0x00002E05,  //  46   1   1 114.545440
	0x00002E05,  //  46   1   1 114.545440
	0x00002F05,  //  47   1   1 116.931803
	0x00002F05,  //  47   1   1 116.931803
	0x00002F05,  //  47   1   1 116.931803
	0x00003005,  //  48   1   1 119.318167
	0x00003005,  //  48   1   1 119.318167
	0x00003105,  //  49   1   1 121.704530
	0x00003105,  //  49   1   1 121.704530
	0x00003205,  //  50   1   1 124.090893
	0x00003205,  //  50   1   1 124.090893
	0x00003205,  //  50   1   1 124.090893
	0x00003305,  //  51   1   1 126.477257
	0x00003305,  //  51   1   1 126.477257
	0x00003405,  //  52   1   1 128.863620
	0x00003405,  //  52   1   1 128.863620
	0x00003405,  //  52   1   1 128.863620
	0x00003505,  //  53   1   1 131.249983
	0x00003505,  //  53   1   1 131.249983
	0x00003605,  //  54   1   1 133.636347
	0x00003605,  //  54   1   1 133.636347
	0x00003705,  //  55   1   1 136.022710
	0x00003705,  //  55   1   1 136.022710
	0x00003705,  //  55   1   1 136.022710
	0x00003805,  //  56   1   1 138.409073
	0x00003805,  //  56   1   1 138.409073
	0x00003905,  //  57   1   1 140.795437
	0x00003905,  //  57   1   1 140.795437
	0x00003A05,  //  58   1   1 143.181800
	0x00003A05,  //  58   1   1 143.181800
	0x00003A05,  //  58   1   1 143.181800
	0x00003B05,  //  59   1   1 145.568163
	0x00003B05,  //  59   1   1 145.568163
	0x00003C05,  //  60   1   1 147.954527
	0x00003C05,  //  60   1   1 147.954527
	0x00003C05,  //  60   1   1 147.954527
	0x00003D05,  //  61   1   1 150.340890
	0x00003D05,  //  61   1   1 150.340890
	0x00003E05,  //  62   1   1 152.727253
	0x00003E05,  //  62   1   1 152.727253
	0x00003F05,  //  63   1   1 155.113617
	0x00003F05,  //  63   1   1 155.113617
	0x00003F05,  //  63   1   1 155.113617
	0x00004005,  //  64   1   1 157.499980
	0x00004005,  //  64   1   1 157.499980
	0x00004105,  //  65   1   1 159.886343
	0x00004105,  //  65   1   1 159.886343
	0x00004105,  //  65   1   1 159.886343
	0x00004205,  //  66   1   1 162.272707
	0x00004205,  //  66   1   1 162.272707
	0x00004305,  //  67   1   1 164.659070
	0x00004305,  //  67   1   1 164.659070
	0x0000e721,  //  68   1   1 166.81
	0x0000e721,  //  68   1   1 166.81
	0x0000e721,  //  68   1   1 166.81
	0x00004505,  //  69   1   1 169.431797
	0x00004505,  //  69   1   1 169.431797
	0x00004605,  //  70   1   1 171.818160
	0x00004605,  //  70   1   1 171.818160
	0x00004605,  //  70   1   1 171.818160
	0x00004705,  //  71   1   1 174.204523
	0x00004705,  //  71   1   1 174.204523
	0x00004805,  //  72   1   1 176.590887
	0x00004805,  //  72   1   1 176.590887
	0x00004905,  //  73   1   1 178.977250
	0x00004905,  //  73   1   1 178.977250
	0x00004905,  //  73   1   1 178.977250
	0x00004A05,  //  74   1   1 181.363613
	0x00004A05,  //  74   1   1 181.363613
	0x00004B05,  //  75   1   1 183.749977
	0x00004B05,  //  75   1   1 183.749977
	0x00004C05,  //  76   1   1 186.136340
	0x00004C05,  //  76   1   1 186.136340
	0x00004C05,  //  76   1   1 186.136340
	0x00004D05,  //  77   1   1 188.522703
	0x00004D05,  //  77   1   1 188.522703
	0x00004E05,  //  78   1   1 190.909067
	0x00004E05,  //  78   1   1 190.909067
	0x00004E05,  //  78   1   1 190.909067
	0x00004F05,  //  79   1   1 193.295430
	0x00004F05,  //  79   1   1 193.295430
	0x00005005,  //  80   1   1 195.681793
	0x00005005,  //  80   1   1 195.681793
	0x00005105,  //  81   1   1 198.068157
	0x00005105,  //  81   1   1 198.068157
	0x00005105,  //  81   1   1 198.068157
	0x00005205,  //  82   1   1 200.454520
	0x00005205,  //  82   1   1 200.454520
	0x00005305,  //  83   1   1 202.840883
	0x00005305,  //  83   1   1 202.840883
	0x00005305,  //  83   1   1 202.840883
	0x00005405,  //  84   1   1 205.227247
	0x00005405,  //  84   1   1 205.227247
	0x00005505,  //  85   1   1 207.613610
	0x00005505,  //  85   1   1 207.613610
	0x00005605,  //  86   1   1 209.999973
	0x00005605,  //  86   1   1 209.999973
	0x00005605,  //  86   1   1 209.999973
	0x00005705,  //  87   1   1 212.386337
	0x00005705,  //  87   1   1 212.386337
	0x00005805,  //  88   1   1 214.772700
	0x00005805,  //  88   1   1 214.772700
	0x00005905,  //  89   1   1 217.159063
	0x00005905,  //  89   1   1 217.159063
	0x00005905,  //  89   1   1 217.159063
	0x00005A05,  //  90   1   1 219.545427
	0x00005A05,  //  90   1   1 219.545427
};

FxU32 *h4pllTable = (FxU32*) (_h4pllTable - MIN_PLL_FREQ);

