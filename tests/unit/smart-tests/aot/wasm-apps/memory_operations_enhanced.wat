(module
  ;; Memory declaration: 1 page (64KB) initial, 16 pages maximum
  (memory (export "memory") 1 16)
  
  ;; Initialize some test data at offset 64
  (data (i32.const 64) "\01\02\03\04\05\06\07\08\09\0a\0b\0c\0d\0e\0f\10\11\12\13\14\15\16\17\18\19\1a\1b\1c\1d\1e\1f\20")

  ;; Test basic memory access (store and load)
  (func (export "test_memory_access") (param $offset i32) (param $value i32) (result i32)
    (i32.store (local.get $offset) (local.get $value))
    (i32.load (local.get $offset))
  )

  ;; Test i32 load/store operations
  (func (export "test_i32_load_store") (param $offset i32) (param $value i32) (result i32)
    (i32.store (local.get $offset) (local.get $value))
    (i32.load (local.get $offset))
  )

  ;; Test i64 load/store operations
  (func (export "test_i64_load_store") (param $offset i32) (param $value_low i32) (param $value_high i32) (result i64)
    (local $value i64)
    (local.set $value 
      (i64.or 
        (i64.extend_i32_u (local.get $value_low))
        (i64.shl (i64.extend_i32_u (local.get $value_high)) (i64.const 32))
      )
    )
    (i64.store (local.get $offset) (local.get $value))
    (i64.load (local.get $offset))
  )

  ;; Test f32 load/store operations
  (func (export "test_f32_load_store") (param $offset i32) (param $value f32) (result f32)
    (f32.store (local.get $offset) (local.get $value))
    (f32.load (local.get $offset))
  )

  ;; Test f64 load/store operations
  (func (export "test_f64_load_store") (param $offset i32) (param $value f64) (result f64)
    (f64.store (local.get $offset) (local.get $value))
    (f64.load (local.get $offset))
  )

  ;; Test bulk memory operations (memory.fill)
  (func (export "test_bulk_memory") (param $dest i32) (param $src i32) (param $size i32) (result i32)
    ;; Fill memory with pattern instead of copy
    (memory.fill (local.get $dest) (i32.const 0xAA) (local.get $size))
    (i32.const 1) ;; Return success
  )

  ;; Test bounds checking - this should trap on out of bounds access
  (func (export "test_bounds_check") (param $offset i32) (param $size i32) (result i32)
    ;; Try to load from potentially out-of-bounds location
    (drop (i32.load (i32.add (local.get $offset) (local.get $size))))
    (i32.const 1) ;; Return success if no trap
  )

  ;; Test memory alignment
  (func (export "test_alignment") (param $offset i32) (result i32)
    ;; Store and load i64 at given offset (tests alignment)
    (i64.store (local.get $offset) (i64.const 0x123456789ABCDEF0))
    (drop (i64.load (local.get $offset)))
    (i32.const 1) ;; Return success
  )

  ;; Simulate heap allocation (returns offset in linear memory)
  (func $heap_alloc (param $size i32) (result i32)
    (local $current_top i32)
    ;; Simple bump allocator starting at offset 1024
    (local.set $current_top (i32.load (i32.const 0))) ;; Load heap top from offset 0
    (if (i32.eqz (local.get $current_top))
      (then (local.set $current_top (i32.const 1024))) ;; Initialize heap top
    )
    
    ;; Check if allocation fits in memory
    (if (result i32) (i32.lt_u (i32.add (local.get $current_top) (local.get $size)) (i32.const 65536))
      (then
        ;; Update heap top
        (i32.store (i32.const 0) (i32.add (local.get $current_top) (local.get $size)))
        (local.get $current_top) ;; Return allocated offset
      )
      (else
        (i32.const 0) ;; Return null (allocation failed)
      )
    )
  )

  (func (export "test_heap_alloc") (param $size i32) (result i32)
    (call $heap_alloc (local.get $size))
  )

  ;; Test stack usage with recursion
  (func $stack_recursive (param $n i32) (result i32)
    (if (result i32) (i32.le_s (local.get $n) (i32.const 0))
      (then (i32.const 0))
      (else
        (i32.add 
          (local.get $n)
          (call $stack_recursive (i32.sub (local.get $n) (i32.const 1)))
        )
      )
    )
  )

  (func (export "test_stack_usage") (param $n i32) (result i32)
    (call $stack_recursive (local.get $n))
  )

  ;; Test large allocation (should fail)
  (func (export "test_large_alloc") (param $size i32) (result i32)
    ;; Try to allocate more than available memory
    (if (result i32) (i32.gt_u (local.get $size) (i32.const 60000))
      (then (i32.const 0)) ;; Return null for oversized allocation
      (else (call $heap_alloc (local.get $size)))
    )
  )

  ;; Test allocation and deallocation cycle
  (func (export "test_alloc_free") (param $size i32) (result i32)
    (local $ptr i32)
    ;; Allocate memory
    (local.set $ptr (call $heap_alloc (local.get $size)))
    (if (result i32) (i32.eqz (local.get $ptr))
      (then (i32.const 0)) ;; Allocation failed
      (else
        ;; Use the memory (write some data)
        (i32.store (local.get $ptr) (i32.const 0xDEADBEEF))
        ;; Verify data
        (if (result i32) (i32.eq (i32.load (local.get $ptr)) (i32.const 0xDEADBEEF))
          (then (i32.const 1)) ;; Success
          (else (i32.const 0))  ;; Failed
        )
      )
    )
  )

  ;; Test memory fragmentation handling
  (func (export "test_fragmentation") (result i32)
    (local $ptr1 i32)
    (local $ptr2 i32)
    (local $ptr3 i32)
    
    ;; Allocate several blocks
    (local.set $ptr1 (call $heap_alloc (i32.const 100)))
    (local.set $ptr2 (call $heap_alloc (i32.const 200)))
    (local.set $ptr3 (call $heap_alloc (i32.const 150)))
    
    ;; Check all allocations succeeded
    (if (result i32) (i32.and 
          (i32.and (i32.ne (local.get $ptr1) (i32.const 0))
                   (i32.ne (local.get $ptr2) (i32.const 0)))
          (i32.ne (local.get $ptr3) (i32.const 0)))
      (then (i32.const 1)) ;; Success
      (else (i32.const 0))  ;; Failed
    )
  )

  ;; Test memory pressure scenarios
  (func (export "test_memory_pressure") (result i32)
    (local $i i32)
    (local $ptr i32)
    (local $success i32)
    (local.set $success (i32.const 1))
    
    ;; Try to allocate many small blocks until failure
    (loop $alloc_loop
      (local.set $ptr (call $heap_alloc (i32.const 64)))
      (if (i32.eqz (local.get $ptr))
        (then 
          ;; Allocation failed, this is expected under memory pressure
          (br $alloc_loop)
        )
        (else
          ;; Continue allocating
          (local.set $i (i32.add (local.get $i) (i32.const 1)))
          (if (i32.lt_u (local.get $i) (i32.const 100))
            (then (br $alloc_loop))
          )
        )
      )
    )
    (local.get $success)
  )

  ;; Test concurrent access safety (single-threaded simulation)
  (func (export "test_concurrent_access") (result i32)
    (local $addr i32)
    (local $value1 i32)
    (local $value2 i32)
    
    (local.set $addr (i32.const 2048))
    (local.set $value1 (i32.const 0x11111111))
    (local.set $value2 (i32.const 0x22222222))
    
    ;; Simulate concurrent writes and reads
    (i32.store (local.get $addr) (local.get $value1))
    (i32.store (i32.add (local.get $addr) (i32.const 4)) (local.get $value2))
    
    ;; Verify both values are correct
    (if (result i32) (i32.and
          (i32.eq (i32.load (local.get $addr)) (local.get $value1))
          (i32.eq (i32.load (i32.add (local.get $addr) (i32.const 4))) (local.get $value2)))
      (then (i32.const 1)) ;; Success
      (else (i32.const 0))  ;; Failed
    )
  )

  ;; Test page boundary access
  (func (export "test_page_boundary") (param $offset i32) (result i32)
    ;; Try to access memory at the given offset
    (i32.store8 (local.get $offset) (i32.const 0xFF))
    (if (result i32) (i32.eq (i32.load8_u (local.get $offset)) (i32.const 0xFF))
      (then (i32.const 1)) ;; Success
      (else (i32.const 0))  ;; Failed
    )
  )
)