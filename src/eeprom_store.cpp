#include "eeprom_store.h"
#include <EEPROM.h>

struct SaveRecord {
  uint32_t magic;
  uint8_t version;
  uint8_t length;
  uint16_t seq;
  int16_t hunger;
  int16_t fun;
  int16_t energy;
  uint16_t crc;
  uint8_t padding[16];
} __attribute__((packed));

static_assert(sizeof(SaveRecord) == 32, "SaveRecord must be 32 bytes");

constexpr uint32_t MAGIC = 0x41514B50;
constexpr uint8_t VERSION = 1;
constexpr uint8_t RECORD_COUNT = 16;
constexpr uint16_t EEPROM_START = 0;

static uint16_t lastSeq = 0;
static int8_t lastIndex = -1;
static unsigned long lastSaveMs = 0;
static int16_t lastHunger = -1;
static int16_t lastFun = -1;
static int16_t lastEnergy = -1;

uint16_t crc16_ccitt(const uint8_t* data, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    }
  }
  return crc;
}

int8_t findLatestRecord() {
  uint16_t maxSeq = 0;
  int8_t maxIdx = -1;
  
  for (uint8_t i = 0; i < RECORD_COUNT; i++) {
    SaveRecord rec;
    uint16_t addr = EEPROM_START + i * sizeof(SaveRecord);
    EEPROM.get(addr, rec);
    
    if (rec.magic != MAGIC) continue;
    
    uint16_t computedCrc = crc16_ccitt((const uint8_t*)&rec, sizeof(SaveRecord) - 2);
    if (computedCrc != rec.crc) continue;
    
    if (maxIdx == -1 || rec.seq > maxSeq) {
      maxSeq = rec.seq;
      maxIdx = i;
    }
  }
  
  if (maxIdx >= 0) {
    lastSeq = maxSeq;
    lastIndex = maxIdx;
  }
  
  return maxIdx;
}

void writeNextRecord(int16_t hunger, int16_t fun, int16_t energy) {
  SaveRecord rec;
  rec.magic = MAGIC;
  rec.version = VERSION;
  rec.length = sizeof(SaveRecord);
  rec.seq = lastSeq + 1;
  rec.hunger = hunger;
  rec.fun = fun;
  rec.energy = energy;
  memset(rec.padding, 0, sizeof(rec.padding));
  
  rec.crc = crc16_ccitt((const uint8_t*)&rec, sizeof(SaveRecord) - 2);
  
  int8_t nextIdx = (lastIndex + 1) % RECORD_COUNT;
  uint16_t addr = EEPROM_START + nextIdx * sizeof(SaveRecord);
  EEPROM.put(addr, rec);
  
  lastSeq = rec.seq;
  lastIndex = nextIdx;
  lastSaveMs = millis();
  lastHunger = hunger;
  lastFun = fun;
  lastEnergy = energy;
}

bool loadStats(int16_t& hunger, int16_t& fun, int16_t& energy) {
  int8_t idx = findLatestRecord();
  if (idx < 0) return false;
  
  SaveRecord rec;
  uint16_t addr = EEPROM_START + idx * sizeof(SaveRecord);
  EEPROM.get(addr, rec);
  
  hunger = rec.hunger;
  fun = rec.fun;
  energy = rec.energy;
  
  lastHunger = hunger;
  lastFun = fun;
  lastEnergy = energy;
  
  return true;
}

void saveStatsIfDue(int16_t hunger, int16_t fun, int16_t energy, bool eventSave) {
  if (hunger == lastHunger && fun == lastFun && energy == lastEnergy) {
    return;
  }
  
  unsigned long now = millis();
  unsigned long minInterval = eventSave ? 10000 : 60000;
  
  if (now - lastSaveMs < minInterval && lastSaveMs != 0) {
    return;
  }
  
  writeNextRecord(hunger, fun, energy);
}
