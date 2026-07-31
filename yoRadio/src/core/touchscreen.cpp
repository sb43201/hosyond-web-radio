#include "options.h"
#if (TS_MODEL!=TS_MODEL_UNDEFINED) && (DSP_MODEL!=DSP_DUMMY)
#include "Arduino.h"
#include "touchscreen.h"
#include "config.h"
#include "controls.h"
#include "display.h"
#include "player.h"

#ifndef TS_X_MIN
  #define TS_X_MIN              400
#endif
#ifndef TS_X_MAX
  #define TS_X_MAX              3800
#endif
#ifndef TS_Y_MIN
  #define TS_Y_MIN              260
#endif
#ifndef TS_Y_MAX
  #define TS_Y_MAX              3800
#endif
#ifndef TS_STEPS
  #define TS_STEPS              40
#endif
#ifndef TS_MODE_SWITCH_CORNER
  #define TS_MODE_SWITCH_CORNER false
#endif
#ifndef TS_MODE_SWITCH_WIDTH
  #define TS_MODE_SWITCH_WIDTH  96
#endif
#ifndef TS_MODE_SWITCH_HEIGHT
  #define TS_MODE_SWITCH_HEIGHT 64
#endif
#ifndef TS_STATION_DOUBLE_TAP_MS
  #define TS_STATION_DOUBLE_TAP_MS  700
#endif
#ifndef TS_STATION_PAGE_STEPS
  #define TS_STATION_PAGE_STEPS 10
#endif
#ifndef TS_STATION_PAGE_CORNER_HEIGHT
  #define TS_STATION_PAGE_CORNER_HEIGHT 64
#endif
#ifndef TS_VOLUME_SWIPE_RANGE
  #define TS_VOLUME_SWIPE_RANGE 254
#endif
#ifndef TS_VOLUME_MIN_CHANGE
  #define TS_VOLUME_MIN_CHANGE  2
#endif

#if TS_MODEL==TS_MODEL_XPT2046
  #ifdef TS_SPIPINS
    SPIClass  TSSPI(HSPI);
  #endif
  #include <XPT2046_Touchscreen.h>
  XPT2046_Touchscreen ts(TS_CS);
  typedef TS_Point TSPoint;
#elif TS_MODEL==TS_MODEL_GT911
  #include "../GT911_Touchscreen/TAMC_GT911.h"
  TAMC_GT911 ts = TAMC_GT911(TS_SDA, TS_SCL, TS_INT, TS_RST, 0, 0);
  typedef TP_Point TSPoint;
#endif

void TouchScreen::init(uint16_t w, uint16_t h){
  
#if TS_MODEL==TS_MODEL_XPT2046
  #ifdef TS_SPIPINS
    TSSPI.begin(TS_SPIPINS);
    ts.begin(TSSPI);
  #else
    #if TS_HSPI
      ts.begin(SPI2);
    #else
      ts.begin();
    #endif
  #endif
  ts.setRotation(config.store.fliptouch?3:1);
#endif
#if TS_MODEL==TS_MODEL_GT911
  ts.begin();
  ts.setRotation(config.store.fliptouch?0:2);
#endif
  _width  = w;
  _height = h;
#if TS_MODEL==TS_MODEL_GT911
  ts.setResolution(_width, _height);
#endif
}

tsDirection_e TouchScreen::_tsDirection(uint16_t x, uint16_t y) {
  int16_t dX = x - _oldTouchX;
  int16_t dY = y - _oldTouchY;
  if (abs(dX) > 20 || abs(dY) > 20) {
    if (abs(dX) > abs(dY)) {
      if (dX > 0) {
        return TSD_RIGHT;
      } else {
        return TSD_LEFT;
      }
    } else {
      if (dY > 0) {
        return TSD_DOWN;
      } else {
        return TSD_UP;
      }
    }
  } else {
    return TDS_REQUEST;
  }
}

void TouchScreen::flip(){
#if TS_MODEL==TS_MODEL_XPT2046
  ts.setRotation(config.store.fliptouch?3:1);
#endif
#if TS_MODEL==TS_MODEL_GT911
  ts.setRotation(config.store.fliptouch?0:2);
#endif
}

