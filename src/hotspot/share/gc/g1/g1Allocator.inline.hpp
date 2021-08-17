/*
 * Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_GC_G1_G1ALLOCATOR_INLINE_HPP
#define SHARE_VM_GC_G1_G1ALLOCATOR_INLINE_HPP

#include "gc/g1/g1Allocator.hpp"
#include "gc/g1/g1AllocRegion.inline.hpp"
#include "gc/shared/plab.inline.hpp"

inline MutatorAllocRegion* G1Allocator::mutator_alloc_region() {
  return &_mutator_alloc_region;
}

inline SurvivorGCAllocRegion* G1Allocator::survivor_gc_alloc_region() {
  return &_survivor_gc_alloc_region;
}

inline OldGCAllocRegion* G1Allocator::old_gc_alloc_region() {
  return &_old_gc_alloc_region;
}

inline HeapWord* G1Allocator::attempt_allocation(size_t min_word_size,
                                                 size_t desired_word_size,
                                                 size_t* actual_word_size) {
  HeapWord* result = mutator_alloc_region()->attempt_retained_allocation(min_word_size, desired_word_size, actual_word_size);
  if (result != NULL) {
    return result;
  }
  return mutator_alloc_region()->attempt_allocation(min_word_size, desired_word_size, actual_word_size);
}

inline HeapWord* G1Allocator::attempt_allocation_locked(size_t word_size) {
  HeapWord* result = mutator_alloc_region()->attempt_allocation_locked(word_size);
  assert(result != NULL || mutator_alloc_region()->get() == NULL,
         "Must not have a mutator alloc region if there is no memory, but is " PTR_FORMAT, p2i(mutator_alloc_region()->get()));
  return result;
}

inline HeapWord* G1Allocator::attempt_allocation_force(size_t word_size) {
  return mutator_alloc_region()->attempt_allocation_force(word_size);
}

inline PLAB* G1PLABAllocator::alloc_buffer(/*int target_region_group*/InCSetState dest) { //cgmin region // rollback
  assert(dest.is_valid(),
         "Allocation buffer index out of bounds: " CSETSTATE_FORMAT, dest.value());
  assert(_alloc_buffers[dest.value()] != NULL,
         "Allocation buffer is NULL: " CSETSTATE_FORMAT, dest.value());
  return _alloc_buffers[dest.value()];
/*
	if (target_region_group >= max_region_group || _alloc_buffer[target_region_group] == NULL)
		return NULL;
return _alloc_buffer[target_region_group];
*/
}
#if 1
inline HeapWord* G1PLABAllocator::plab_allocate(/*int target_region_group,*/ InCSetState dest, //cgmin region
                                                size_t word_sz) {
  PLAB* buffer = alloc_buffer(dest);
//	PLAB* buffer = alloc_buffer(target_region_group);
//	if (target_region_group >= max_region_group || _alloc_buffer[target_region_group] == NULL)
//		return NULL;
//	PLAB* buffer = _alloc_buffer[target_region_group];
	/*
	PLAB* buffer = alloc_buffer(target_region_group);
	if (buffer == NULL)
		return NULL;
		*/
  if (_survivor_alignment_bytes == 0 || !dest.is_young()) {
    return buffer->allocate(word_sz);
  } else {
    return buffer->allocate_aligned(word_sz, _survivor_alignment_bytes);
  }
}
#endif

inline HeapWord* G1PLABAllocator::plab_allocate(int scid,size_t word_sz)
{

	// scid to region
	// if there is no region find new region (repeat)
	// region to plab
	// if there is no plab alloc new plab
	// if there is no space in region find new region
	// plab alloc
	// if there is no space in plab alloc new plab
	// if there is no space in region find new region
	



HeapWord* result = NULL;
	int dst_region = -1;// = Universe::gd.scidToDstRegion(scid); // include NULL case handle
	HeapRegion* region;// = _g1h->region_at(dst_region);
//	PLAB* plab;// = region->plab_for_allocator(allocator_id);
while(result == NULL)
{
//	while (plab == NULL) //there is no plab for allocator and can't alloc new
//	{
		Universe::gd.regionFull(dst_region); // when dst_region is -1 do nothing
		dst_region = Universe::gd.scid_to_dst_region(scid);
		if (dst_region < 0) // OOM
			return NULL;
		region = _g1h->region_at(dst_region);
//		 plab = region->plab_for_allocator(allocator_id);
		result = region->region_plab_allocate(allocator_id,word_sz);
//	}


//  if (_survivor_alignment_bytes == 0/* || !dest.is_young()*/) {
//    result = plab->allocate(word_sz);
//  } else {
//    result = plab->allocate_aligned(word_sz, _survivor_alignment_bytes);
//  }
}


  return result;
}


