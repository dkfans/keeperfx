/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_arrays.c
 *     Helper arrays for the engine.
 * @par Purpose:
 *     Defines arrays, their initialization and routines to use.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     02 Apr 2010 - 06 Nov 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engine_arrays.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "engine_lenses.h"
#include "config.h"
#include "front_simple.h"
#include "engine_render.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_td_iso[TD_ISO_POINTS];
#define td_iso _DK_td_iso
DLLIMPORT short _DK_iso_td[TD_ISO_POINTS];
#define iso_td _DK_iso_td
unsigned short floor_to_ceiling_map[FLOOR_TO_CEILING_MAP_LEN];
struct WibbleTable blank_wibble_table[128];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
short convert_td_iso(short n)
{
    if ((lens_mode == 2) || (lens_mode == 3))
    {
        if ((n < TD_ISO_POINTS) && (iso_td[n] >= 0))
          return iso_td[n];
        else if ((n >= KEEPERSPRITE_ADD_OFFSET) && (n < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM))
        {
            return iso_td_add[n - KEEPERSPRITE_ADD_OFFSET];
        }
    } else
    {
        if ((n < TD_ISO_POINTS) && (td_iso[n] >= 0))
          return td_iso[n];
        else if ((n >= KEEPERSPRITE_ADD_OFFSET) && (n < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM))
        {
            return td_iso_add[n - KEEPERSPRITE_ADD_OFFSET];
        }
    }
    return n;
}

short straight_td_iso(short n)
{
    if (n < TD_ISO_POINTS)
        return td_iso[n];
    else if ((n >= KEEPERSPRITE_ADD_OFFSET) && (n < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM))
    {
        return td_iso_add[n - KEEPERSPRITE_ADD_OFFSET];
    }
    return n;
}

short straight_iso_td(short n)
{
    if (n < TD_ISO_POINTS)
        return iso_td[n];
    else if ((n >= KEEPERSPRITE_ADD_OFFSET) && (n < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM))
    {
        return iso_td_add[n - KEEPERSPRITE_ADD_OFFSET];
    }
    return n;
}