void TouchScreen::loop(){
  uint16_t touchX, touchY;
  static bool wastouched = true;
  static uint32_t touchLongPress;
  static tsDirection_e direct;
  static uint16_t touchVolOrigin, touchStation;
  static uint8_t touchVolStart, touchVolLast;
  static bool gestureMoved;
  static uint32_t lastStationTap;
  static uint16_t lastStationTapItem;
  if (!_checklpdelay(20, _touchdelay)) return;
#if TS_MODEL==TS_MODEL_GT911
  ts.read();
#endif
  bool istouched = _istouched();
  if(istouched){
  #if TS_MODEL==TS_MODEL_XPT2046
    TSPoint p = ts.getPoint();
    touchX = map(p.x, TS_X_MIN, TS_X_MAX, 0, _width);
    touchY = map(p.y, TS_Y_MIN, TS_Y_MAX, 0, _height);
  #elif TS_MODEL==TS_MODEL_GT911
    TSPoint p = ts.points[0];
    touchX = p.x;
    touchY = p.y;
  #endif
  if (!wastouched) { /*     START TOUCH     */
      _oldTouchX = touchX;
      _oldTouchY = touchY;
      touchVolOrigin = touchX;
      touchVolStart = config.store.volume;
      touchVolLast = touchVolStart;
      touchStation = touchY;
      direct = TDS_REQUEST;
      gestureMoved = false;
      touchLongPress=millis();
    } else { /*     SWIPE TOUCH     */
      direct = _tsDirection(touchX, touchY);
      if(direct != TDS_REQUEST) gestureMoved = true;
      if(gestureMoved) {
        lastStationTap = 0;
        lastStationTapItem = 0;
      }
      switch (direct) {
        case TSD_LEFT:
        case TSD_RIGHT: {
            touchLongPress=millis();
            if(display.mode()==PLAYER || display.mode()==VOL){
              display.putRequest(NEWMODE, VOL);
              const int32_t travel = static_cast<int32_t>(touchX) - touchVolOrigin;
              int32_t targetVolume = static_cast<int32_t>(touchVolStart) +
                travel * TS_VOLUME_SWIPE_RANGE / _width;
              targetVolume = constrain(targetVolume, 0, 254);
              if(abs(targetVolume - touchVolLast) >= TS_VOLUME_MIN_CHANGE) {
                touchVolLast = static_cast<uint8_t>(targetVolume);
                player.setVol(touchVolLast);
              }
            }
            break;
          }
        case TSD_UP:
        case TSD_DOWN: {
            touchLongPress=millis();
            if(display.mode()==PLAYER || display.mode()==STATIONS){
              int16_t yDelta = map(abs(touchStation - touchY), 0, _height, 0, TS_STEPS);
              display.putRequest(NEWMODE, STATIONS);
              if (yDelta>1) {
                controlsEvent((touchStation - touchY)<0);
                touchStation = touchY;
              }
            }
            break;
          }
        default:
            break;
      }
    }
    if (config.store.dbgtouch) {
      Serial.print(", x = ");
      Serial.print(p.x);
      Serial.print(", y = ");
      Serial.println(p.y);
    }
  }else{
    if (wastouched) {/*     END TOUCH     */
      if (direct == TDS_REQUEST && !gestureMoved) {
        uint32_t pressTicks = millis()-touchLongPress;
        if( pressTicks < BTN_PRESS_TICKS*2){
          if(pressTicks > 50) {
#if TS_MODE_SWITCH_CORNER
            const bool modeSwitchTap =
              _oldTouchX >= (_width > TS_MODE_SWITCH_WIDTH ? _width - TS_MODE_SWITCH_WIDTH : 0) &&
              _oldTouchY < TS_MODE_SWITCH_HEIGHT &&
              (display.mode() == PLAYER || display.mode() == STATIONS);
            if(modeSwitchTap) {
              lastStationTap = 0;
              lastStationTapItem = 0;
              display.putRequest(NEWMODE, display.mode() == PLAYER ? STATIONS : PLAYER);
            } else if(display.mode() == STATIONS) {
              const uint32_t now = millis();
              const bool doubleTap =
                lastStationTap != 0 &&
                display.currentPlItem == lastStationTapItem &&
                now - lastStationTap <= TS_STATION_DOUBLE_TAP_MS;
              if(doubleTap) {
                lastStationTap = 0;
                lastStationTapItem = 0;
                onBtnClick(EVT_BTNCENTER);
              } else {
                // The first tap only confirms which centered station is
                // highlighted. A second tap on the same item starts playback.
                lastStationTap = now;
                lastStationTapItem = display.currentPlItem;
              }
            } else {
              lastStationTap = 0;
              lastStationTapItem = 0;
              onBtnClick(EVT_BTNCENTER);
            }
#else
            onBtnClick(EVT_BTNCENTER);
#endif
          }
        }else{
          const bool rightCorner =
            _oldTouchX >= (_width > TS_MODE_SWITCH_WIDTH ? _width - TS_MODE_SWITCH_WIDTH : 0);
          if(display.mode() == STATIONS && rightCorner &&
             (_oldTouchY < TS_STATION_PAGE_CORNER_HEIGHT ||
              _oldTouchY >= _height - TS_STATION_PAGE_CORNER_HEIGHT)) {
            const bool nextPage = _oldTouchY >= _height - TS_STATION_PAGE_CORNER_HEIGHT;
            lastStationTap = 0;
            lastStationTapItem = 0;
            for(uint8_t step = 0; step < TS_STATION_PAGE_STEPS; ++step) {
              controlsEvent(nextPage);
            }
          } else {
            display.putRequest(NEWMODE, display.mode() == PLAYER ? STATIONS : PLAYER);
          }
        }
      }
      direct = TSD_STAY;
    }
  }
  wastouched = istouched;
}

bool TouchScreen::_checklpdelay(int m, uint32_t &tstamp) {
  if (millis() - tstamp > m) {
    tstamp = millis();
    return true;
  } else {
    return false;
  }
}

bool TouchScreen::_istouched(){
#if TS_MODEL==TS_MODEL_XPT2046
  return ts.touched();
#elif TS_MODEL==TS_MODEL_GT911
  return ts.isTouched;
#endif
}

#endif  // TS_MODEL!=TS_MODEL_UNDEFINED
