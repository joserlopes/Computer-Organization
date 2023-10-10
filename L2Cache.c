/* L1 bits - tag-18bits/index-8bits/offset-6bits */
/* L2 bits - tag-17bits/index-9bits/offset-6bits*/
#include "L2Cache.h"

uint8_t L1_Cache[L1_SIZE];
uint8_t L2_Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache SimpleL1Cache;
L1Cache SimpleL2Cache;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}

/*********************** L1 cache *************************/

void initL1Cache() { SimpleL1Cache.init = 0; }

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, MemAddress;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (SimpleL1Cache.init == 0) {
    for (int i = 0; i < 256; i++)
        SimpleL1Cache.lines[i].Valid = 0;
    SimpleL1Cache.init = 1;
  }

  Tag = address >> 14; // shift right 14 times to remove offset and index
  index = address << 18;
  index = index >> 24;
  MemAddress = address >> 6;
  MemAddress = MemAddress << 6; // address of the block in memory

  CacheLine *Line = &SimpleL1Cache.lines[index];

  /* access Cache*/

  if (!Line->Valid || Line->Tag != Tag) {         // if block not present - miss
    accessL2(MemAddress, TempBlock, MODE_READ); // get new block from DRAM

    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      MemAddress = Line->Tag << 3;        // get address of the block in memory
      accessL2(MemAddress, &(L1_Cache[0]), MODE_WRITE); // then write back old block
    }

    memcpy(&(L1_Cache[0]), TempBlock,
           BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    if (0 == (address % 8)) { // even word on block
      memcpy(data, &(L1_Cache[0]), WORD_SIZE);
    } else { // odd word on block
      memcpy(data, &(L1_Cache[WORD_SIZE]), WORD_SIZE);
    }
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    if (!(address % 8)) {   // even word on block
      memcpy(&(L1_Cache[0]), data, WORD_SIZE);
    } else { // odd word on block
      memcpy(&(L1_Cache[WORD_SIZE]), data, WORD_SIZE);
    }
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
  }
}

/*********************** L2 cache *************************/

void initL2Cache() { SimpleL2Cache.init = 0; }

void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, MemAddress;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (SimpleL2Cache.init == 0) {
    for (int i = 0; i < 512; i++)
        SimpleL2Cache.lines[i].Valid = 0;
    SimpleL2Cache.init = 1;
  }

  Tag = address >> 15; // shift right 14 times to remove offset and index
  index = address << 17;
  index = index >> 25;
  MemAddress = address >> 6;
  MemAddress = MemAddress << 6; // address of the block in memory

  CacheLine *Line = &SimpleL2Cache.lines[index];

  /* access Cache*/

  if (!Line->Valid || Line->Tag != Tag) {         // if block not present - miss
    accessDRAM(MemAddress, TempBlock, MODE_READ); // get new block from DRAM

    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      MemAddress = Line->Tag << 3;        // get address of the block in memory
      accessDRAM(MemAddress, &(L2_Cache[0]), MODE_WRITE); // then write back old block
    }

    memcpy(&(L2_Cache[0]), TempBlock,
           BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    if (0 == (address % 8)) { // even word on block
      memcpy(data, &(L2_Cache[0]), WORD_SIZE);
    } else { // odd word on block
      memcpy(data, &(L2_Cache[WORD_SIZE]), WORD_SIZE);
    }
    time += L2_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    if (!(address % 8)) {   // even word on block
      memcpy(&(L2_Cache[0]), data, WORD_SIZE);
    } else { // odd word on block
      memcpy(&(L2_Cache[WORD_SIZE]), data, WORD_SIZE);
    }
    time += L2_WRITE_TIME;
    Line->Dirty = 1;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