void init_iso_3d_conversion_tables(void)
{
  long i;
  for (i=0; i < TD_ISO_POINTS; i++)
  {
    td_iso[i] = -1;
    iso_td[i] = -1;
  }
  td_iso[48] = 49;
  iso_td[49] = 48;
  td_iso[404] = 405;
  td_iso[406] = 407;
  td_iso[402] = 403;
  td_iso[408] = 409;
  td_iso[412] = 413;
  td_iso[410] = 411;
  td_iso[420] = 421;
  td_iso[418] = 419;
  td_iso[416] = 417;
  td_iso[414] = 415;
  td_iso[422] = 423;
  td_iso[62] = 63;
  iso_td[405] = 404;
  iso_td[407] = 406;
  iso_td[403] = 402;
  iso_td[409] = 408;
  iso_td[413] = 412;
  iso_td[411] = 410;
  iso_td[421] = 420;
  iso_td[419] = 418;
  iso_td[417] = 416;
  iso_td[415] = 414;
  iso_td[423] = 422;
  iso_td[63] = 62;
  td_iso[250] = 251;
  td_iso[248] = 249;
  td_iso[252] = 253;
  td_iso[254] = 255;
  td_iso[258] = 259;
  td_iso[256] = 257;
  td_iso[266] = 267;
  td_iso[264] = 265;
  td_iso[262] = 263;
  td_iso[260] = 261;
  td_iso[268] = 269;
  td_iso[64] = 65;
  iso_td[251] = 250;
  iso_td[249] = 248;
  iso_td[253] = 252;
  iso_td[255] = 254;
  iso_td[259] = 258;
  iso_td[257] = 256;
  iso_td[267] = 266;
  iso_td[265] = 264;
  iso_td[263] = 262;
  iso_td[261] = 260;
  iso_td[269] = 268;
  iso_td[65] = 64;
  td_iso[26] = 27;
  td_iso[24] = 25;
  td_iso[28] = 29;
  td_iso[30] = 31;
  td_iso[34] = 35;
  td_iso[32] = 33;
  td_iso[42] = 43;
  td_iso[40] = 41;
  td_iso[38] = 39;
  td_iso[36] = 37;
  td_iso[44] = 45;
  iso_td[27] = 26;
  iso_td[25] = 24;
  iso_td[29] = 28;
  iso_td[31] = 30;
  iso_td[35] = 34;
  iso_td[33] = 32;
  iso_td[43] = 42;
  iso_td[41] = 40;
  iso_td[39] = 38;
  iso_td[37] = 36;
  iso_td[45] = 44;
  td_iso[732] = 733;
  td_iso[730] = 731;
  td_iso[734] = 735;
  td_iso[736] = 737;
  td_iso[740] = 741;
  td_iso[738] = 739;
  td_iso[748] = 749;
  td_iso[746] = 747;
  td_iso[744] = 745;
  td_iso[742] = 743;
  td_iso[750] = 751;
  td_iso[92] = 93;
  iso_td[733] = 732;
  iso_td[731] = 730;
  iso_td[735] = 734;
  iso_td[737] = 736;
  iso_td[741] = 740;
  iso_td[739] = 738;
  iso_td[749] = 748;
  iso_td[747] = 746;
  iso_td[745] = 744;
  iso_td[743] = 742;
  iso_td[751] = 750;
  iso_td[93] = 92;
  td_iso[710] = 711;
  td_iso[708] = 709;
  td_iso[712] = 713;
  td_iso[714] = 715;
  td_iso[718] = 719;
  td_iso[716] = 717;
  td_iso[726] = 727;
  td_iso[724] = 725;
  td_iso[722] = 723;
  td_iso[720] = 721;
  td_iso[728] = 729;
  td_iso[90] = 91;
  iso_td[711] = 710;
  iso_td[709] = 708;
  iso_td[713] = 712;
  iso_td[715] = 714;
  iso_td[719] = 718;
  iso_td[717] = 716;
  iso_td[727] = 726;
  iso_td[725] = 724;
  iso_td[723] = 722;
  iso_td[721] = 720;
  iso_td[729] = 728;
  iso_td[91] = 90;
  td_iso[236] = 237;
  td_iso[228] = 229;
  td_iso[230] = 231;
  td_iso[232] = 233;
  td_iso[234] = 235;
  td_iso[244] = 245;
  td_iso[242] = 243;
  td_iso[240] = 241;
  td_iso[238] = 239;
  td_iso[246] = 247;
  td_iso[60] = 61;
  iso_td[237] = 236;
  iso_td[229] = 228;
  iso_td[227] = 226;
  iso_td[231] = 230;
  iso_td[233] = 232;
  iso_td[235] = 234;
  iso_td[245] = 244;
  iso_td[243] = 242;
  iso_td[241] = 240;
  iso_td[239] = 238;
  iso_td[247] = 246;
  iso_td[61] = 60;
  td_iso[170] = 171;
  td_iso[162] = 163;
  td_iso[160] = 161;
  td_iso[164] = 165;
  td_iso[166] = 167;
  td_iso[168] = 169;
  td_iso[178] = 179;
  td_iso[176] = 177;
  td_iso[174] = 175;
  td_iso[172] = 173;
  td_iso[180] = 181;
  iso_td[171] = 170;
  iso_td[163] = 162;
  iso_td[161] = 160;
  iso_td[165] = 164;
  iso_td[167] = 166;
  iso_td[169] = 168;
  iso_td[179] = 178;
  iso_td[177] = 176;
  iso_td[175] = 174;
  iso_td[173] = 172;
  iso_td[181] = 180;
  td_iso[280] = 281;
  td_iso[270] = 271;
  td_iso[272] = 273;
  td_iso[274] = 275;
  td_iso[276] = 277;
  td_iso[288] = 289;
  td_iso[286] = 287;
  td_iso[284] = 285;
  td_iso[282] = 283;
  td_iso[278] = 279;
  td_iso[290] = 291;
  td_iso[58] = 59;
  iso_td[281] = 280;
  iso_td[271] = 270;
  iso_td[273] = 272;
  iso_td[275] = 274;
  iso_td[277] = 276;
  iso_td[289] = 288;
  iso_td[287] = 286;
  iso_td[285] = 284;
  iso_td[283] = 282;
  iso_td[279] = 278;
  iso_td[291] = 290;
  iso_td[59] = 58;
  td_iso[214] = 215;
  td_iso[206] = 207;
  td_iso[204] = 205;
  td_iso[208] = 209;
  td_iso[210] = 211;
  td_iso[212] = 213;
  td_iso[222] = 223;
  td_iso[220] = 221;
  td_iso[218] = 219;
  td_iso[216] = 217;
  td_iso[224] = 225;
  iso_td[215] = 214;
  iso_td[207] = 206;
  iso_td[205] = 204;
  iso_td[209] = 208;
  iso_td[211] = 210;
  iso_td[213] = 212;
  iso_td[223] = 222;
  iso_td[221] = 220;
  iso_td[219] = 218;
  iso_td[217] = 216;
  iso_td[225] = 224;
  td_iso[368] = 369;
  td_iso[360] = 361;
  td_iso[358] = 359;
  td_iso[362] = 363;
  td_iso[364] = 365;
  td_iso[366] = 367;
  td_iso[376] = 377;
  td_iso[374] = 375;
  td_iso[372] = 373;
  td_iso[370] = 371;
  td_iso[378] = 379;
  iso_td[369] = 368;
  iso_td[361] = 360;
  iso_td[359] = 358;
  iso_td[363] = 362;
  iso_td[365] = 364;
  iso_td[367] = 366;
  iso_td[377] = 376;
  iso_td[375] = 374;
  iso_td[373] = 372;
  iso_td[371] = 370;
  iso_td[379] = 378;
  td_iso[346] = 347;
  td_iso[338] = 339;
  td_iso[336] = 337;
  td_iso[340] = 341;
  td_iso[342] = 343;
  td_iso[344] = 345;
  td_iso[354] = 355;
  td_iso[352] = 353;
  td_iso[350] = 351;
  td_iso[348] = 349;
  td_iso[356] = 357;
  iso_td[347] = 346;
  iso_td[339] = 338;
  iso_td[337] = 336;
  iso_td[341] = 340;
  iso_td[343] = 342;
  iso_td[345] = 344;
  iso_td[355] = 354;
  iso_td[353] = 352;
  iso_td[351] = 350;
  iso_td[349] = 348;
  iso_td[357] = 356;
  td_iso[302] = 303;
  td_iso[294] = 295;
  td_iso[292] = 293;
  td_iso[296] = 297;
  td_iso[298] = 299;
  td_iso[300] = 301;
  td_iso[310] = 311;
  td_iso[308] = 309;
  td_iso[306] = 307;
  td_iso[304] = 305;
  td_iso[312] = 313;
  iso_td[303] = 302;
  iso_td[295] = 294;
  iso_td[293] = 292;
  iso_td[297] = 296;
  iso_td[299] = 298;
  iso_td[301] = 300;
  iso_td[311] = 310;
  iso_td[309] = 308;
  iso_td[307] = 306;
  iso_td[305] = 304;
  iso_td[313] = 312;
  td_iso[390] = 391;
  td_iso[382] = 383;
  td_iso[380] = 381;
  td_iso[384] = 385;
  td_iso[386] = 387;
  td_iso[388] = 389;
  td_iso[398] = 399;
  td_iso[396] = 397;
  td_iso[394] = 395;
  td_iso[392] = 393;
  td_iso[400] = 401;
  td_iso[54] = 55;
  iso_td[391] = 390;
  iso_td[383] = 382;
  iso_td[381] = 380;
  iso_td[385] = 384;
  iso_td[387] = 386;
  iso_td[389] = 388;
  iso_td[399] = 398;
  iso_td[397] = 396;
  iso_td[395] = 394;
  iso_td[393] = 392;
  iso_td[401] = 400;
  iso_td[55] = 54;
  td_iso[456] = 457;
  td_iso[448] = 449;
  td_iso[446] = 447;
  td_iso[450] = 451;
  td_iso[452] = 453;
  td_iso[454] = 455;
  td_iso[464] = 465;
  td_iso[462] = 463;
  td_iso[460] = 461;
  td_iso[458] = 459;
  td_iso[466] = 467;
  td_iso[68] = 69;
  iso_td[457] = 456;
  iso_td[449] = 448;
  iso_td[447] = 446;
  iso_td[451] = 450;
  iso_td[453] = 452;
  iso_td[455] = 454;
  iso_td[465] = 464;
  iso_td[463] = 462;
  iso_td[461] = 460;
  iso_td[459] = 458;
  iso_td[467] = 466;
  iso_td[69] = 68;
  td_iso[688] = 689;
  td_iso[686] = 687;
  td_iso[690] = 691;
  td_iso[692] = 693;
  td_iso[694] = 695;
  td_iso[696] = 697;
  td_iso[704] = 705;
  td_iso[702] = 703;
  td_iso[700] = 701;
  td_iso[698] = 699;
  td_iso[706] = 707;
  td_iso[88] = 89;
  iso_td[689] = 688;
  iso_td[687] = 686;
  iso_td[691] = 690;
  iso_td[693] = 692;
  iso_td[695] = 694;
  iso_td[697] = 696;
  iso_td[705] = 704;
  iso_td[703] = 702;
  iso_td[701] = 700;
  iso_td[699] = 698;
  iso_td[707] = 706;
  iso_td[89] = 88;
  td_iso[468] = 469;
  td_iso[470] = 471;
  td_iso[472] = 473;
  td_iso[474] = 475;
  td_iso[476] = 477;
  td_iso[478] = 479;
  td_iso[480] = 481;
  td_iso[482] = 483;
  td_iso[484] = 485;
  td_iso[486] = 487;
  td_iso[70] = 71;
  iso_td[469] = 468;
  iso_td[471] = 470;
  iso_td[473] = 472;
  iso_td[475] = 474;
  iso_td[477] = 476;
  iso_td[479] = 478;
  iso_td[481] = 480;
  iso_td[483] = 482;
  iso_td[485] = 484;
  iso_td[487] = 486;
  iso_td[71] = 70;
  td_iso[754] = 755;
  td_iso[752] = 753;
  td_iso[756] = 757;
  td_iso[758] = 759;
  td_iso[760] = 761;
  td_iso[762] = 763;
  td_iso[770] = 771;
  td_iso[768] = 769;
  td_iso[766] = 767;
  td_iso[764] = 765;
  td_iso[772] = 773;
  td_iso[94] = 95;
  iso_td[755] = 754;
  iso_td[753] = 752;
  iso_td[757] = 756;
  iso_td[759] = 758;
  iso_td[761] = 760;
  iso_td[763] = 762;
  iso_td[771] = 770;
  iso_td[769] = 768;
  iso_td[767] = 766;
  iso_td[765] = 764;
  iso_td[773] = 772;
  iso_td[95] = 94;
  td_iso[580] = 581;
  td_iso[578] = 579;
  td_iso[582] = 583;
  td_iso[584] = 585;
  td_iso[586] = 587;
  td_iso[588] = 589;
  td_iso[596] = 597;
  td_iso[594] = 595;
  td_iso[592] = 593;
  td_iso[590] = 591;
  td_iso[598] = 599;
  td_iso[78] = 79;
  iso_td[581] = 580;
  iso_td[579] = 578;
  iso_td[583] = 582;
  iso_td[585] = 584;
  iso_td[587] = 586;
  iso_td[589] = 588;
  iso_td[597] = 596;
  iso_td[595] = 594;
  iso_td[593] = 592;
  iso_td[591] = 590;
  iso_td[599] = 598;
  iso_td[79] = 78;
  td_iso[644] = 645;
  td_iso[646] = 647;
  td_iso[648] = 649;
  td_iso[650] = 651;
  td_iso[652] = 653;
  td_iso[658] = 659;
  td_iso[660] = 661;
  td_iso[656] = 657;
  td_iso[654] = 655;
  td_iso[662] = 663;
  td_iso[84] = 85;
  iso_td[645] = 644;
  iso_td[647] = 646;
  iso_td[649] = 648;
  iso_td[651] = 650;
  iso_td[653] = 652;
  iso_td[659] = 658;
  iso_td[661] = 660;
  iso_td[657] = 656;
  iso_td[655] = 654;
  iso_td[663] = 662;
  iso_td[85] = 84;
  td_iso[532] = 533;
  td_iso[534] = 535;
  td_iso[536] = 537;
  td_iso[538] = 539;
  td_iso[540] = 541;
  td_iso[542] = 543;
  td_iso[550] = 551;
  td_iso[548] = 549;
  td_iso[546] = 547;
  td_iso[544] = 545;
  td_iso[552] = 553;
  td_iso[76] = 77;
  iso_td[533] = 532;
  iso_td[535] = 534;
  iso_td[537] = 536;
  iso_td[539] = 538;
  iso_td[541] = 540;
  iso_td[543] = 542;
  iso_td[551] = 550;
  iso_td[549] = 548;
  iso_td[547] = 546;
  iso_td[545] = 544;
  iso_td[553] = 552;
  iso_td[77] = 76;
  td_iso[666] = 667;
  td_iso[668] = 669;
  td_iso[664] = 665;
  td_iso[670] = 671;
  td_iso[674] = 675;
  td_iso[672] = 673;
  td_iso[682] = 683;
  td_iso[680] = 681;
  td_iso[678] = 679;
  td_iso[676] = 677;
  td_iso[684] = 685;
  td_iso[86] = 87;
  iso_td[667] = 666;
  iso_td[669] = 668;
  iso_td[665] = 664;
  iso_td[671] = 670;
  iso_td[675] = 674;
  iso_td[673] = 672;
  iso_td[683] = 682;
  iso_td[681] = 680;
  iso_td[679] = 678;
  iso_td[677] = 676;
  iso_td[685] = 684;
  iso_td[87] = 86;
  td_iso[424] = 425;
  td_iso[426] = 427;
  td_iso[428] = 429;
  td_iso[430] = 431;
  td_iso[432] = 433;
  td_iso[434] = 435;
  td_iso[436] = 437;
  td_iso[438] = 439;
  td_iso[440] = 441;
  td_iso[442] = 443;
  td_iso[444] = 445;
  td_iso[52] = 53;
  iso_td[425] = 424;
  iso_td[427] = 426;
  iso_td[429] = 428;
  iso_td[431] = 430;
  iso_td[433] = 432;
  iso_td[435] = 434;
  iso_td[437] = 436;
  iso_td[439] = 438;
  iso_td[441] = 440;
  iso_td[443] = 442;
  iso_td[445] = 444;
  iso_td[53] = 52;
  td_iso[512] = 513;
  td_iso[510] = 511;
  td_iso[514] = 515;
  td_iso[516] = 517;
  td_iso[518] = 519;
  td_iso[520] = 521;
  td_iso[522] = 523;
  td_iso[524] = 525;
  td_iso[528] = 529;
  td_iso[526] = 527;
  td_iso[530] = 531;
  td_iso[50] = 51;
  iso_td[513] = 512;
  iso_td[511] = 510;
  iso_td[515] = 514;
  iso_td[517] = 516;
  iso_td[519] = 518;
  iso_td[521] = 520;
  iso_td[523] = 522;
  iso_td[525] = 524;
  iso_td[529] = 528;
  iso_td[527] = 526;
  iso_td[531] = 530;
  iso_td[51] = 50;
  td_iso[2] = 3;
  td_iso[0] = 1;
  td_iso[4] = 5;
  td_iso[6] = 7;
  td_iso[8] = 9;
  td_iso[10] = 11;
  td_iso[12] = 13;
  td_iso[14] = 15;
  td_iso[16] = 17;
  td_iso[18] = 19;
  td_iso[22] = 23;
  td_iso[20] = 21;
  td_iso[72] = 73;
  iso_td[3] = 2;
  iso_td[1] = 0;
  iso_td[5] = 4;
  iso_td[7] = 6;
  iso_td[9] = 8;
  iso_td[11] = 10;
  iso_td[13] = 12;
  iso_td[15] = 14;
  iso_td[19] = 18;
  iso_td[17] = 16;
  iso_td[23] = 22;
  iso_td[21] = 20;
  iso_td[73] = 72;
  td_iso[314] = 315;
  td_iso[316] = 317;
  td_iso[318] = 319;
  td_iso[320] = 321;
  td_iso[322] = 323;
  td_iso[324] = 325;
  td_iso[326] = 327;
  td_iso[328] = 329;
  td_iso[330] = 331;
  td_iso[332] = 333;
  td_iso[334] = 335;
  iso_td[315] = 314;
  iso_td[317] = 316;
  iso_td[319] = 318;
  iso_td[321] = 320;
  iso_td[323] = 322;
  iso_td[325] = 324;
  iso_td[327] = 326;
  iso_td[329] = 328;
  iso_td[331] = 330;
  iso_td[333] = 332;
  iso_td[335] = 334;
  td_iso[602] = 603;
  td_iso[600] = 601;
  td_iso[604] = 605;
  td_iso[606] = 607;
  td_iso[608] = 609;
  td_iso[610] = 611;
  td_iso[618] = 619;
  td_iso[616] = 617;
  td_iso[614] = 615;
  td_iso[612] = 613;
  td_iso[620] = 621;
  td_iso[80] = 81;
  iso_td[603] = 602;
  iso_td[601] = 600;
  iso_td[605] = 604;
  iso_td[607] = 606;
  iso_td[609] = 608;
  iso_td[611] = 610;
  iso_td[619] = 618;
  iso_td[617] = 616;
  iso_td[615] = 614;
  iso_td[613] = 612;
  iso_td[621] = 620;
  iso_td[81] = 80;
  td_iso[624] = 625;
  td_iso[622] = 623;
  td_iso[626] = 627;
  td_iso[628] = 629;
  td_iso[630] = 631;
  td_iso[632] = 633;
  td_iso[640] = 641;
  td_iso[638] = 639;
  td_iso[636] = 637;
  td_iso[634] = 635;
  td_iso[642] = 643;
  td_iso[82] = 83;
  iso_td[625] = 624;
  iso_td[623] = 622;
  iso_td[627] = 626;
  iso_td[629] = 628;
  iso_td[631] = 630;
  iso_td[633] = 632;
  iso_td[641] = 640;
  iso_td[639] = 638;
  iso_td[637] = 636;
  iso_td[635] = 634;
  iso_td[643] = 642;
  iso_td[83] = 82;
  td_iso[490] = 491;
  td_iso[488] = 489;
  td_iso[492] = 493;
  td_iso[494] = 495;
  td_iso[496] = 497;
  td_iso[498] = 499;
  td_iso[506] = 507;
  td_iso[504] = 505;
  td_iso[502] = 503;
  td_iso[500] = 501;
  td_iso[508] = 509;
  td_iso[74] = 75;
  iso_td[491] = 490;
  iso_td[489] = 488;
  iso_td[493] = 492;
  iso_td[495] = 494;
  iso_td[497] = 496;
  iso_td[499] = 498;
  iso_td[507] = 506;
  iso_td[505] = 504;
  iso_td[503] = 502;
  iso_td[501] = 500;
  iso_td[509] = 508;
  iso_td[75] = 74;
  td_iso[184] = 185;
  td_iso[182] = 183;
  td_iso[186] = 187;
  td_iso[188] = 189;
  td_iso[190] = 191;
  td_iso[192] = 193;
  td_iso[200] = 201;
  td_iso[198] = 199;
  td_iso[196] = 197;
  td_iso[194] = 195;
  td_iso[202] = 203;
  td_iso[66] = 67;
  iso_td[185] = 184;
  iso_td[183] = 182;
  iso_td[187] = 186;
  iso_td[189] = 188;
  iso_td[191] = 190;
  iso_td[193] = 192;
  iso_td[201] = 200;
  iso_td[199] = 198;
  iso_td[197] = 196;
  iso_td[195] = 194;
  iso_td[203] = 202;
  iso_td[67] = 66;
  td_iso[554] = 555;
  td_iso[556] = 557;
  td_iso[558] = 559;
  td_iso[560] = 561;
  td_iso[562] = 563;
  td_iso[564] = 565;
  td_iso[566] = 567;
  td_iso[568] = 569;
  td_iso[570] = 571;
  td_iso[572] = 573;
  td_iso[574] = 575;
  td_iso[576] = 577;
  td_iso[56] = 57;
  iso_td[555] = 554;
  iso_td[557] = 556;
  iso_td[559] = 558;
  iso_td[561] = 560;
  iso_td[563] = 562;
  iso_td[565] = 564;
  iso_td[567] = 566;
  iso_td[569] = 568;
  iso_td[571] = 570;
  iso_td[573] = 572;
  iso_td[575] = 574;
  iso_td[577] = 576;
  iso_td[57] = 56;
  td_iso[896] = 900;
  td_iso[819] = 0x335;
  td_iso[820] = 0x336;
  td_iso[930] = 931;
  td_iso[789] = 788;
  td_iso[791] = 790;
  td_iso[793] = 792;
  td_iso[794] = 0x31A;
  td_iso[795] = 0x31A;
  td_iso[892] = 0x37B;
  td_iso[893] = 0x381;
  td_iso[894] = 0x382;
  td_iso[895] = 0x383;
  td_iso[804] = 803;
  td_iso[806] = 805;
  td_iso[808] = 807;
  td_iso[810] = 809;
  td_iso[812] = 811;
  td_iso[814] = 813;
  td_iso[816] = 815;
  td_iso[818] = 817;
  td_iso[128] = 129;
  td_iso[132] = 133;
  td_iso[134] = 135;
  td_iso[136] = 137;
  td_iso[138] = 139;
  td_iso[140] = 141;
  td_iso[142] = 143;
  td_iso[144] = 145;
  td_iso[146] = 147;
  td_iso[148] = 149;
  td_iso[150] = 151;
  td_iso[152] = 153;
  td_iso[124] = 125;
  td_iso[154] = 155;
  td_iso[156] = 157;
  td_iso[158] = 159;
  td_iso[126] = 127;
  td_iso[226] = 227;
  td_iso[776] = 0x30B;
  td_iso[777] = 0x30A;
  td_iso[962] = 963;
  td_iso[950] = 951;
  td_iso[905] = 906;
  td_iso[932] = 933;
  td_iso[934] = 935;
  td_iso[948] = 949;
  td_iso[903] = 904;
  td_iso[840] = 0x34A;
  td_iso[841] = 0x34B;
  td_iso[844] = 0x34F;
  td_iso[845] = 0x350;
  td_iso[846] = 849;
  td_iso[836] = 0x347;
  td_iso[837] = 0x346;
  td_iso[825] = 826;
  td_iso[861] = 862;
  td_iso[936] = 0x3AD;
  td_iso[937] = 0x3AE;
  td_iso[938] = 0x3AF;
  td_iso[939] = 0x3B0;
  td_iso[940] = 945;
  td_iso[774] = 775;
  td_iso[922] = 923;
  td_iso[867] = 0x366;
  td_iso[868] = 0x367;
  td_iso[869] = 872;
  td_iso[873] = 0x36C;
  td_iso[874] = 0x36D;
  td_iso[875] = 878;
  td_iso[879] = 0x372;
  td_iso[880] = 0x373;
  td_iso[881] = 884;
  td_iso[885] = 0x378;
  td_iso[886] = 0x379;
  td_iso[887] = 890;
  td_iso[901] = 902;
  td_iso[130] = 131;
  iso_td[897] = 893;
  iso_td[900] = 896;
  iso_td[821] = 0x333;
  iso_td[822] = 0x334;
  iso_td[931] = 930;
  iso_td[788] = 789;
  iso_td[790] = 791;
  iso_td[792] = 793;
  iso_td[794] = 795;
  iso_td[803] = 804;
  iso_td[805] = 806;
  iso_td[807] = 808;
  iso_td[809] = 810;
  iso_td[811] = 812;
  iso_td[813] = 814;
  iso_td[815] = 816;
  iso_td[817] = 818;
  iso_td[129] = 128;
  iso_td[133] = 132;
  iso_td[135] = 134;
  iso_td[137] = 136;
  iso_td[139] = 138;
  iso_td[141] = 140;
  iso_td[143] = 142;
  iso_td[145] = 144;
  iso_td[147] = 146;
  iso_td[149] = 148;
  iso_td[151] = 150;
  iso_td[153] = 152;
  iso_td[125] = 124;
  iso_td[155] = 154;
  iso_td[157] = 156;
  iso_td[159] = 158;
  iso_td[963] = 962;
  iso_td[951] = 950;
  iso_td[906] = 905;
  iso_td[933] = 932;
  iso_td[935] = 934;
  iso_td[949] = 948;
  iso_td[904] = 903;
  iso_td[842] = 0x348;
  iso_td[843] = 0x349;
  iso_td[847] = 0x34C;
  iso_td[848] = 0x34D;
  iso_td[849] = 846;
  iso_td[838] = 837;
  iso_td[826] = 825;
  iso_td[862] = 861;
  iso_td[941] = 0x3A8;
  iso_td[942] = 0x3A9;
  iso_td[943] = 0x3AA;
  iso_td[944] = 0x3AB;
  iso_td[898] = 0x382;
  iso_td[899] = 0x37F;
  iso_td[945] = 940;
  iso_td[775] = 774;
  iso_td[923] = 922;
  iso_td[870] = 0x363;
  iso_td[871] = 0x364;
  iso_td[872] = 869;
  iso_td[876] = 0x369;
  iso_td[877] = 0x36A;
  iso_td[878] = 875;
  iso_td[882] = 0x36F;
  iso_td[883] = 0x370;
  iso_td[884] = 881;
  iso_td[888] = 0x375;
  iso_td[889] = 0x376;
  iso_td[890] = 0x377;
  iso_td[891] = 0x37C;
  iso_td[778] = 0x309;
  iso_td[779] = 0x308;
  iso_td[902] = 901;
  iso_td[131] = 130;
  td_iso[120] = 121;
  iso_td[121] = 120;
  td_iso[114] = 115;
  iso_td[115] = 114;
  td_iso[110] = 111;
  iso_td[111] = 110;
  td_iso[102] = 103;
  iso_td[103] = 102;
  td_iso[104] = 105;
  iso_td[105] = 104;
  td_iso[106] = 107;
  iso_td[107] = 106;
  td_iso[108] = 109;
  iso_td[109] = 108;
  td_iso[100] = 101;
  iso_td[101] = 100;
  td_iso[98] = 99;
  iso_td[99] = 98;
  td_iso[46] = 47;
  iso_td[47] = 46;
  td_iso[952] = 953;
  iso_td[953] = 952;
  td_iso[954] = 955;
  iso_td[955] = 954;
  td_iso[956] = 957;
  iso_td[957] = 956;
  td_iso[958] = 959;
  iso_td[959] = 958;
  td_iso[960] = 961;
  iso_td[961] = 960;
  td_iso[946] = 947;
  iso_td[947] = 946;
}

