// Computer organization course 2 way associative L2 Cache emulator

/* Made by:
 *
 * João Rocha - ist103465
 * João Fidalgo - ist103471
 * José Lopes - ist103938
 *
 */

/* Cache address organization
 *
 * L1  - tag-18bits/index-8bits/offset-6bits
 * L2  - tag-18bits/index-8bits/offset-4+2bits
 *
 */

#include "L2Cache2W.h"
#include "Cache.h"

uint8_t L1_Cache[L1_SIZE];
uint8_t L2_Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache SimpleL1Cache;
L2Cache SimpleL2Cache;

/**************** Initialization ***************/
void initCache() {
	SimpleL1Cache.init = 0;
	SimpleL2Cache.init = 0;
}

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
void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

	uint32_t index, Tag, offset;
	uint8_t TempBlock[BLOCK_SIZE];

	/* init cache */
	if (SimpleL1Cache.init == 0) {
		for (int i = 0; i < 256; i++)
				SimpleL1Cache.lines[i].Valid = 0;
		SimpleL1Cache.init = 1;
	}

	Tag = address >> 14; // shift right 14 times to remove offset and index
	index = address << 18; // shift left 18 times to remove tag
	index = index >> 24; // shift right 24 times to remove offset and put all the index bits on the right
	offset = address << 26;	// shift left 26 times to remove index and tag
	offset = offset >> 26;	// shift right 26 times to put all the offset bits on the right

	CacheLine *Line = &SimpleL1Cache.lines[index];

	/* access Cache*/

	if (!Line->Valid || Line->Tag != Tag) {         // if block not present - miss
		accessL2(address, TempBlock, MODE_READ);

		if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
			accessL2(address, &(L1_Cache[index * BLOCK_SIZE]), MODE_WRITE); // then write back old block
		}

		memcpy(&(L1_Cache[index * BLOCK_SIZE]), TempBlock,
					 BLOCK_SIZE); // copy new block to cache line
		Line->Valid = 1;
		Line->Tag = Tag;
		Line->Dirty = 0;
		Line->Index = index;
	} // if miss, then replaced with the correct block

	if (mode == MODE_READ) {    // read data from cache line
		memcpy(data, &(L1_Cache[index * BLOCK_SIZE + offset]), WORD_SIZE);
		time += L1_READ_TIME;
	}

	if (mode == MODE_WRITE) { // write data from cache line
		memcpy(&(L1_Cache[index * BLOCK_SIZE + offset]), data, WORD_SIZE);
		time += L1_WRITE_TIME;
		Line->Dirty = 1;
	}
}

/*********************** L2 cache *************************/
void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {

	uint32_t index, Tag, MemAddress, offset;
	uint8_t TempBlock[BLOCK_SIZE], oldest;

	/* init cache */
	if (SimpleL2Cache.init == 0) {
		for (int i = 0; i < 256; i++){
				SimpleL2Cache.sets[i].oldest = 0;
				for(int j = 0; j < 2; j++)
					SimpleL2Cache.sets[i].blocks[j].Valid = 0;
		} SimpleL2Cache.init = 1;
	}

	Tag = address >> 14; // shift right 14 times to remove offset and index
	index = address << 18; // shift left 18 times to remove tag
	index = index >> 24; // shift right 24 times to remove offset and put all the index bits on the right
	offset = address << 26; // shift left 26 times to remove index and tag
	offset = offset >> 26;  // shift right 26 times to put all the offset bits on the right
	MemAddress = address >> 6; // shift right 6 times to remove offset
	MemAddress = MemAddress << 6; // address of the block in memory

	CacheSet *Set = &SimpleL2Cache.sets[index]; // get the set
	oldest = Set->oldest;
/*--------------------------CASO DE HIT NUM BLOCO---------------------------*/
  for(int i = 0; i < 2; i++){
    if (Set->blocks[i].Valid && Set->blocks[i].Tag == Tag){
      if (mode == MODE_READ) {    // read data from cache line
        memcpy(data, &(L2_Cache[(index + i) * BLOCK_SIZE + offset]), WORD_SIZE);
        time += L2_READ_TIME;
        break;
      } 

      if (mode == MODE_WRITE) { // write data from cache line
          memcpy(&(L2_Cache[(index + i)* BLOCK_SIZE + offset]), data, WORD_SIZE);
          time += L2_WRITE_TIME;
          Set->blocks[i].Dirty = 1;
          break;
      }
    }
  } 
/*--------------------------------------------------------------------------*/
  

  /* access Cache*/
  if ((!Set->blocks[0].Valid || Set->blocks[0].Tag != Tag) && (!Set->blocks[1].Valid || Set->blocks[1].Tag != Tag)){
    accessDRAM(MemAddress, TempBlock, MODE_READ); // get new block from DRAM

    if (Set->blocks[oldest].Valid && (Set->blocks[oldest].Dirty)) { // line has dirty block
      MemAddress = Set->blocks[oldest].Tag << 14;
      MemAddress = MemAddress + Set->blocks[oldest].Index;        // get address of the block in memory
      accessDRAM(MemAddress, &(L2_Cache[(index + oldest) * BLOCK_SIZE]), MODE_WRITE); // then write back old block
    }

    if (Set->oldest) memcpy(&(L2_Cache[(index + 1) * BLOCK_SIZE]), TempBlock, BLOCK_SIZE); // copy new block to cache line
    
    else memcpy(&(L2_Cache[index * BLOCK_SIZE]), TempBlock, BLOCK_SIZE); // copy new block to cache line
      
      Set->blocks[oldest].Valid = 1;
      Set->blocks[oldest].Tag = Tag;
      Set->blocks[oldest].Dirty = 0;
      Set->blocks[oldest].Index = index;


    if (mode == MODE_READ) {    // read data from cache line
      memcpy(data, &(L2_Cache[(index + oldest) * BLOCK_SIZE + offset]), WORD_SIZE);
      time += L2_READ_TIME;
    } 

    if (mode == MODE_WRITE) { // write data from cache line
        memcpy(&(L2_Cache[(index + oldest)* BLOCK_SIZE + offset]), data, WORD_SIZE);
        time += L2_WRITE_TIME;
        Set->blocks[oldest].Dirty = 1;
    }

    if (Set->oldest) Set->oldest = 0; else Set->oldest = 1;  
  }

} // if miss, then replaced with the correct block

void read(uint32_t address, uint8_t *data) {
	accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
	accessL1(address, data, MODE_WRITE);
}
