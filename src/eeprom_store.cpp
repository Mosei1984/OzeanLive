#include "eeprom_store.h"
#include <EEPROM.h>

#ifdef ESP32
#define EEPROM_SIZE 1024
#endif

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

struct SaveDataV2 {
  char magic[4];
  uint8_t version;
  int16_t hunger;
  int16_t fun;
  int16_t energy;
  int16_t hp;
  uint32_t ageSec;
  uint8_t dead;
  uint16_t checksum;
} __attribute__((packed));

constexpr uint32_t MAGIC = 0x41514B50;
constexpr uint8_t VERSION = 1;
constexpr uint8_t RECORD_COUNT = 16;
constexpr uint16_t EEPROM_START = 0;
constexpr uint16_t V2_EEPROM_ADDR = 512;

static uint16_t lastSeq = 0;
static int8_t lastIndex = -1;
static unsigned long lastSaveMs = 0;
static int16_t lastHunger = -1;
static int16_t lastFun = -1;
static int16_t lastEnergy = -1;

static unsigned long lastV2SaveMs = 0;
static int16_t lastV2Hunger = -1;
static int16_t lastV2Fun = -1;
static int16_t lastV2Energy = -1;
static int16_t lastV2Hp = -1;
static uint32_t lastV2AgeSec = 0;
static bool lastV2Dead = false;

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
    
    if (maxIdx == -1 || (int16_t)(rec.seq - maxSeq) > 0) {
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
  
#ifdef ESP32
  EEPROM.commit();
  Serial.println("[SAVE] V1 record written to EEPROM");
#endif
  
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
  unsigned long minInterval = eventSave ? 30000 : 600000; // 30s for events, 10min for auto
  
  if (now - lastSaveMs < minInterval && lastSaveMs != 0) {
    return;
  }
  
  writeNextRecord(hunger, fun, energy);
}

bool hasSave() {
  SaveDataV2 save;
  EEPROM.get(V2_EEPROM_ADDR, save);
  
  if (memcmp(save.magic, "FISH", 4) == 0 && (save.version == 1 || save.version == 2)) {
    return true;
  }
  
  int8_t idx = findLatestRecord();
  return idx >= 0;
}

bool loadFull(int16_t& hunger, int16_t& fun, int16_t& energy, int16_t& hp, uint32_t& ageSec, bool& dead) {
  SaveDataV2 save;
  EEPROM.get(V2_EEPROM_ADDR, save);
  
  if (memcmp(save.magic, "FISH", 4) == 0) {
    if (save.version == 2) {
      uint16_t computedChecksum = crc16_ccitt((const uint8_t*)&save, sizeof(SaveDataV2) - 2);
      if (computedChecksum != save.checksum) {
        return false;
      }
      
      hunger = save.hunger;
      fun = save.fun;
      energy = save.energy;
      hp = save.hp;
      ageSec = save.ageSec;
      dead = (save.dead != 0);
      
      lastV2Hunger = hunger;
      lastV2Fun = fun;
      lastV2Energy = energy;
      lastV2Hp = hp;
      lastV2AgeSec = ageSec;
      lastV2Dead = dead;
      
      return true;
    } else if (save.version == 1) {
      int8_t idx = findLatestRecord();
      if (idx >= 0) {
        SaveRecord rec;
        uint16_t addr = EEPROM_START + idx * sizeof(SaveRecord);
        EEPROM.get(addr, rec);
        
        hunger = rec.hunger;
        fun = rec.fun;
        energy = rec.energy;
        hp = 20;
        ageSec = 0;
        dead = false;
        
        lastV2Hunger = hunger;
        lastV2Fun = fun;
        lastV2Energy = energy;
        lastV2Hp = hp;
        lastV2AgeSec = ageSec;
        lastV2Dead = dead;
        
        return true;
      }
    }
  }
  
  return false;
}

void saveFullIfDue(int16_t hunger, int16_t fun, int16_t energy, int16_t hp, uint32_t ageSec, bool dead, bool eventSave) {
  if (hunger == lastV2Hunger && fun == lastV2Fun && energy == lastV2Energy && 
      hp == lastV2Hp && ageSec == lastV2AgeSec && dead == lastV2Dead) {
    return;
  }
  
  unsigned long now = millis();
  unsigned long minInterval = eventSave ? 30000 : 600000; // 30s for events, 10min for auto
  
  if (now - lastV2SaveMs < minInterval && lastV2SaveMs != 0) {
    return;
  }
  
  SaveDataV2 save;
  memcpy(save.magic, "FISH", 4);
  save.version = 2;
  save.hunger = hunger;
  save.fun = fun;
  save.energy = energy;
  save.hp = hp;
  save.ageSec = ageSec;
  save.dead = dead ? 1 : 0;
  
  save.checksum = crc16_ccitt((const uint8_t*)&save, sizeof(SaveDataV2) - 2);
  
  EEPROM.put(V2_EEPROM_ADDR, save);
  
#ifdef ESP32
  EEPROM.commit();
  Serial.print("[SAVE] V2 full save written - HP: ");
  Serial.print(hp);
  Serial.print(", Age: ");
  Serial.print(ageSec);
  Serial.print(", Dead: ");
  Serial.println(dead);
#endif
  
  lastV2SaveMs = now;
  lastV2Hunger = hunger;
  lastV2Fun = fun;
  lastV2Energy = energy;
  lastV2Hp = hp;
  lastV2AgeSec = ageSec;
  lastV2Dead = dead;
}

void clearSave() {
  SaveDataV2 save;
  memset(&save, 0, sizeof(save));
  EEPROM.put(V2_EEPROM_ADDR, save);
  
#ifdef ESP32
  EEPROM.commit();
  Serial.println("[SAVE] Save cleared");
#endif
}
