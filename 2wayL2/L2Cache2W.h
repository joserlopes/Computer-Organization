#ifndef SIMPLECACHE_H
#define SIMPLECACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Cache.h"

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void accessL1(uint32_t, uint8_t *, uint32_t);
void accessL2(uint32_t, uint8_t *, uint32_t);

typedef struct CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint16_t Index;
  uint32_t Tag;
} CacheLine;

typedef struct CacheSet {
  uint8_t oldest; // 0 or 1 to know which block is the oldest
  uint16_t Index;
  CacheLine blocks[2];
} CacheSet;

typedef struct L1Cache {
  uint32_t init;
  CacheLine lines[256];
} L1Cache;

typedef struct L2Cache {
  uint32_t init;
  CacheSet sets[256];
} L2Cache;

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif
