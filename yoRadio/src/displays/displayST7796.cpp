#include "../core/options.h"
#if DSP_MODEL==DSP_ST7796
#include "dspcore.h"
#include "../core/config.h"
#if CJK_SUBSET_FONT
  #include "fonts/chinese_playlist_font.h"
#endif

#if DSP_HSPI
DspCore::DspCore(): Adafruit_ST7796S_kbv(&SPI2, TFT_DC, TFT_CS, TFT_RST) {}
#else
DspCore::DspCore(): Adafruit_ST7796S_kbv(TFT_CS, TFT_DC, TFT_RST) {}
#endif

void DspCore::initDisplay() {
  begin();
  invert();
  cp437(true);
  flip();
  setTextWrap(false);
  setTextSize(1);
  fillScreen(0x0000);
}

void DspCore::clearDsp(bool black){ fillScreen(black?0:config.theme.background); }
void DspCore::flip(){ setRotation(config.store.flipscreen?3:1); }
void DspCore::invert(){ invertDisplay(config.store.invertdisplay); }
void DspCore::sleep(void){ sendCommand(ST7796S_SLPIN); delay(150); sendCommand(ST7796S_DISPOFF); delay(150); }
void DspCore::wake(void){ sendCommand(ST7796S_DISPON); delay(150); sendCommand(ST7796S_SLPOUT); delay(150); }

#if CJK_SUBSET_FONT
size_t DspCore::write(uint8_t c) {
  const int16_t glyphIndex = cjkIndexForEncodedByte(c);
  if (glyphIndex < 0 || gfxFont != nullptr) return yoDisplay::write(c);

  const int16_t glyphWidth = 8 * textsize_x;
  const int16_t glyphHeight = 8 * textsize_y;
  if (wrap && cursor_x + glyphWidth > _width) {
    cursor_x = 0;
    cursor_y += glyphHeight;
  }

  startWrite();
  for (uint8_t row = 0; row < glyphHeight; ++row) {
    const uint8_t sourceRow = row * 12 / glyphHeight;
    const uint16_t bits = pgm_read_word(&cjkGlyphs[glyphIndex][sourceRow]);
    for (uint8_t column = 0; column < glyphWidth; ++column) {
      const uint8_t sourceColumn = column * 12 / glyphWidth;
      const bool foreground = bits & (0x0800 >> sourceColumn);
      if (foreground || textbgcolor != textcolor) {
        writePixel(cursor_x + column, cursor_y + row,
                   foreground ? textcolor : textbgcolor);
      }
    }
  }
  endWrite();
  cursor_x += glyphWidth;
  return 1;
}
#endif

#endif
