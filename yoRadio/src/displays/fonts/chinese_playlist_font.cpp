#include "../../core/options.h"

#if CJK_SUBSET_FONT
#include "chinese_playlist_font.h"

int16_t cjkIndexForCodepoint(uint16_t codepoint) {
  int16_t low = 0;
  int16_t high = CJK_GLYPH_COUNT - 1;
  while (low <= high) {
    const int16_t middle = low + (high - low) / 2;
    const uint16_t candidate = pgm_read_word(&cjkCodepoints[middle]);
    if (candidate == codepoint) return middle;
    if (candidate < codepoint) low = middle + 1;
    else high = middle - 1;
  }
  return -1;
}

int16_t cjkIndexForEncodedByte(uint8_t encoded) {
  for (uint16_t index = 0; index < CJK_GLYPH_COUNT; ++index) {
    if (pgm_read_byte(&cjkEncodedBytes[index]) == encoded) return index;
  }
  return -1;
}

uint8_t cjkEncodeCodepoint(uint16_t codepoint) {
  const int16_t index = cjkIndexForCodepoint(codepoint);
  return index < 0 ? 0 : pgm_read_byte(&cjkEncodedBytes[index]);
}

#endif
