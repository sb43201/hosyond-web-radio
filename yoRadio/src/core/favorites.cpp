#include "options.h"
#include "favorites.h"
#include "config.h"
#include <SPIFFS.h>

#define FAVORITES_PATH "/data/favorites.txt"

Favorites favorites;

uint32_t Favorites::_hashUrl(const String &url) {
  // FNV-1a keeps favorite identity in four fixed bytes instead of retaining
  // heap-allocated Strings. This matters because audio decoders need a large
  // contiguous heap block when a stream reconnects.
  uint32_t hash = 2166136261UL;
  for (size_t index = 0; index < url.length(); ++index) {
    hash ^= static_cast<uint8_t>(url[index]);
    hash *= 16777619UL;
  }
  return hash;
}

void Favorites::_load() {
  if (_loaded) return;
  _loaded = true;
  _count = 0;
  File file = SPIFFS.open(FAVORITES_PATH, "r");
  while (file && file.available() && _count < MAX_FAVORITES) {
    String entry = file.readStringUntil('\n');
    entry.trim();
    if (!entry.length()) continue;

    // New files contain an eight-digit hash. Old URL-based files are read
    // transparently and converted the next time favorites are saved.
    bool isHash = entry.length() == 8;
    for (uint8_t index = 0; isHash && index < 8; ++index) {
      const char value = entry[index];
      isHash = (value >= '0' && value <= '9') ||
               (value >= 'a' && value <= 'f') ||
               (value >= 'A' && value <= 'F');
    }
    _urlHashes[_count++] = isHash
      ? static_cast<uint32_t>(strtoul(entry.c_str(), nullptr, 16))
      : _hashUrl(entry);
  }
  if (file) file.close();
}

void Favorites::_save() {
  File file = SPIFFS.open(FAVORITES_PATH, "w");
  if (!file) return;
  for (uint8_t index = 0; index < _count; ++index)
    file.printf("%08lx\n", static_cast<unsigned long>(_urlHashes[index]));
  file.close();
}

void Favorites::_resolve() {
  _load();
  const uint16_t playlistLength = config.playlistLength();
  if (_resolvedPlaylistLength == playlistLength) return;
  _resolvedPlaylistLength = playlistLength;
  _resolvedCount = 0;
  File playlist = config.SDPLFS()->open(REAL_PLAYL, "r");
  uint16_t station = 0;
  while (playlist && playlist.available() && station < playlistLength &&
         _resolvedCount < MAX_FAVORITES) {
    ++station;
    const String line = playlist.readStringUntil('\n');
    const int firstTab = line.indexOf('\t');
    const int secondTab = firstTab < 0 ? -1 : line.indexOf('\t', firstTab + 1);
    if (firstTab < 0) continue;
    const String url = line.substring(firstTab + 1, secondTab < 0 ? line.length() : secondTab);
    const uint32_t urlHash = _hashUrl(url);
    for (uint8_t favorite = 0; favorite < _count; ++favorite) {
      if (_urlHashes[favorite] == urlHash) {
        _stationIds[_resolvedCount++] = station;
        break;
      }
    }
  }
  if (playlist) playlist.close();
}

bool Favorites::isFavorite(uint16_t stationId) {
  _resolve();
  for (uint8_t index = 0; index < _resolvedCount; ++index)
    if (_stationIds[index] == stationId) return true;
  return false;
}

bool Favorites::toggle(uint16_t stationId) {
  _load();
  const String url = config.stationUrlByNum(stationId);
  if (!url.length()) return false;
  const uint32_t urlHash = _hashUrl(url);
  for (uint8_t index = 0; index < _count; ++index) {
    if (_urlHashes[index] == urlHash) {
      for (uint8_t move = index; move + 1 < _count; ++move)
        _urlHashes[move] = _urlHashes[move + 1];
      --_count;
      _resolvedPlaylistLength = 0xFFFF;
      _save();
      return false;
    }
  }
  if (_count < MAX_FAVORITES) {
    _urlHashes[_count++] = urlHash;
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
