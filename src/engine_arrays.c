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
#include "pre_inc.h"
#include "engine_arrays.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_fileio.h"
#include "engine_lenses.h"
#include "config.h"
#include "front_simple.h"
#include "engine_render.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
short fp_to_td_animation[FP_TD_ANIMATION_COUNT];
short td_to_fp_animation[FP_TD_ANIMATION_COUNT];
unsigned short floor_to_ceiling_map[TEXTURE_BLOCKS_COUNT];
struct WibbleTable blank_wibble_table[128];

int32_t randomisors[512];
struct WibbleTable wibble_table[128];
long floor_height_table[256];
long lintel_top_height[256];
long lintel_bottom_height[256];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
short get_td_animation_sprite(short animation_sprite)
{
    if ((animation_sprite >= 0) && (animation_sprite < FP_TD_ANIMATION_COUNT) && (fp_to_td_animation[animation_sprite] >= 0)) {
        return fp_to_td_animation[animation_sprite];
    }
    if ((animation_sprite >= KEEPERSPRITE_ADD_OFFSET) && (animation_sprite < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)) {
        short td_sprite = fp_to_td_sprite_add[animation_sprite - KEEPERSPRITE_ADD_OFFSET];
        if (td_sprite > 0) {
            return td_sprite;
        }
    }
    return animation_sprite;
}

unsigned short get_render_animation_sprite(unsigned short animation_sprite)
{
    if ((lens_mode == 2) || (lens_mode == 3)) {
        if (animation_sprite < FP_TD_ANIMATION_COUNT) {
            short fp_sprite = td_to_fp_animation[animation_sprite];
            if (fp_sprite >= 0) {
                return (unsigned short)fp_sprite;
            }
        }
        if ((animation_sprite >= KEEPERSPRITE_ADD_OFFSET) && (animation_sprite < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)) {
            short fp_sprite = td_to_fp_sprite_add[animation_sprite - KEEPERSPRITE_ADD_OFFSET];
            if (fp_sprite > 0) {
                return (unsigned short)fp_sprite;
            }
        }
    }
    return animation_sprite;
}