/**
 * Fills randomisors array used for mesh deformations.
 */
void setup_3d(void)
{
    unsigned long seed;
    long i;
    long k;
    SYNCDBG(6,"Starting");
    seed = 0x0f0f0f0f;
    for (i=0; i < RANDOMISORS_LEN; i++)
    {
        // fill with value -RANDOMISORS_RANGE..RANDOMISORS_RANGE
        k = LB_RANDOM(2*RANDOMISORS_RANGE+1, &seed);
        randomisors[i] = k - RANDOMISORS_RANGE;
    }
}

void fill_floor_heights_table(void)
{
    long top_height;
    long btm_height;
    long shade_back;
    unsigned long flag_bit;
    long i;
    long n;
    for (n=0; n < 256; n++)
    {
        i = 0;
        flag_bit = 1;
        btm_height = i;
        top_height = i;
        for (; i < 8; i++)
        {
            if ((flag_bit & n) == 0)
              break;
            flag_bit = (flag_bit << 1);
        }
        shade_back = i;
        for (; i < 8; i++)
        {
            if ((flag_bit & n) != 0)
              break;
            flag_bit = (flag_bit << 1);
        }
        if (i < 8)
        {
            btm_height = i;
            for (; i < 8; i++)
            {
                if ((flag_bit & n) == 0)
                  break;
                flag_bit = (flag_bit << 1);
            }
            top_height = i;
        }
        lintel_bottom_height[n] = btm_height;
        lintel_top_height[n] = top_height;
        _DK_floor_height[n] = shade_back;
    }
}

