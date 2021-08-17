#include "cgmin/GlobalData.hpp"
#include "oops/method.hpp"
//#include "oops/oopsHierarchy.hpp"

int GlobalData::test_m(Method *m,int bci)
{
	char s[100];
	m->name_and_sig_as_C_string(s,100);
	if (s[0] == 'j' && s[1] == 'a' && s[2] == 'v' && s[3] == 'a')
		return 0;
	return 1;
}

int GlobalData::hc_hash(int h,int c) //heade and class
{
	return h + c; //temp
}

int GlobalData::get_target_region_group(oop const o)
{
	return 0; //temp
//	return max_lt[get_oop_hash(o)]/region_time_unit;
}
int GlobalData::get_oop_hash(oop const o)
{
	return hc_hash(0,0); //temp
}

int oopToUniqueId(oop const o)
{
	return 0; //temp
}

int hcToUniqueId(int h,int c)
{
	return 0; //temp
}
// stack
//--------------------------------------------------------------------------
// region
void GlobalData::region_init()
{
	g1h = G1CollectedHeap::heap();
	last_gc_time = 0;
}

void GlobalData::gc_start()
{
	struct timespec ts;
	gc_start_time = ts.tv_sec*1000000000+ts.tv_nsec;
}

void GlobalData::gc_end()
{
	struct timespec ts;
	last_gc_time = ts.tv_sec*1000000000+ts.tv_nsec - gc_start_time;
}

void GlobalData::initHeapRegionArray()
{
	heapRegionCount = 0;
	UpdateRegionTimeClosure cl;
	cl.region_count = &heapRegionCount;
	cl.sortedHeapRegionPointer = &sortedHeapRegionPointer;
	cl.sortedHeapRegionTime = &sortedHeapRegionTime;
	g1h->heap_region_iterate(cl);
}

void GlobalData::sortHeapRegionArray()
{
	int i;
	HeapRegion* tp;
	unsigned long tv;
	for (i=0;i<heapRegionCount;i++)
	{
		for (j=i+1;j<heapRegionCount;j++)
		{
			if (sortedHeapRegionTime[i] > sortedHeapRegionTime[j])
			{
				tp = sortedHeapRegionPointer[i];
				sortedHeapRegionPointer[i] = sortedHeapRegionPointer[j];
				sortedHeapRegionPointer[j] = tp;
				tv = sortedHeapRegionTime[i];
				sortedHeapRegionTime[i] = sortedHeapRegionTime[j];
				sortedHeapRegionTime[j] = tp;
			}
		}
	}
}

//PLAB
void GlobalData::regionFull(int dst_region)
{
	if (dst_region < 0)
		return;
	isRegionFull[dst_region] = true;
}

// find fit reigon
// alloc new region
// find upper region
// find lower region
// OOM

// -------lower------//lt//------fit-------//lt+minor_GC//------upper------

int GlobalData::scid_to_dst_region(int scid)
{
	if (isRegionFull[scid] == false && scidToRegion[scid] > -1)
		return scidToRegion[scid];
	// find new region

	//find scid time
	//find appropriate region
	//set and region or alloc new region or fail and return -1


	int i,li,ui;
	unsigned long lt = max_scid_lt[scid];

	li = -1;
	ui = -1;
	for (i=0;i<heapRegionCount;i++) // scan ver may need improvement
	{
		if (isRegionFull[i])
			continue;
		if (sortedHeapRegionTime[i] <= lt)
			li = i;
		else if (sortedHeapRegionTime[i] >= lt && sortedHeapRegionTime[i] <= lt+minor_GC_interval)
			return sortedHeapRegionPointer[i]->hrm_index();//return index
		else
		{
			ui = i;
			break;
		}

	}
	//alloc new region

	InCSetState dest(1); // Young
	HeapRegion* new_alloc_region = g1h->new_gc_alloc_region(0,dest);

	if (new_alloc_region != NULL) {
    new_alloc_region->reset_pre_dummy_top();
    // Need to do this before the allocation
    _used_bytes_before = new_alloc_region->used();
    HeapWord* result = allocate(new_alloc_region, word_size);
    assert_alloc_region(result != NULL, "the allocation should succeeded");

    OrderAccess::storestore();
    // Note that we first perform the allocation and then we store the
    // region in _alloc_region. This is the reason why an active region
    // can never be empty.
//    update_alloc_region(new_alloc_region);
    trace("region allocation successful");

for (i=heapRegionCount;i>=0;i--)
{
	if (i == 0 || sortedHeapRegionTime[i-1] <= lt)
	{
		sortedHeapRegionTime[i] = lt;
		sortedHeapRegionPointer[i] = new_alloc_region;
		isRegionFull[i] = false;

		//need reigon time initialization


		break;
	}
	sortedHeapRegionTime[i] = sortedHeapReigonTime[i-1];
	sortedHeapRegionPointer[i] = sortedHeapRegionPointer[i-1];
	isRegionFull[i] = isRegionFull[i-1];

}
heapRegionCount++;


    return new_alloc_region;
	}

	if (ui != -1)
		return sortedHeapRegionPointer[ui]->hrm_index();
	if (li != -1)
		return sortedHeapRegionPointer[li]->hrm_index();
	return -1; // OOM
}

