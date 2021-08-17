#ifndef SHARE_CGMIN_GLOBAL_DATA_HPP
#define SHARE_CGMIN_GLOBAL_DATA_HPP

#define HT_NUM 10000
#define REGION_NUM 2048
#define SCID_NUM 10000

class Method;
//class oop;

#include "oops/oopsHierarchy.hpp"

class GlobalData
{
	public:
		GlobalData() : minor_GC_interval(100),region_time_unit(100) {
//		g1h = G1CollectedHeap::heap();
		} //temp

//------------------------------------------------------------------- stack

	int test_m(Method* m,int bci);

	// header-method + class

	// hash?

	int hc_hash(int h,int c);


	unsigned long max_lt[HT_NUM]; //max life time
	int max_sc[HT_NUM],current_sc[HT_NUM]; //sample count
	//pointer sample_object[HT_NUM];


	int get_target_region_group(oop const o);
	int get_oop_hash(oop const o);

	int oopToUniqueId(oop const o);
	int hcToUniqueId(int h,int c); //int ?? max value??

	unsigned long max_scid_lt[SCID_NUM];

//------------------------------------------------------------------- region

	void gc_start();
	void gc_end();

	void region_init();

	unsigned long gc_start_time;
	unsigned long minor_GC_interval;
	unsigned long region_time_unit;
	unsigned long last_gc_time;


	G1CollectedHeap* g1h;
	//need max heap region count
	//or need growable array
	HeapRegion* sortedHeapRegionPointer[REGION_NUM];
	unsigned long sortedHeapRegionTime[REGION_NUM];
	int heapRegionCount;

	void initHeapRegionArray();
	void sortHeapRegionArray();

	//------------------------------------- PLAB
	bool isRegionFull[REGION_NUM];
	int scidToRegion[SCID_NUM];
	void regionFull(int dst_region);
	void scid_to_dst_region(int scid);
};

class UpdateRegionTimeClosure : public HeapRegionClosure {

	public:
	HeapRegion** sortedHeapRegionPointer;
	unsigned long* sortedHeapRegionTime;
	unsigned long last_gc_time;
	int* region_count;

	virtual bool do_heap_region(HeapRegion* r) {
		sortedHeapRegionPointer[*region_count] = r;
		r->gc_time+=last_gc_time;
		sortedHeapRegionTime[*region_count] = r->end_time-r->start_time+r->gc_time;
		++(*region_count);
		return false;
	}
}

#endif
