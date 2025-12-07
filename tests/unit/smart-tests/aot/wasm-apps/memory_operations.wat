(module
  ;; Memory operations test module for AOT testing
  ;; Memory: 2 pages (128KB) with maximum 10 pages
  (memory 2 10)
  
  ;; Export memory for external access
  (export "memory" (memory 0))
  
  ;; Initialize some data in memory
  (data (i32.const 0) "Hello, AOT World!")
  (data (i32.const 32) "\00\01\02\03\04\05\06\07\08\09\0a\0b\0c\0d\0e\0f")
  
  ;; Memory grow function - tests memory growth
  (func $grow_memory (param $pages i32) (result i32)
    local.get $pages
    memory.grow
  )
  (export "grow_memory" (func $grow_memory))
  
  ;; Get current memory size
  (func $memory_size (result i32)
    memory.size
  )
  (export "memory_size" (func $memory_size))
  
  ;; Test i32 load/store operations
  (func $test_i32_load_store (param $addr i32) (param $value i32) (result i32)
    ;; Store i32 value
    local.get $addr
    local.get $value
    i32.store
    
    ;; Load and return i32 value
    local.get $addr
    i32.load
  )
  (export "test_i32_load_store" (func $test_i32_load_store))
  
  ;; Test i64 load/store operations
  (func $test_i64_load_store (param $addr i32) (param $value i64) (result i64)
    ;; Store i64 value
    local.get $addr
    local.get $value
    i64.store
    
    ;; Load and return i64 value
    local.get $addr
    i64.load
  )
  (export "test_i64_load_store" (func $test_i64_load_store))
  
  ;; Test f32 load/store operations
  (func $test_f32_load_store (param $addr i32) (param $value f32) (result f32)
    ;; Store f32 value
    local.get $addr
    local.get $value
    f32.store
    
    ;; Load and return f32 value
    local.get $addr
    f32.load
  )
  (export "test_f32_load_store" (func $test_f32_load_store))
  
  ;; Test f64 load/store operations
  (func $test_f64_load_store (param $addr i32) (param $value f64) (result f64)
    ;; Store f64 value
    local.get $addr
    local.get $value
    f64.store
    
    ;; Load and return f64 value
    local.get $addr
    f64.load
  )
  (export "test_f64_load_store" (func $test_f64_load_store))
  
  ;; Test memory copy operation
  (func $test_memory_copy (param $dest i32) (param $src i32) (param $size i32)
    local.get $dest
    local.get $src
    local.get $size
    memory.copy
  )
  (export "test_memory_copy" (func $test_memory_copy))
  
  ;; Test memory fill operation
  (func $test_memory_fill (param $addr i32) (param $value i32) (param $size i32)
    local.get $addr
    local.get $value
    local.get $size
    memory.fill
  )
  (export "test_memory_fill" (func $test_memory_fill))
  
  ;; Test out of bounds access (should trap)
  (func $test_out_of_bounds (param $addr i32) (result i32)
    local.get $addr
    i32.load
  )
  (export "test_out_of_bounds" (func $test_out_of_bounds))
  
  ;; Read string from memory
  (func $read_string (param $addr i32) (param $len i32) (result i32)
    ;; Simple checksum of string bytes
    (local $i i32)
    (local $sum i32)
    
    i32.const 0
    local.set $sum
    i32.const 0
    local.set $i
    
    (loop $loop
      local.get $i
      local.get $len
      i32.ge_u
      if
        br 1
      end
      
      local.get $sum
      local.get $addr
      local.get $i
      i32.add
      i32.load8_u
      i32.add
      local.set $sum
      
      local.get $i
      i32.const 1
      i32.add
      local.set $i
      br $loop
    )
    
    local.get $sum
  )
  (export "read_string" (func $read_string))
)