void init_fp_td_animation_conversion_tables(void)
{
  long i;
  for (i=0; i < FP_TD_ANIMATION_COUNT; i++)
  {
    fp_to_td_animation[i] = -1;
    td_to_fp_animation[i] = -1;
  }
  fp_to_td_animation[48] = 49;
  td_to_fp_animation[49] = 48;
  fp_to_td_animation[404] = 405;
  fp_to_td_animation[406] = 407;
  fp_to_td_animation[402] = 403;
  fp_to_td_animation[408] = 409;
  fp_to_td_animation[412] = 413;
  fp_to_td_animation[410] = 411;
  fp_to_td_animation[420] = 421;
  fp_to_td_animation[418] = 419;
  fp_to_td_animation[416] = 417;
  fp_to_td_animation[414] = 415;
  fp_to_td_animation[422] = 423;
  fp_to_td_animation[62] = 63;
  td_to_fp_animation[405] = 404;
  td_to_fp_animation[407] = 406;
  td_to_fp_animation[403] = 402;
  td_to_fp_animation[409] = 408;
  td_to_fp_animation[413] = 412;
  td_to_fp_animation[411] = 410;
  td_to_fp_animation[421] = 420;
  td_to_fp_animation[419] = 418;
  td_to_fp_animation[417] = 416;
  td_to_fp_animation[415] = 414;
  td_to_fp_animation[423] = 422;
  td_to_fp_animation[63] = 62;
  fp_to_td_animation[250] = 251;
  fp_to_td_animation[248] = 249;
  fp_to_td_animation[252] = 253;
  fp_to_td_animation[254] = 255;
  fp_to_td_animation[258] = 259;
  fp_to_td_animation[256] = 257;
  fp_to_td_animation[266] = 267;
  fp_to_td_animation[264] = 265;
  fp_to_td_animation[262] = 263;
  fp_to_td_animation[260] = 261;
  fp_to_td_animation[268] = 269;
  fp_to_td_animation[64] = 65;
  td_to_fp_animation[251] = 250;
  td_to_fp_animation[249] = 248;
  td_to_fp_animation[253] = 252;
  td_to_fp_animation[255] = 254;
  td_to_fp_animation[259] = 258;
  td_to_fp_animation[257] = 256;
  td_to_fp_animation[267] = 266;
  td_to_fp_animation[265] = 264;
  td_to_fp_animation[263] = 262;
  td_to_fp_animation[261] = 260;
  td_to_fp_animation[269] = 268;
  td_to_fp_animation[65] = 64;
  fp_to_td_animation[26] = 27;
  fp_to_td_animation[24] = 25;
  fp_to_td_animation[28] = 29;
  fp_to_td_animation[30] = 31;
  fp_to_td_animation[34] = 35;
  fp_to_td_animation[32] = 33;
  fp_to_td_animation[42] = 43;
  fp_to_td_animation[40] = 41;
  fp_to_td_animation[38] = 39;
  fp_to_td_animation[36] = 37;
  fp_to_td_animation[44] = 45;
  td_to_fp_animation[27] = 26;
  td_to_fp_animation[25] = 24;
  td_to_fp_animation[29] = 28;
  td_to_fp_animation[31] = 30;
  td_to_fp_animation[35] = 34;
  td_to_fp_animation[33] = 32;
  td_to_fp_animation[43] = 42;
  td_to_fp_animation[41] = 40;
  td_to_fp_animation[39] = 38;
  td_to_fp_animation[37] = 36;
  td_to_fp_animation[45] = 44;
  fp_to_td_animation[732] = 733;
  fp_to_td_animation[730] = 731;
  fp_to_td_animation[734] = 735;
  fp_to_td_animation[736] = 737;
  fp_to_td_animation[740] = 741;
  fp_to_td_animation[738] = 739;
  fp_to_td_animation[748] = 749;
  fp_to_td_animation[746] = 747;
  fp_to_td_animation[744] = 745;
  fp_to_td_animation[742] = 743;
  fp_to_td_animation[750] = 751;
  fp_to_td_animation[92] = 93;
  td_to_fp_animation[733] = 732;
  td_to_fp_animation[731] = 730;
  td_to_fp_animation[735] = 734;
  td_to_fp_animation[737] = 736;
  td_to_fp_animation[741] = 740;
  td_to_fp_animation[739] = 738;
  td_to_fp_animation[749] = 748;
  td_to_fp_animation[747] = 746;
  td_to_fp_animation[745] = 744;
  td_to_fp_animation[743] = 742;
  td_to_fp_animation[751] = 750;
  td_to_fp_animation[93] = 92;
  fp_to_td_animation[710] = 711;
  fp_to_td_animation[708] = 709;
  fp_to_td_animation[712] = 713;
  fp_to_td_animation[714] = 715;
  fp_to_td_animation[718] = 719;
  fp_to_td_animation[716] = 717;
  fp_to_td_animation[726] = 727;
  fp_to_td_animation[724] = 725;
  fp_to_td_animation[722] = 723;
  fp_to_td_animation[720] = 721;
  fp_to_td_animation[728] = 729;
  fp_to_td_animation[90] = 91;
  td_to_fp_animation[711] = 710;
  td_to_fp_animation[709] = 708;
  td_to_fp_animation[713] = 712;
  td_to_fp_animation[715] = 714;
  td_to_fp_animation[719] = 718;
  td_to_fp_animation[717] = 716;
  td_to_fp_animation[727] = 726;
  td_to_fp_animation[725] = 724;
  td_to_fp_animation[723] = 722;
  td_to_fp_animation[721] = 720;
  td_to_fp_animation[729] = 728;
  td_to_fp_animation[91] = 90;
  fp_to_td_animation[236] = 237;
  fp_to_td_animation[228] = 229;
  fp_to_td_animation[230] = 231;
  fp_to_td_animation[232] = 233;
  fp_to_td_animation[234] = 235;
  fp_to_td_animation[244] = 245;
  fp_to_td_animation[242] = 243;
  fp_to_td_animation[240] = 241;
  fp_to_td_animation[238] = 239;
  fp_to_td_animation[246] = 247;
  fp_to_td_animation[60] = 61;
  td_to_fp_animation[237] = 236;
  td_to_fp_animation[229] = 228;
  td_to_fp_animation[227] = 226;
  td_to_fp_animation[231] = 230;
  td_to_fp_animation[233] = 232;
  td_to_fp_animation[235] = 234;
  td_to_fp_animation[245] = 244;
  td_to_fp_animation[243] = 242;
  td_to_fp_animation[241] = 240;
  td_to_fp_animation[239] = 238;
  td_to_fp_animation[247] = 246;
  td_to_fp_animation[61] = 60;
  fp_to_td_animation[170] = 171;
  fp_to_td_animation[162] = 163;
  fp_to_td_animation[160] = 161;
  fp_to_td_animation[164] = 165;
  fp_to_td_animation[166] = 167;
  fp_to_td_animation[168] = 169;
  fp_to_td_animation[178] = 179;
  fp_to_td_animation[176] = 177;
  fp_to_td_animation[174] = 175;
  fp_to_td_animation[172] = 173;
  fp_to_td_animation[180] = 181;
  td_to_fp_animation[171] = 170;
  td_to_fp_animation[163] = 162;
  td_to_fp_animation[161] = 160;
  td_to_fp_animation[165] = 164;
  td_to_fp_animation[167] = 166;
  td_to_fp_animation[169] = 168;
  td_to_fp_animation[179] = 178;
  td_to_fp_animation[177] = 176;
  td_to_fp_animation[175] = 174;
  td_to_fp_animation[173] = 172;
  td_to_fp_animation[181] = 180;
  fp_to_td_animation[280] = 281;
  fp_to_td_animation[270] = 271;
  fp_to_td_animation[272] = 273;
  fp_to_td_animation[274] = 275;
  fp_to_td_animation[276] = 277;
  fp_to_td_animation[288] = 289;
  fp_to_td_animation[286] = 287;
  fp_to_td_animation[284] = 285;
  fp_to_td_animation[282] = 283;
  fp_to_td_animation[278] = 279;
  fp_to_td_animation[290] = 291;
  fp_to_td_animation[58] = 59;
  td_to_fp_animation[281] = 280;
  td_to_fp_animation[271] = 270;
  td_to_fp_animation[273] = 272;
  td_to_fp_animation[275] = 274;
  td_to_fp_animation[277] = 276;
  td_to_fp_animation[289] = 288;
  td_to_fp_animation[287] = 286;
  td_to_fp_animation[285] = 284;
  td_to_fp_animation[283] = 282;
  td_to_fp_animation[279] = 278;
  td_to_fp_animation[291] = 290;
  td_to_fp_animation[59] = 58;
  fp_to_td_animation[214] = 215;
  fp_to_td_animation[206] = 207;
  fp_to_td_animation[204] = 205;
  fp_to_td_animation[208] = 209;
  fp_to_td_animation[210] = 211;
  fp_to_td_animation[212] = 213;
  fp_to_td_animation[222] = 223;
  fp_to_td_animation[220] = 221;
  fp_to_td_animation[218] = 219;
  fp_to_td_animation[216] = 217;
  fp_to_td_animation[224] = 225;
  td_to_fp_animation[215] = 214;
  td_to_fp_animation[207] = 206;
  td_to_fp_animation[205] = 204;
  td_to_fp_animation[209] = 208;
  td_to_fp_animation[211] = 210;
  td_to_fp_animation[213] = 212;
  td_to_fp_animation[223] = 222;
  td_to_fp_animation[221] = 220;
  td_to_fp_animation[219] = 218;
  td_to_fp_animation[217] = 216;
  td_to_fp_animation[225] = 224;
  fp_to_td_animation[368] = 369;
  fp_to_td_animation[360] = 361;
  fp_to_td_animation[358] = 359;
  fp_to_td_animation[362] = 363;
  fp_to_td_animation[364] = 365;
  fp_to_td_animation[366] = 367;
  fp_to_td_animation[376] = 377;
  fp_to_td_animation[374] = 375;
  fp_to_td_animation[372] = 373;
  fp_to_td_animation[370] = 371;
  fp_to_td_animation[378] = 379;
  td_to_fp_animation[369] = 368;
  td_to_fp_animation[361] = 360;
  td_to_fp_animation[359] = 358;
  td_to_fp_animation[363] = 362;
  td_to_fp_animation[365] = 364;
  td_to_fp_animation[367] = 366;
  td_to_fp_animation[377] = 376;
  td_to_fp_animation[375] = 374;
  td_to_fp_animation[373] = 372;
  td_to_fp_animation[371] = 370;
  td_to_fp_animation[379] = 378;
  fp_to_td_animation[346] = 347;
  fp_to_td_animation[338] = 339;
  fp_to_td_animation[336] = 337;
  fp_to_td_animation[340] = 341;
  fp_to_td_animation[342] = 343;
  fp_to_td_animation[344] = 345;
  fp_to_td_animation[354] = 355;
  fp_to_td_animation[352] = 353;
  fp_to_td_animation[350] = 351;
  fp_to_td_animation[348] = 349;
  fp_to_td_animation[356] = 357;
  td_to_fp_animation[347] = 346;
  td_to_fp_animation[339] = 338;
  td_to_fp_animation[337] = 336;
  td_to_fp_animation[341] = 340;
  td_to_fp_animation[343] = 342;
  td_to_fp_animation[345] = 344;
  td_to_fp_animation[355] = 354;
  td_to_fp_animation[353] = 352;
  td_to_fp_animation[351] = 350;
  td_to_fp_animation[349] = 348;
  td_to_fp_animation[357] = 356;
  fp_to_td_animation[302] = 303;
  fp_to_td_animation[294] = 295;
  fp_to_td_animation[292] = 293;
  fp_to_td_animation[296] = 297;
  fp_to_td_animation[298] = 299;
  fp_to_td_animation[300] = 301;
  fp_to_td_animation[310] = 311;
  fp_to_td_animation[308] = 309;
  fp_to_td_animation[306] = 307;
  fp_to_td_animation[304] = 305;
  fp_to_td_animation[312] = 313;
  td_to_fp_animation[303] = 302;
  td_to_fp_animation[295] = 294;
  td_to_fp_animation[293] = 292;
  td_to_fp_animation[297] = 296;
  td_to_fp_animation[299] = 298;
  td_to_fp_animation[301] = 300;
  td_to_fp_animation[311] = 310;
  td_to_fp_animation[309] = 308;
  td_to_fp_animation[307] = 306;
  td_to_fp_animation[305] = 304;
  td_to_fp_animation[313] = 312;
  fp_to_td_animation[390] = 391;
  fp_to_td_animation[382] = 383;
  fp_to_td_animation[380] = 381;
  fp_to_td_animation[384] = 385;
  fp_to_td_animation[386] = 387;
  fp_to_td_animation[388] = 389;
  fp_to_td_animation[398] = 399;
  fp_to_td_animation[396] = 397;
  fp_to_td_animation[394] = 395;
  fp_to_td_animation[392] = 393;
  fp_to_td_animation[400] = 401;
  fp_to_td_animation[54] = 55;
  td_to_fp_animation[391] = 390;
  td_to_fp_animation[383] = 382;
  td_to_fp_animation[381] = 380;
  td_to_fp_animation[385] = 384;
  td_to_fp_animation[387] = 386;
  td_to_fp_animation[389] = 388;
  td_to_fp_animation[399] = 398;
  td_to_fp_animation[397] = 396;
  td_to_fp_animation[395] = 394;
  td_to_fp_animation[393] = 392;
  td_to_fp_animation[401] = 400;
  td_to_fp_animation[55] = 54;
  fp_to_td_animation[456] = 457;
  fp_to_td_animation[448] = 449;
  fp_to_td_animation[446] = 447;
  fp_to_td_animation[450] = 451;
  fp_to_td_animation[452] = 453;
  fp_to_td_animation[454] = 455;
  fp_to_td_animation[464] = 465;
  fp_to_td_animation[462] = 463;
  fp_to_td_animation[460] = 461;
  fp_to_td_animation[458] = 459;
  fp_to_td_animation[466] = 467;
  fp_to_td_animation[68] = 69;
  td_to_fp_animation[457] = 456;
  td_to_fp_animation[449] = 448;
  td_to_fp_animation[447] = 446;
  td_to_fp_animation[451] = 450;
  td_to_fp_animation[453] = 452;
  td_to_fp_animation[455] = 454;
  td_to_fp_animation[465] = 464;
  td_to_fp_animation[463] = 462;
  td_to_fp_animation[461] = 460;
  td_to_fp_animation[459] = 458;
  td_to_fp_animation[467] = 466;
  td_to_fp_animation[69] = 68;
  fp_to_td_animation[688] = 689;
  fp_to_td_animation[686] = 687;
  fp_to_td_animation[690] = 691;
  fp_to_td_animation[692] = 693;
  fp_to_td_animation[694] = 695;
  fp_to_td_animation[696] = 697;
  fp_to_td_animation[704] = 705;
  fp_to_td_animation[702] = 703;
  fp_to_td_animation[700] = 701;
  fp_to_td_animation[698] = 699;
  fp_to_td_animation[706] = 707;
  fp_to_td_animation[88] = 89;
  td_to_fp_animation[689] = 688;
  td_to_fp_animation[687] = 686;
  td_to_fp_animation[691] = 690;
  td_to_fp_animation[693] = 692;
  td_to_fp_animation[695] = 694;
  td_to_fp_animation[697] = 696;
  td_to_fp_animation[705] = 704;
  td_to_fp_animation[703] = 702;
  td_to_fp_animation[701] = 700;
  td_to_fp_animation[699] = 698;
  td_to_fp_animation[707] = 706;
  td_to_fp_animation[89] = 88;
  fp_to_td_animation[468] = 469;
  fp_to_td_animation[470] = 471;
  fp_to_td_animation[472] = 473;
  fp_to_td_animation[474] = 475;
  fp_to_td_animation[476] = 477;
  fp_to_td_animation[478] = 479;
  fp_to_td_animation[480] = 481;
  fp_to_td_animation[482] = 483;
  fp_to_td_animation[484] = 485;
  fp_to_td_animation[486] = 487;
  fp_to_td_animation[70] = 71;
  td_to_fp_animation[469] = 468;
  td_to_fp_animation[471] = 470;
  td_to_fp_animation[473] = 472;
  td_to_fp_animation[475] = 474;
  td_to_fp_animation[477] = 476;
  td_to_fp_animation[479] = 478;
  td_to_fp_animation[481] = 480;
  td_to_fp_animation[483] = 482;
  td_to_fp_animation[485] = 484;
  td_to_fp_animation[487] = 486;
  td_to_fp_animation[71] = 70;
  fp_to_td_animation[754] = 755;
  fp_to_td_animation[752] = 753;
  fp_to_td_animation[756] = 757;
  fp_to_td_animation[758] = 759;
  fp_to_td_animation[760] = 761;
  fp_to_td_animation[762] = 763;
  fp_to_td_animation[770] = 771;
  fp_to_td_animation[768] = 769;
  fp_to_td_animation[766] = 767;
  fp_to_td_animation[764] = 765;
  fp_to_td_animation[772] = 773;
  fp_to_td_animation[94] = 95;
  td_to_fp_animation[755] = 754;
  td_to_fp_animation[753] = 752;
  td_to_fp_animation[757] = 756;
  td_to_fp_animation[759] = 758;
  td_to_fp_animation[761] = 760;
  td_to_fp_animation[763] = 762;
  td_to_fp_animation[771] = 770;
  td_to_fp_animation[769] = 768;
  td_to_fp_animation[767] = 766;
  td_to_fp_animation[765] = 764;
  td_to_fp_animation[773] = 772;
  td_to_fp_animation[95] = 94;
  fp_to_td_animation[580] = 581;
  fp_to_td_animation[578] = 579;
  fp_to_td_animation[582] = 583;
  fp_to_td_animation[584] = 585;
  fp_to_td_animation[586] = 587;
  fp_to_td_animation[588] = 589;
  fp_to_td_animation[596] = 597;
  fp_to_td_animation[594] = 595;
  fp_to_td_animation[592] = 593;
  fp_to_td_animation[590] = 591;
  fp_to_td_animation[598] = 599;
  fp_to_td_animation[78] = 79;
  td_to_fp_animation[581] = 580;
  td_to_fp_animation[579] = 578;
  td_to_fp_animation[583] = 582;
  td_to_fp_animation[585] = 584;
  td_to_fp_animation[587] = 586;
  td_to_fp_animation[589] = 588;
  td_to_fp_animation[597] = 596;
  td_to_fp_animation[595] = 594;
  td_to_fp_animation[593] = 592;
  td_to_fp_animation[591] = 590;
  td_to_fp_animation[599] = 598;
  td_to_fp_animation[79] = 78;
  fp_to_td_animation[644] = 645;
  fp_to_td_animation[646] = 647;
  fp_to_td_animation[648] = 649;
  fp_to_td_animation[650] = 651;
  fp_to_td_animation[652] = 653;
  fp_to_td_animation[658] = 659;
  fp_to_td_animation[660] = 661;
  fp_to_td_animation[656] = 657;
  fp_to_td_animation[654] = 655;
  fp_to_td_animation[662] = 663;
  fp_to_td_animation[84] = 85;
  td_to_fp_animation[645] = 644;
  td_to_fp_animation[647] = 646;
  td_to_fp_animation[649] = 648;
  td_to_fp_animation[651] = 650;
  td_to_fp_animation[653] = 652;
  td_to_fp_animation[659] = 658;
  td_to_fp_animation[661] = 660;
  td_to_fp_animation[657] = 656;
  td_to_fp_animation[655] = 654;
  td_to_fp_animation[663] = 662;
  td_to_fp_animation[85] = 84;
  fp_to_td_animation[532] = 533;
  fp_to_td_animation[534] = 535;
  fp_to_td_animation[536] = 537;
  fp_to_td_animation[538] = 539;
  fp_to_td_animation[540] = 541;
  fp_to_td_animation[542] = 543;
  fp_to_td_animation[550] = 551;
  fp_to_td_animation[548] = 549;
  fp_to_td_animation[546] = 547;
  fp_to_td_animation[544] = 545;
  fp_to_td_animation[552] = 553;
  fp_to_td_animation[76] = 77;
  td_to_fp_animation[533] = 532;
  td_to_fp_animation[535] = 534;
  td_to_fp_animation[537] = 536;
  td_to_fp_animation[539] = 538;
  td_to_fp_animation[541] = 540;
  td_to_fp_animation[543] = 542;
  td_to_fp_animation[551] = 550;
  td_to_fp_animation[549] = 548;
  td_to_fp_animation[547] = 546;
  td_to_fp_animation[545] = 544;
  td_to_fp_animation[553] = 552;
  td_to_fp_animation[77] = 76;
  fp_to_td_animation[666] = 667;
  fp_to_td_animation[668] = 669;
  fp_to_td_animation[664] = 665;
  fp_to_td_animation[670] = 671;
  fp_to_td_animation[674] = 675;
  fp_to_td_animation[672] = 673;
  fp_to_td_animation[682] = 683;
  fp_to_td_animation[680] = 681;
  fp_to_td_animation[678] = 679;
  fp_to_td_animation[676] = 677;
  fp_to_td_animation[684] = 685;
  fp_to_td_animation[86] = 87;
  td_to_fp_animation[667] = 666;
  td_to_fp_animation[669] = 668;
  td_to_fp_animation[665] = 664;
  td_to_fp_animation[671] = 670;
  td_to_fp_animation[675] = 674;
  td_to_fp_animation[673] = 672;
  td_to_fp_animation[683] = 682;
  td_to_fp_animation[681] = 680;
  td_to_fp_animation[679] = 678;
  td_to_fp_animation[677] = 676;
  td_to_fp_animation[685] = 684;
  td_to_fp_animation[87] = 86;
  fp_to_td_animation[424] = 425;
  fp_to_td_animation[426] = 427;
  fp_to_td_animation[428] = 429;
  fp_to_td_animation[430] = 431;
  fp_to_td_animation[432] = 433;
  fp_to_td_animation[434] = 435;
  fp_to_td_animation[436] = 437;
  fp_to_td_animation[438] = 439;
  fp_to_td_animation[440] = 441;
  fp_to_td_animation[442] = 443;
  fp_to_td_animation[444] = 445;
  fp_to_td_animation[52] = 53;
  td_to_fp_animation[425] = 424;
  td_to_fp_animation[427] = 426;
  td_to_fp_animation[429] = 428;
  td_to_fp_animation[431] = 430;
  td_to_fp_animation[433] = 432;
  td_to_fp_animation[435] = 434;
  td_to_fp_animation[437] = 436;
  td_to_fp_animation[439] = 438;
  td_to_fp_animation[441] = 440;
  td_to_fp_animation[443] = 442;
  td_to_fp_animation[445] = 444;
  td_to_fp_animation[53] = 52;
  fp_to_td_animation[512] = 513;
  fp_to_td_animation[510] = 511;
  fp_to_td_animation[514] = 515;
  fp_to_td_animation[516] = 517;
  fp_to_td_animation[518] = 519;
  fp_to_td_animation[520] = 521;
  fp_to_td_animation[522] = 523;
  fp_to_td_animation[524] = 525;
  fp_to_td_animation[528] = 529;
  fp_to_td_animation[526] = 527;
  fp_to_td_animation[530] = 531;
  fp_to_td_animation[50] = 51;
  td_to_fp_animation[513] = 512;
  td_to_fp_animation[511] = 510;
  td_to_fp_animation[515] = 514;
  td_to_fp_animation[517] = 516;
  td_to_fp_animation[519] = 518;
  td_to_fp_animation[521] = 520;
  td_to_fp_animation[523] = 522;
  td_to_fp_animation[525] = 524;
  td_to_fp_animation[529] = 528;
  td_to_fp_animation[527] = 526;
  td_to_fp_animation[531] = 530;
  td_to_fp_animation[51] = 50;
  fp_to_td_animation[2] = 3;
  fp_to_td_animation[0] = 1;
  fp_to_td_animation[4] = 5;
  fp_to_td_animation[6] = 7;
  fp_to_td_animation[8] = 9;
  fp_to_td_animation[10] = 11;
  fp_to_td_animation[12] = 13;
  fp_to_td_animation[14] = 15;
  fp_to_td_animation[16] = 17;
  fp_to_td_animation[18] = 19;
  fp_to_td_animation[22] = 23;
  fp_to_td_animation[20] = 21;
  fp_to_td_animation[72] = 73;
  td_to_fp_animation[3] = 2;
  td_to_fp_animation[1] = 0;
  td_to_fp_animation[5] = 4;
  td_to_fp_animation[7] = 6;
  td_to_fp_animation[9] = 8;
  td_to_fp_animation[11] = 10;
  td_to_fp_animation[13] = 12;
  td_to_fp_animation[15] = 14;
  td_to_fp_animation[19] = 18;
  td_to_fp_animation[17] = 16;
  td_to_fp_animation[23] = 22;
  td_to_fp_animation[21] = 20;
  td_to_fp_animation[73] = 72;
  fp_to_td_animation[314] = 315;
  fp_to_td_animation[316] = 317;
  fp_to_td_animation[318] = 319;
  fp_to_td_animation[320] = 321;
  fp_to_td_animation[322] = 323;
  fp_to_td_animation[324] = 325;
  fp_to_td_animation[326] = 327;
  fp_to_td_animation[328] = 329;
  fp_to_td_animation[330] = 331;
  fp_to_td_animation[332] = 333;
  fp_to_td_animation[334] = 335;
  td_to_fp_animation[315] = 314;
  td_to_fp_animation[317] = 316;
  td_to_fp_animation[319] = 318;
  td_to_fp_animation[321] = 320;
  td_to_fp_animation[323] = 322;
  td_to_fp_animation[325] = 324;
  td_to_fp_animation[327] = 326;
  td_to_fp_animation[329] = 328;
  td_to_fp_animation[331] = 330;
  td_to_fp_animation[333] = 332;
  td_to_fp_animation[335] = 334;
  fp_to_td_animation[602] = 603;
  fp_to_td_animation[600] = 601;
  fp_to_td_animation[604] = 605;
  fp_to_td_animation[606] = 607;
  fp_to_td_animation[608] = 609;
  fp_to_td_animation[610] = 611;
  fp_to_td_animation[618] = 619;
  fp_to_td_animation[616] = 617;
  fp_to_td_animation[614] = 615;
  fp_to_td_animation[612] = 613;
  fp_to_td_animation[620] = 621;
  fp_to_td_animation[80] = 81;
  td_to_fp_animation[603] = 602;
  td_to_fp_animation[601] = 600;
  td_to_fp_animation[605] = 604;
  td_to_fp_animation[607] = 606;
  td_to_fp_animation[609] = 608;
  td_to_fp_animation[611] = 610;
  td_to_fp_animation[619] = 618;
  td_to_fp_animation[617] = 616;
  td_to_fp_animation[615] = 614;
  td_to_fp_animation[613] = 612;
  td_to_fp_animation[621] = 620;
  td_to_fp_animation[81] = 80;
  fp_to_td_animation[624] = 625;
  fp_to_td_animation[622] = 623;
  fp_to_td_animation[626] = 627;
  fp_to_td_animation[628] = 629;
  fp_to_td_animation[630] = 631;
  fp_to_td_animation[632] = 633;
  fp_to_td_animation[640] = 641;
  fp_to_td_animation[638] = 639;
  fp_to_td_animation[636] = 637;
  fp_to_td_animation[634] = 635;
  fp_to_td_animation[642] = 643;
  fp_to_td_animation[82] = 83;
  td_to_fp_animation[625] = 624;
  td_to_fp_animation[623] = 622;
  td_to_fp_animation[627] = 626;
  td_to_fp_animation[629] = 628;
  td_to_fp_animation[631] = 630;
  td_to_fp_animation[633] = 632;
  td_to_fp_animation[641] = 640;
  td_to_fp_animation[639] = 638;
  td_to_fp_animation[637] = 636;
  td_to_fp_animation[635] = 634;
  td_to_fp_animation[643] = 642;
  td_to_fp_animation[83] = 82;
  fp_to_td_animation[490] = 491;
  fp_to_td_animation[488] = 489;
  fp_to_td_animation[492] = 493;
  fp_to_td_animation[494] = 495;
  fp_to_td_animation[496] = 497;
  fp_to_td_animation[498] = 499;
  fp_to_td_animation[506] = 507;
  fp_to_td_animation[504] = 505;
  fp_to_td_animation[502] = 503;
  fp_to_td_animation[500] = 501;
  fp_to_td_animation[508] = 509;
  fp_to_td_animation[74] = 75;
  td_to_fp_animation[491] = 490;
  td_to_fp_animation[489] = 488;
  td_to_fp_animation[493] = 492;
  td_to_fp_animation[495] = 494;
  td_to_fp_animation[497] = 496;
  td_to_fp_animation[499] = 498;
  td_to_fp_animation[507] = 506;
  td_to_fp_animation[505] = 504;
  td_to_fp_animation[503] = 502;
  td_to_fp_animation[501] = 500;
  td_to_fp_animation[509] = 508;
  td_to_fp_animation[75] = 74;
  fp_to_td_animation[184] = 185;
  fp_to_td_animation[182] = 183;
  fp_to_td_animation[186] = 187;
  fp_to_td_animation[188] = 189;
  fp_to_td_animation[190] = 191;
  fp_to_td_animation[192] = 193;
  fp_to_td_animation[200] = 201;
  fp_to_td_animation[198] = 199;
  fp_to_td_animation[196] = 197;
  fp_to_td_animation[194] = 195;
  fp_to_td_animation[202] = 203;
  fp_to_td_animation[66] = 67;
  td_to_fp_animation[185] = 184;
  td_to_fp_animation[183] = 182;
  td_to_fp_animation[187] = 186;
  td_to_fp_animation[189] = 188;
  td_to_fp_animation[191] = 190;
  td_to_fp_animation[193] = 192;
  td_to_fp_animation[201] = 200;
  td_to_fp_animation[199] = 198;
  td_to_fp_animation[197] = 196;
  td_to_fp_animation[195] = 194;
  td_to_fp_animation[203] = 202;
  td_to_fp_animation[67] = 66;
  fp_to_td_animation[554] = 555;
  fp_to_td_animation[556] = 557;
  fp_to_td_animation[558] = 559;
  fp_to_td_animation[560] = 561;
  fp_to_td_animation[562] = 563;
  fp_to_td_animation[564] = 565;
  fp_to_td_animation[566] = 567;
  fp_to_td_animation[568] = 569;
  fp_to_td_animation[570] = 571;
  fp_to_td_animation[572] = 573;
  fp_to_td_animation[574] = 575;
  fp_to_td_animation[576] = 577;
  fp_to_td_animation[56] = 57;
  td_to_fp_animation[555] = 554;
  td_to_fp_animation[557] = 556;
  td_to_fp_animation[559] = 558;
  td_to_fp_animation[561] = 560;
  td_to_fp_animation[563] = 562;
  td_to_fp_animation[565] = 564;
  td_to_fp_animation[567] = 566;
  td_to_fp_animation[569] = 568;
  td_to_fp_animation[571] = 570;
  td_to_fp_animation[573] = 572;
  td_to_fp_animation[575] = 574;
  td_to_fp_animation[577] = 576;
  td_to_fp_animation[57] = 56;
  fp_to_td_animation[896] = 900;
  fp_to_td_animation[819] = 0x335;
  fp_to_td_animation[820] = 0x336;
  fp_to_td_animation[930] = 931;
  fp_to_td_animation[789] = 788;
  fp_to_td_animation[791] = 790;
  fp_to_td_animation[793] = 792;
  fp_to_td_animation[794] = 0x31A;
  fp_to_td_animation[795] = 0x31A;
  fp_to_td_animation[892] = 0x37B;
  fp_to_td_animation[893] = 0x381;
  fp_to_td_animation[894] = 0x382;
  fp_to_td_animation[895] = 0x383;
  fp_to_td_animation[804] = 803;
  fp_to_td_animation[806] = 805;
  fp_to_td_animation[808] = 807;
  fp_to_td_animation[810] = 809;
  fp_to_td_animation[812] = 811;
  fp_to_td_animation[814] = 813;
  fp_to_td_animation[816] = 815;
  fp_to_td_animation[818] = 817;
  fp_to_td_animation[128] = 129;
  fp_to_td_animation[132] = 133;
  fp_to_td_animation[134] = 135;
  fp_to_td_animation[136] = 137;
  fp_to_td_animation[138] = 139;
  fp_to_td_animation[140] = 141;
  fp_to_td_animation[142] = 143;
  fp_to_td_animation[144] = 145;
  fp_to_td_animation[146] = 147;
  fp_to_td_animation[148] = 149;
  fp_to_td_animation[150] = 151;
  fp_to_td_animation[152] = 153;
  fp_to_td_animation[124] = 125;
  fp_to_td_animation[154] = 155;
  fp_to_td_animation[156] = 157;
  fp_to_td_animation[158] = 159;
  fp_to_td_animation[126] = 127;
  fp_to_td_animation[226] = 227;
  fp_to_td_animation[776] = 0x30B;
  fp_to_td_animation[777] = 0x30A;
  fp_to_td_animation[962] = 963;
  fp_to_td_animation[950] = 951;
  fp_to_td_animation[905] = 906;
  fp_to_td_animation[932] = 933;
  fp_to_td_animation[934] = 935;
  fp_to_td_animation[948] = 949;
  fp_to_td_animation[903] = 904;
  fp_to_td_animation[840] = 0x34A;
  fp_to_td_animation[841] = 0x34B;
  fp_to_td_animation[844] = 0x34F;
  fp_to_td_animation[845] = 0x350;
  fp_to_td_animation[846] = 849;
  fp_to_td_animation[836] = 0x347;
  fp_to_td_animation[837] = 0x346;
  fp_to_td_animation[825] = 826;
  fp_to_td_animation[861] = 862;
  fp_to_td_animation[936] = 0x3AD;
  fp_to_td_animation[937] = 0x3AE;
  fp_to_td_animation[938] = 0x3AF;
  fp_to_td_animation[939] = 0x3B0;
  fp_to_td_animation[940] = 945;
  fp_to_td_animation[774] = 775;
  fp_to_td_animation[922] = 923;
  fp_to_td_animation[867] = 0x366;
  fp_to_td_animation[868] = 0x367;
  fp_to_td_animation[869] = 872;
  fp_to_td_animation[873] = 0x36C;
  fp_to_td_animation[874] = 0x36D;
  fp_to_td_animation[875] = 878;
  fp_to_td_animation[879] = 0x372;
  fp_to_td_animation[880] = 0x373;
  fp_to_td_animation[881] = 884;
  fp_to_td_animation[885] = 0x378;
  fp_to_td_animation[886] = 0x379;
  fp_to_td_animation[887] = 890;
  fp_to_td_animation[901] = 902;
  fp_to_td_animation[130] = 131;
  td_to_fp_animation[897] = 893;
  td_to_fp_animation[900] = 896;
  td_to_fp_animation[821] = 0x333;
  td_to_fp_animation[822] = 0x334;
  td_to_fp_animation[931] = 930;
  td_to_fp_animation[788] = 789;
  td_to_fp_animation[790] = 791;
  td_to_fp_animation[792] = 793;
  td_to_fp_animation[794] = 795;
  td_to_fp_animation[803] = 804;
  td_to_fp_animation[805] = 806;
  td_to_fp_animation[807] = 808;
  td_to_fp_animation[809] = 810;
  td_to_fp_animation[811] = 812;
  td_to_fp_animation[813] = 814;
  td_to_fp_animation[815] = 816;
  td_to_fp_animation[817] = 818;
  td_to_fp_animation[129] = 128;
  td_to_fp_animation[133] = 132;
  td_to_fp_animation[135] = 134;
  td_to_fp_animation[137] = 136;
  td_to_fp_animation[139] = 138;
  td_to_fp_animation[141] = 140;
  td_to_fp_animation[143] = 142;
  td_to_fp_animation[145] = 144;
  td_to_fp_animation[147] = 146;
  td_to_fp_animation[149] = 148;
  td_to_fp_animation[151] = 150;
  td_to_fp_animation[153] = 152;
  td_to_fp_animation[125] = 124;
  td_to_fp_animation[155] = 154;
  td_to_fp_animation[157] = 156;
  td_to_fp_animation[159] = 158;
  td_to_fp_animation[963] = 962;
  td_to_fp_animation[951] = 950;
  td_to_fp_animation[906] = 905;
  td_to_fp_animation[933] = 932;
  td_to_fp_animation[935] = 934;
  td_to_fp_animation[949] = 948;
  td_to_fp_animation[904] = 903;
  td_to_fp_animation[842] = 0x348;
  td_to_fp_animation[843] = 0x349;
  td_to_fp_animation[847] = 0x34C;
  td_to_fp_animation[848] = 0x34D;
  td_to_fp_animation[849] = 846;
  td_to_fp_animation[838] = 837;
  td_to_fp_animation[826] = 825;
  td_to_fp_animation[862] = 861;
  td_to_fp_animation[941] = 0x3A8;
  td_to_fp_animation[942] = 0x3A9;
  td_to_fp_animation[943] = 0x3AA;
  td_to_fp_animation[944] = 0x3AB;
  td_to_fp_animation[898] = 0x382;
  td_to_fp_animation[899] = 0x37F;
  td_to_fp_animation[945] = 940;
  td_to_fp_animation[775] = 774;
  td_to_fp_animation[923] = 922;
  td_to_fp_animation[870] = 0x363;
  td_to_fp_animation[871] = 0x364;
  td_to_fp_animation[872] = 869;
  td_to_fp_animation[876] = 0x369;
  td_to_fp_animation[877] = 0x36A;
  td_to_fp_animation[878] = 875;
  td_to_fp_animation[882] = 0x36F;
  td_to_fp_animation[883] = 0x370;
  td_to_fp_animation[884] = 881;
  td_to_fp_animation[888] = 0x375;
  td_to_fp_animation[889] = 0x376;
  td_to_fp_animation[890] = 0x377;
  td_to_fp_animation[891] = 0x37C;
  td_to_fp_animation[778] = 0x309;
  td_to_fp_animation[779] = 0x308;
  td_to_fp_animation[902] = 901;
  td_to_fp_animation[131] = 130;
  fp_to_td_animation[120] = 121;
  td_to_fp_animation[121] = 120;
  fp_to_td_animation[114] = 115;
  td_to_fp_animation[115] = 114;
  fp_to_td_animation[110] = 111;
  td_to_fp_animation[111] = 110;
  fp_to_td_animation[102] = 103;
  td_to_fp_animation[103] = 102;
  fp_to_td_animation[104] = 105;
  td_to_fp_animation[105] = 104;
  fp_to_td_animation[106] = 107;
  td_to_fp_animation[107] = 106;
  fp_to_td_animation[108] = 109;
  td_to_fp_animation[109] = 108;
  fp_to_td_animation[100] = 101;
  td_to_fp_animation[101] = 100;
  fp_to_td_animation[98] = 99;
  td_to_fp_animation[99] = 98;
  fp_to_td_animation[46] = 47;
  td_to_fp_animation[47] = 46;
  fp_to_td_animation[952] = 953;
  td_to_fp_animation[953] = 952;
  fp_to_td_animation[954] = 955;
  td_to_fp_animation[955] = 954;
  fp_to_td_animation[956] = 957;
  td_to_fp_animation[957] = 956;
  fp_to_td_animation[958] = 959;
  td_to_fp_animation[959] = 958;
  fp_to_td_animation[960] = 961;
  td_to_fp_animation[961] = 960;
  fp_to_td_animation[946] = 947;
  td_to_fp_animation[947] = 946;
}

