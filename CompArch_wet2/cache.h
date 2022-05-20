#ifndef CACHE_H_
#define CACHE_H_

#include <iostream>
#include <math.h>
#include <vector>

using std::vector;

class CacheEntry {
private:
	unsigned tag;
	bool valid_bit;
	bool dirty_bit;
	unsigned int counter;

public:
	CacheEntry() : tag(0), valid_bit(false), dirty_bit(false), counter(0) {}
	CacheEntry(const CacheEntry& cach_entry) = default;
	~CacheEntry() = default;
	CacheEntry& operator=(const CacheEntry&) = default;

	unsigned getTag() { return tag; }
	bool isValid() { return valid_bit; }
	bool isDirty() { return dirty_bit; }
	unsigned int getCounter() { return counter; }
	void setCounter(unsigned int cnt) { counter = cnt; }
	void decCounter() { counter--; }
	void setTag(unsigned newTag) { tag = newTag; }
	void setValid(bool isValid) { valid_bit = isValid; }
	void setDirtyBit(bool isDirty) { dirty_bit = isDirty; }
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
	unsigned miss_number;
	int ways_number;
	vector<vector<CacheEntry>> ways;


public:
	Cache(unsigned cahceSize, unsigned blockSize, unsigned assocLevel, unsigned cyclesNum);
	Cache(const Cache& cach) = default;
	~Cache() = default;
	Cache& operator=(const Cache&) = default;

	unsigned getAddressSet(unsigned long int address);
	unsigned getAddressTag(unsigned long int address);

	void LRUUpdate(unsigned way, unsigned set);
	bool read(unsigned long int address);
	bool write(unsigned long int address);
	unsigned int getLRUWay(unsigned int set);
	unsigned long int buildAddress(unsigned tag, unsigned set);
	void removeLRU(unsigned tag, unsigned set);

	void incCacheAccess() { cache_access_number++; }
	void incMissNumber() { miss_number++; }

	unsigned getCycleCahceNumber() { return cache_cycles_number; }
	unsigned getCahceAccessNumber() { return cache_access_number; }
	unsigned getMissNumber() { return miss_number; }
	//unsigned getTag(unsigned way, unsigned set) { return ways[way][set].getTag(); }
	bool getDirty(unsigned way, unsigned set) { return ways[way][set].isDirty(); }
	bool isValidEntry(unsigned way, unsigned set);

	void writeBack(unsigned way, unsigned set, unsigned tag, bool modify_dirty);

	double getMissRate() { return (cache_access_number ? double(miss_number) / double(cache_access_number) : 0); }
};


Cache::Cache(unsigned cahceSize, unsigned blockSize, unsigned assocLevel, unsigned cyclesNum) : cache_size(cahceSize),
	block_size(blockSize), associative_level(assocLevel), cache_cycles_number(cyclesNum), cache_access_number(0), 
	miss_number(0), ways_number(pow(2, assocLevel))
{
	offset_bits = blockSize;
	set_bits = cahceSize - assocLevel - blockSize;
	tag_bits = 32 - (set_bits + offset_bits);

	ways = vector<vector<CacheEntry>>(ways_number, vector<CacheEntry>(pow(2, set_bits)));
}

inline unsigned Cache::getAddressSet(unsigned long int address)
{
	return ((address >> offset_bits) & (int)pow(2, set_bits) - 1);
}

inline unsigned Cache::getAddressTag(unsigned long int address)
{
	return (address >> (offset_bits + set_bits) & (int)pow(2, tag_bits) - 1);
}

inline void Cache::LRUUpdate(unsigned way, unsigned set)
{
	unsigned int x = ways[way][set].getCounter();
	ways[way][set].setCounter(ways_number - 1);
	for (int j = 0; j < ways_number; j++) {
		if ((j != way) && (ways[j][set].getCounter() > x)) {
			ways[j][set].decCounter();
		}
	}
}

inline bool Cache::read(unsigned long int address)
{
	unsigned address_set = getAddressSet(address);
	unsigned address_tag = getAddressTag(address);

	// search for the valid block address in the ways vector
	for (int way = 0; way < ways_number; way++) {
		// HIT
		if (ways[way][address_set].getTag() == address_tag && ways[way][address_set].isValid()) { 
			LRUUpdate(way, address_set);
			return true;
		}
	}
	return false;
}

inline bool Cache::write(unsigned long int address)
{
	unsigned address_set = getAddressSet(address);
	unsigned address_tag = getAddressTag(address);

	// HIT
	for (int way = 0; way < ways_number; way++) {
		if (ways[way][address_set].getTag() == address_tag && ways[way][address_set].isValid()) {
			LRUUpdate(way, address_set);
			ways[way][address_set].setDirtyBit(true);
			return true;
		}
	}
	return false;
}

inline unsigned int Cache::getLRUWay(unsigned int set)
{
	unsigned int min_counter = ways[0][set].getCounter();
	unsigned int min_way = 0;
	for (int i = 0; i < ways_number; i++) {
		if (min_counter > ways[i][set].getCounter()) {
			min_counter = ways[i][set].getCounter();
			min_way = i;
		}
	}

	return min_way;
}