/**
 * Modification of LB_RANDOM() which allows generating Wibble values same to original game.
 */
unsigned short wibble_random(unsigned short range, unsigned short *seed)
{
    if (range == 0)
        return 0;
    unsigned short i;
    *seed = 9377 * (*seed) + 9439;
    i = (*seed) % range;
    return i;
}

void generate_wibble_table(void)
{
    struct WibbleTable *wibl;
    struct WibbleTable *empty_wibl;
    struct WibbleTable *qwibl;
    unsigned short seed;
    int i;
    int n;
    // Clear the whole wibble table and create an empty wibble table
    for (n=0; n < 4; n++)
    {
        wibl = &wibble_table[32*n];
        empty_wibl = &blank_wibble_table[32*n];
        for (i=0; i < 32; i++)
        {
            LbMemorySet(wibl, 0, sizeof(struct WibbleTable));
            wibl++;
            LbMemorySet(empty_wibl, 0, sizeof(struct WibbleTable));
            empty_wibl++;
        }
    }
    if (wibble_enabled() || liquid_wibble_enabled())
    {
        // Set wibble values using special random algorithm
        seed = 0;
        for (i=0; i < 32; i++)
        {
            wibl = &wibble_table[i+32];
            n = wibble_random(65447,&seed);
            wibl->field_0 = (n % 127) - 63;
            n = wibble_random(65447,&seed);
            wibl->field_4 = ((n % 127) - 63) / 3;
            n = wibble_random(65447,&seed);
            wibl->field_8 = (n % 127) - 63;
            qwibl = &wibble_table[i+64];
            n = wibble_random(65447,&seed);
            wibl->field_C = (n % 2047) - 1023;
            n = wibble_random(65447,&seed);
            qwibl->field_0 = (n % 127) - 63;
            n = wibble_random(65447,&seed);
            qwibl->field_8 = (n % 127) - 63;
        }
    }
}

