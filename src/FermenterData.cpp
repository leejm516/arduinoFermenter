#include "FermenterData.h"

void sendPvProfile(PvProfile *p) {
    // 바이트 배열로 변환시 잘 출력되는지 확인해볼 것
    // ESP32 파트도 만들어 줘야 함
    uint8_t *ptr = (uint8_t *) p;
    Serial.write(ptr, sizeof(PvProfile));
}