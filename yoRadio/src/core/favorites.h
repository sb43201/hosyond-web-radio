#ifndef favorites_h
#define favorites_h

#include <Arduino.h>

class Favorites {
  public:
    bool isFavorite(uint16_t stationId);
    bool toggle(uint16_t stationId);
    uint16_t count();
    uint16_t stationAt(uint16_t position);
    uint16_t positionOf(uint16_t stationId);
    uint16_t nextStation(uint16_t currentStation, bool forward);
  private:
    static constexpr uint8_t MAX_FAVORITES = 64;
    uint32_t _urlHashes[MAX_FAVORITES];
    uint16_t _stationIds[MAX_FAVORITES];
    uint8_t _count = 0;
    uint8_t _resolvedCount = 0;
    uint16_t _resolvedPlaylistLength = 0xFFFF;
    bool _loaded = false;
    static uint32_t _hashUrl(const String &url);
    void _load();
    void _save();
    void _resolve();
};

extern Favorites favorites;

#endif