TbBool load_ceiling_table(void)
{
    char *fname;
    TbFileHandle fh;
    unsigned short *value_array;
    char nchr;
    char numstr[8];
    TbBool do_next;
    long i;
    long n;
    //_DK_load_ceiling_table(); return true;
    // Prepare filename and open the file
    wait_for_cd_to_be_available();
    fname = prepare_file_path(FGrp_StdData,"ceiling.txt");
    fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (fh == -1) {
        return false;
    }

    value_array = &floor_to_ceiling_map[0];
    n = 0;
    do_next = 1;
    while (do_next == 1)
    {
        {
            do_next = LbFileRead(fh, &nchr, 1);
            if (do_next != 1)
                break;
            if ( (nchr == 10) || (nchr == 44) || (nchr == 32) || (nchr == 9) || (nchr == 13) )
                continue;
        }
        LbMemorySet(numstr, 0, sizeof(numstr));
        for (i=0; i < sizeof(numstr); i++)
        {
            numstr[i] = nchr;
            do_next = LbFileRead(fh, &nchr, 1);
            if (do_next != 1)
                break;
            if ( (nchr == 10) || (nchr == 44) || (nchr == 32) || (nchr == 9) || (nchr == 13) )
                break;
        }
        value_array[n] = atol(numstr);
        n++;
        if (n >= sizeof(floor_to_ceiling_map)/sizeof(floor_to_ceiling_map[0]))
        {
            do_next = 0;
        }
    }
    LbFileClose(fh);
    memcpy(_DK_floor_to_ceiling_map,floor_to_ceiling_map,sizeof(floor_to_ceiling_map));
    return true;
}

/******************************************************************************/