/**
 * Fills randomisors array used for mesh deformations.
 */
void setup_mesh_randomizers(void)
{
    uint32_t seed;
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
        floor_height_table[n] = shade_back;
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
            memset(wibl, 0, sizeof(struct WibbleTable));
            wibl++;
            memset(empty_wibl, 0, sizeof(struct WibbleTable));
            empty_wibl++;
        }
    }
    // Set wibble values using special random algorithm
    seed = 0;
    for (i=0; i < 32; i++)
    {
        wibl = &wibble_table[i+32];
        n = wibble_random(65447,&seed);
        wibl->offset_x = (n % 127) - 63;
        n = wibble_random(65447,&seed);
        wibl->offset_y = ((n % 127) - 63) / 3;
        n = wibble_random(65447,&seed);
        wibl->offset_z = (n % 127) - 63;
        qwibl = &wibble_table[i+64];
        n = wibble_random(65447,&seed);
        wibl->lightness_offset = (n % 2047) - 1023;
        n = wibble_random(65447,&seed);
        qwibl->offset_x = (n % 127) - 63;
        n = wibble_random(65447,&seed);
        qwibl->offset_z = (n % 127) - 63;
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
    fname = prepare_file_path(FGrp_StdData,"ceiling.txt");
    fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (!fh) {
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
        memset(numstr, 0, sizeof(numstr));
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
    return true;
}

/******************************************************************************/
