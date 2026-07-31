#include "options.h"
#include "favorites.h"
#include "config.h"
#include <SPIFFS.h>

#define FAVORITES_PATH "/data/favorites.txt"

Favorites favorites;

void Favorites::_load() {
  if (_loaded) return;
  _loaded = true;
  _count = 0;
  File file = SPIFFS.open(FAVORITES_PATH, "r");
  while (file && file.available() && _count < MAX_FAVORITES) {
    String url = file.readStringUntil('\n');
    url.trim();
    if (url.length()) _urls[_count++] = url;
  }
  if (file) file.close();
}

void Favorites::_save() {
  File file = SPIFFS.open(FAVORITES_PATH, "w");
  if (!file) return;
  for (uint8_t index = 0; index < _count; ++index) file.println(_urls[index]);
  file.close();
}

void Favorites::_resolve() {
  _load();
  const uint16_t playlistLength = config.playlistLength();
  if (_resolvedPlaylistLength == playlistLength) return;
  _resolvedPlaylistLength = playlistLength;
  _resolvedCount = 0;
  for (uint16_t station = 1; station <= playlistLength && _resolvedCount < MAX_FAVORITES; ++station) {
    const String url = config.stationUrlByNum(station);
    for (uint8_t favorite = 0; favorite < _count; ++favorite) {
      if (_urls[favorite] == url) {
        _stationIds[_resolvedCount++] = station;
        break;
      }
    }
  }
}

bool Favorites::isFavorite(uint16_t stationId) {
  _load();
  const String url = config.stationUrlByNum(stationId);
  if (!url.length()) return false;
  for (uint8_t index = 0; index < _count; ++index) {
    if (_urls[index] == url) return true;
  }
  return false;
}

bool Favorites::toggle(uint16_t stationId) {
  _load();
  const String url = config.stationUrlByNum(stationId);
  if (!url.length()) return false;
  for (uint8_t index = 0; index < _count; ++index) {
    if (_urls[index] == url) {
      for (uint8_t move = index; move + 1 < _count; ++move) _urls[move] = _urls[move + 1];
      --_count;
      _resolvedPlaylistLength = 0xFFFF;
      _save();
      return false;
    }
  }
  if (_count < MAX_FAVORITES) {
    _urls[_count++] = url;
    _resolvedPlaylistLength = 0xFFFF;
    _save();
    return true;
  }
  return false;
}

uint16_t Favorites::count() {
  _resolve();
  return _resolvedCount;
}

uint16_t Favorites::stationAt(uint16_t position) {
  _resolve();
  return position >= 1 && position <= _resolvedCount ? _stationIds[position - 1] : 0;
}

uint16_t Favorites::positionOf(uint16_t stationId) {
  _resolve();
  for (uint8_t index = 0; index < _resolvedCount; ++index)
    if (_stationIds[index] == stationId) return index + 1;
  return 0;
}

uint16_t Favorites::nextStation(uint16_t currentStation, bool forward) {
  const uint16_t total = count();
  if (!total) return 0;
  uint16_t position = positionOf(currentStation);
  if (!position) position = 1;
  else if (forward) position = position >= total ? 1 : position + 1;
  else position = position <= 1 ? total : position - 1;
  return stationAt(position);
}
