/*************************************************************************************
    Hosyond E32R35T ST7796 480x320 landscape layout.

    This is based on yoRadio's standard ST7796 layout. The weather ticker has a
    three-second reading pause and scrolls at roughly 25 pixels per second
    instead of the upstream profile's roughly 175 pixels per second.
*************************************************************************************/

#ifndef displayST7796conf_custom_h
#define displayST7796conf_custom_h

#define DSP_WIDTH       480
#define DSP_HEIGHT      320
#define TFT_FRAMEWDT    10
#define MAX_WIDTH       DSP_WIDTH-TFT_FRAMEWDT*2

#if BITRATE_FULL
  #define TITLE_FIX 44
#else
  #define TITLE_FIX 0
#endif
#define bootLogoTop     110

/* SCROLLS */                            /* {{ left, top, fontsize, align }, buffsize, uppercase, width, scrolldelay, scrolldelta, scrolltime } */
const ScrollConfig metaConf       PROGMEM = {{ TFT_FRAMEWDT, TFT_FRAMEWDT, 4, WA_LEFT }, 140, true, MAX_WIDTH, 5000, 7, 40 };
const ScrollConfig title1Conf     PROGMEM = {{ TFT_FRAMEWDT, 62, 2, WA_LEFT }, 140, true, MAX_WIDTH-(TITLE_FIX==0?6*2*7-6:TITLE_FIX), 5000, 7, 40 };
const ScrollConfig title2Conf     PROGMEM = {{ TFT_FRAMEWDT, 86, 2, WA_LEFT }, 140, true, MAX_WIDTH-TITLE_FIX, 5000, 7, 40 };
const ScrollConfig playlistConf   PROGMEM = {{ TFT_FRAMEWDT, 146, 3, WA_LEFT }, 140, true, MAX_WIDTH, 1000, 7, 40 };
const ScrollConfig apTitleConf    PROGMEM = {{ TFT_FRAMEWDT, TFT_FRAMEWDT, 4, WA_CENTER }, 140, false, MAX_WIDTH, 0, 7, 40 };
const ScrollConfig apSettConf     PROGMEM = {{ TFT_FRAMEWDT, 320-TFT_FRAMEWDT-16, 2, WA_LEFT }, 140, false, MAX_WIDTH, 0, 7, 40 };
const ScrollConfig weatherConf    PROGMEM = {{ TFT_FRAMEWDT, 116, 2, WA_LEFT }, 254, false, MAX_WIDTH, 3000, 2, 80 };

/* BACKGROUNDS */
const FillConfig   metaBGConf     PROGMEM = {{ 0, 0, 0, WA_LEFT }, DSP_WIDTH, 50, false };
const FillConfig   metaBGConfInv  PROGMEM = {{ 0, 50, 0, WA_LEFT }, DSP_WIDTH, 2, false };
const FillConfig   volbarConf     PROGMEM = {{ TFT_FRAMEWDT, DSP_HEIGHT-TFT_FRAMEWDT-8, 0, WA_LEFT }, MAX_WIDTH, 8, true };
const FillConfig  playlBGConf     PROGMEM = {{ 0, 138, 0, WA_LEFT }, DSP_WIDTH, 36, false };
const FillConfig  heapbarConf     PROGMEM = {{ 0, DSP_HEIGHT-2, 0, WA_LEFT }, DSP_WIDTH, 2, false };

/* WIDGETS */
const WidgetConfig bootstrConf    PROGMEM = { 0, 243, 1, WA_CENTER };
const WidgetConfig bitrateConf    PROGMEM = { 6, 62, 2, WA_RIGHT };
const WidgetConfig voltxtConf     PROGMEM = { 0, DSP_HEIGHT-38, 2, WA_CENTER };
const WidgetConfig batteryConf    PROGMEM = { TFT_FRAMEWDT, DSP_HEIGHT-38, 2, WA_LEFT };
const WidgetConfig  iptxtConf     PROGMEM = { TFT_FRAMEWDT, DSP_HEIGHT-38, 2, WA_LEFT };
const WidgetConfig   rssiConf     PROGMEM = { TFT_FRAMEWDT, DSP_HEIGHT-38, 2, WA_RIGHT };
const WidgetConfig numConf        PROGMEM = { 0, 200, 0, WA_CENTER };
const WidgetConfig apNameConf     PROGMEM = { TFT_FRAMEWDT, 88, 3, WA_CENTER };
const WidgetConfig apName2Conf    PROGMEM = { TFT_FRAMEWDT, 120, 3, WA_CENTER };
const WidgetConfig apPassConf     PROGMEM = { TFT_FRAMEWDT, 173, 3, WA_CENTER };
const WidgetConfig apPass2Conf    PROGMEM = { TFT_FRAMEWDT, 205, 3, WA_CENTER };
const WidgetConfig  clockConf     PROGMEM = { TFT_FRAMEWDT*2, 230, 0, WA_RIGHT };
const WidgetConfig vuConf         PROGMEM = { TFT_FRAMEWDT, 136, 1, WA_LEFT };

const WidgetConfig bootWdtConf    PROGMEM = { 0, 216, 1, WA_CENTER };
const ProgressConfig bootPrgConf  PROGMEM = { 90, 14, 4 };
const BitrateConfig fullbitrateConf PROGMEM = {{DSP_WIDTH-TFT_FRAMEWDT-38, 59, 2, WA_LEFT}, 42 };

/* BANDS */
const VUBandsConfig bandsConf     PROGMEM = { 32, 130, 4, 2, 10, 3 };

/* STRINGS */
const char         numtxtFmt[]    PROGMEM = "%d";
const char           rssiFmt[]    PROGMEM = "WiFi %d";
const char          iptxtFmt[]    PROGMEM = "%s";
const char         voltxtFmt[]    PROGMEM = "VOL %d";
const char        bitrateFmt[]    PROGMEM = "%d kBs";

/* MOVES */
const MoveConfig    clockMove     PROGMEM = { 0, 176, -1 };
const MoveConfig   weatherMove    PROGMEM = { 8, 120, MAX_WIDTH };
const MoveConfig   weatherMoveVU  PROGMEM = { 89, 120, MAX_WIDTH-89+TFT_FRAMEWDT };

#endif