inline HeapWord* G1PLABAllocator::allocate(InCSetState dest,
                                           size_t word_sz,
                                           bool* refill_failed) {
  HeapWord* const obj = plab_allocate(dest, word_sz);
  if (obj != NULL) {
    return obj;
  }
  return allocate_direct_or_new_plab(dest, word_sz, refill_failed);
}

// Create the maps which is used to identify archive objects.
inline void G1ArchiveAllocator::enable_archive_object_check() {
  if (_archive_check_enabled) {
    return;
  }

  _archive_check_enabled = true;
  size_t length = Universe::heap()->max_capacity();
  _closed_archive_region_map.initialize((HeapWord*)Universe::heap()->base(),
                                        (HeapWord*)Universe::heap()->base() + length,
                                        HeapRegion::GrainBytes);
  _open_archive_region_map.initialize((HeapWord*)Universe::heap()->base(),
                                      (HeapWord*)Universe::heap()->base() + length,
                                      HeapRegion::GrainBytes);
}

// Set the regions containing the specified address range as archive.
inline void G1ArchiveAllocator::set_range_archive(MemRegion range, bool open) {
  assert(_archive_check_enabled, "archive range check not enabled");
  log_info(gc, cds)("Mark %s archive regions in map: [" PTR_FORMAT ", " PTR_FORMAT "]",
                     open ? "open" : "closed",
                     p2i(range.start()),
                     p2i(range.last()));
  if (open) {
    _open_archive_region_map.set_by_address(range, true);
  } else {
    _closed_archive_region_map.set_by_address(range, true);
  }
}

// Clear the archive regions map containing the specified address range.
inline void G1ArchiveAllocator::clear_range_archive(MemRegion range, bool open) {
  assert(_archive_check_enabled, "archive range check not enabled");
  log_info(gc, cds)("Clear %s archive regions in map: [" PTR_FORMAT ", " PTR_FORMAT "]",
                    open ? "open" : "closed",
                    p2i(range.start()),
                    p2i(range.last()));
  if (open) {
    _open_archive_region_map.set_by_address(range, false);
  } else {
    _closed_archive_region_map.set_by_address(range, false);
  }
}

// Check if an object is in a closed archive region using the _archive_region_map.
inline bool G1ArchiveAllocator::in_closed_archive_range(oop object) {
  // This is the out-of-line part of is_closed_archive_object test, done separately
  // to avoid additional performance impact when the check is not enabled.
  return _closed_archive_region_map.get_by_address((HeapWord*)object);
}

inline bool G1ArchiveAllocator::in_open_archive_range(oop object) {
  return _open_archive_region_map.get_by_address((HeapWord*)object);
}

// Check if archive object checking is enabled, to avoid calling in_open/closed_archive_range
// unnecessarily.
inline bool G1ArchiveAllocator::archive_check_enabled() {
  return _archive_check_enabled;
}

inline bool G1ArchiveAllocator::is_closed_archive_object(oop object) {
  return (archive_check_enabled() && in_closed_archive_range(object));
}

inline bool G1ArchiveAllocator::is_open_archive_object(oop object) {
  return (archive_check_enabled() && in_open_archive_range(object));
}

inline bool G1ArchiveAllocator::is_archive_object(oop object) {
  return (archive_check_enabled() && (in_closed_archive_range(object) ||
                                      in_open_archive_range(object)));
}

#endif // SHARE_VM_GC_G1_G1ALLOCATOR_HPP
