#ifndef utf8Rus_h
#define  utf8Rus_h

size_t strlen_utf8(const char* s);
char* utf8Rus(const char* str, bool uppercase);
uint8_t displayEncodedCharWidth(uint8_t encoded, uint8_t textsize);
uint16_t displayEncodedTextWidth(const char* text, uint8_t textsize);

#endif
