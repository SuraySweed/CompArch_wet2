#ifndef CACHE_H_
#define CACHE_H_

#include <iostream>
#include <math.h>
#include <vector>

using std::vector;

class CacheEntry {
private:
	uint32_t tag;
	bool valid_bit;
	bool dirty_bit;
	unsigned int counter;

public:
	CacheEntry() : tag(0), valid_bit(false), dirty_bit(false), counter(0) {}
	CacheEntry(const CacheEntry& cach_entry) = default;
	~CacheEntry() = default;
	CacheEntry& operator=(const CacheEntry&) = default;
};


class Cache {
private:
	unsigned cache_size;
	unsigned block_size;
	unsigned associative_level;
	unsigned int cache_cycles_number;
	unsigned tag_bits;
	unsigned set_bits;
	unsigned offset_bits;
	unsigned cache_access_number;
	int ways_number;
	vector<vector<CacheEntry>> ways;

public:
	Cache(unsigned cahceSize, unsigned blockSize, unsigned assocLevel, unsigned cyclesNum);
	Cache(const Cache& cach) = default;
	~Cache() = default;
	Cache& operator=(const Cache&) = default;


};


Cache::Cache(unsigned cahceSize, unsigned blockSize, unsigned assocLevel, unsigned cyclesNum) : cache_size(cahceSize),
	block_size(blockSize), associative_level(assocLevel), cache_cycles_number(cyclesNum), cache_access_number(0), 
	ways_number(pow(2, assocLevel))
{
	offset_bits = blockSize;
	set_bits = cahceSize - assocLevel - blockSize;
	tag_bits = 32 - (set_bits + offset_bits);

	ways = vector<vector<CacheEntry>>(ways_number, vector<CacheEntry>(pow(2, set_bits)));
}








class CacheSim {
private:
	unsigned is_write_allocate;
	unsigned memory_cycle;
	unsigned cacheSim_access_number;
	Cache* L1;
	Cache* L2;

public:
	CacheSim(unsigned MemCyc, unsigned BSize, unsigned L1Size, unsigned L2Size, unsigned L1Assoc,
		unsigned L2Assoc, unsigned L1Cyc, unsigned L2Cyc, unsigned WrAlloc);
	~CacheSim();
	CacheSim(const CacheSim& cach) = default;
	CacheSim& operator=(const CacheSim&) = default;

	void operationHandle(char operation_type, unsigned long int address);
};

CacheSim::CacheSim(unsigned MemCyc, unsigned BSize, unsigned L1Size, unsigned L2Size, unsigned L1Assoc, unsigned L2Assoc,
	unsigned L1Cyc, unsigned L2Cyc, unsigned WrAlloc) : is_write_allocate(WrAlloc), memory_cycle(MemCyc), cacheSim_access_number(0), 
	L1(new Cache(L1Size, BSize, L1Assoc, L1Cyc)), L2(new Cache(L2Size, BSize, L2Assoc, L2Cyc)) {}


inline CacheSim::~CacheSim()
{
	delete L1;
	delete L2;
}

inline void CacheSim::operationHandle(char operation_type, unsigned long int address)
{
	if (operation_type == 'w') {
		if (is_write_allocate) {
			//write
			//add to cache
		}
		else {
			//write
		}
	}
}

#endif // CACHE_H