inline unsigned long int Cache::buildAddress(unsigned tag, unsigned set)
{
	unsigned long int address = tag;
	address <<= set_bits;
	address += set;
	address <<= offset_bits;

	return address;
}

inline void Cache::removeLRU(unsigned tag, unsigned set)
{
	unsigned long int target_address = buildAddress(tag, set);
	unsigned address_set = getAddressSet(target_address);
	unsigned address_tag = getAddressTag(target_address);

	int way = -1;
	for (int i = 0; i < ways_number; i++) {
		if (ways[i][address_set].getTag() == address_tag) {
			way = i;
		}
	}

	if (way == -1) return;
	ways[way][address_set].setValid(false);
}

inline bool Cache::isValidEntry(unsigned way, unsigned set)
{
	return (ways[way][set].isValid());
}

inline void Cache::writeBack(unsigned way, unsigned set, unsigned tag, bool modify_dirty)
{
	ways[way][set].setDirtyBit(modify_dirty);
	ways[way][set].setValid(true);
	ways[way][set].setTag(tag);
	LRUUpdate(way, set);
}


class CacheSim {
private:
	unsigned is_write_allocate;
	unsigned memCyc;
	unsigned memory_cycle;
	unsigned cacheSim_access_number;
	Cache* L1;
	Cache* L2;

	void writeAllocate(unsigned long int address);
	void L2Hit(unsigned long int address, bool modify_dirty);

public:
	CacheSim(unsigned MemCyc, unsigned BSize, unsigned L1Size, unsigned L2Size, unsigned L1Assoc,
		unsigned L2Assoc, unsigned L1Cyc, unsigned L2Cyc, unsigned WrAlloc);
	~CacheSim();
	CacheSim(const CacheSim& cach) = default;
	CacheSim& operator=(const CacheSim&) = default;

	void operationHandle(char operation_type, unsigned long int address);
	double getL1MissRate() { return L1->getMissRate(); }
	double getL2MissRate() { return L2->getMissRate(); }
	double getAvgAccessTime() { return (cacheSim_access_number ? double(memory_cycle) / double(cacheSim_access_number) : 0); }
};

inline void CacheSim::writeAllocate(unsigned long int address)
{
	unsigned address_set_L2 = L2->getAddressSet(address);
	unsigned address_tag_L2 = L2->getAddressTag(address);

	unsigned LRU_way = L2->getLRUWay(address_set_L2);
	if (L2->isValidEntry(LRU_way, address_set_L2)) {
		L1->removeLRU(address_tag_L2, address_set_L2);
	}
	L2->writeBack(LRU_way, address_set_L2, address_tag_L2, false);
}

inline void CacheSim::L2Hit(unsigned long int address, bool modify_dirty)
{
	unsigned address_set_L1 = L1->getAddressSet(address);
	unsigned address_tag_L1 = L1->getAddressTag(address);
	unsigned LRU_way1 = L1->getLRUWay(address_set_L1);
	bool dirty = L1->getDirty(LRU_way1, address_set_L1);

	L1->writeBack(LRU_way1, address_set_L1, address_tag_L1, modify_dirty);
	if (dirty) {
		L2->write(L2->buildAddress(address_tag_L1, address_set_L1));
	}
}

CacheSim::CacheSim(unsigned MemCyc, unsigned BSize, unsigned L1Size, unsigned L2Size, unsigned L1Assoc, unsigned L2Assoc,
	unsigned L1Cyc, unsigned L2Cyc, unsigned WrAlloc) : is_write_allocate(WrAlloc), memCyc(MemCyc), memory_cycle(0),
	cacheSim_access_number(0), L1(new Cache(L1Size, BSize, L1Assoc, L1Cyc)), L2(new Cache(L2Size, BSize, L2Assoc, L2Cyc)) {}


inline CacheSim::~CacheSim()
{
	delete L1;
	delete L2;
}

inline void CacheSim::operationHandle(char operation_type, unsigned long int address)
{
	if (operation_type == 'w' || operation_type == 'W') {
		cacheSim_access_number++;
		L1->incCacheAccess();
		memory_cycle += L1->getCycleCahceNumber();

		if (!L1->write(address)) { // L1 MISS
			L1->incMissNumber();
			L2->incCacheAccess();
			memory_cycle += L2->getCycleCahceNumber();

			if (!L2->write(address)) { // L2 MISS
				L2->incMissNumber();
				memory_cycle += memCyc;

				// update
				if (is_write_allocate) {
					writeAllocate(address);
				}
				// if write no allocate, we do nothing
			}

			// HIT IN L2
			if (is_write_allocate) {
				L2Hit(address, true);
			}
		}
	}

	else if (operation_type == 'r' || operation_type == 'R') {
		cacheSim_access_number++;
		L1->incCacheAccess();
		memory_cycle += L1->getCycleCahceNumber();

		if (!L1->read(address)) { // L1 MISS 
			L1->incMissNumber();
			L2->incCacheAccess();
			memory_cycle += L2->getCycleCahceNumber();

			if (!L2->read(address)) { // L2 MISS
				L2->incMissNumber();
				memory_cycle += memCyc;
				writeAllocate(address);
			}
			L2Hit(address, false);
		}
	}
}

#endif // CACHE_H