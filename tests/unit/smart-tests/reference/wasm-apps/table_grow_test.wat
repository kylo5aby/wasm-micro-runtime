(module
  ;; Function type definitions
  (type $void_func (func))
  (type $test_func (func (param i32 i32 i32) (result i32)))
  (type $size_func (func (param i32) (result i32)))
  (type $simple_func (func (param i32) (result i32)))

  ;; Test function definitions (placeholder for initialization)
  (func $dummy_func (type $void_func))
  (func $init_func (type $void_func))

  ;; Tables for testing table.grow opcode
  ;; Table 0: funcref table with initial size 5, maximum size 20
  (table $test_table 5 20 funcref)

  ;; Table 1: funcref table with initial size 2, maximum size 10
  (table $small_table 2 10 funcref)

  ;; Table 2: funcref table with initial size 1, maximum size 5 (for overflow tests)
  (table $limit_table 1 5 funcref)

  ;; Initialize tables with function references
  (elem (i32.const 0) $dummy_func $init_func $dummy_func $init_func $dummy_func)
  (elem (table $small_table) (i32.const 0) $dummy_func $init_func)
  (elem (table $limit_table) (i32.const 0) $dummy_func)

  ;; Main test function: table.grow with specified parameters
  ;; Parameters: table_idx, growth_size, init_func_idx
  ;; Returns: original table size or -1 on failure
  (func $test_table_grow (type $test_func)
    (param $table_idx i32) (param $growth_size i32) (param $init_func_idx i32)
    (result i32)

    ;; Select table and perform growth based on table_idx
    local.get $table_idx
    i32.const 0
    i32.eq
    if (result i32)
      ;; Table 0: Use null or function reference based on init_func_idx
      local.get $init_func_idx
      i32.const -1
      i32.eq
      if (result i32)
        ref.null func
        local.get $growth_size
        table.grow $test_table
      else
        ref.func $dummy_func
        local.get $growth_size
        table.grow $test_table
      end
    else
      local.get $table_idx
      i32.const 1
      i32.eq
      if (result i32)
        ;; Table 1: Use null or function reference based on init_func_idx
        local.get $init_func_idx
        i32.const -1
        i32.eq
        if (result i32)
          ref.null func
          local.get $growth_size
          table.grow $small_table
        else
          ref.func $init_func
          local.get $growth_size
          table.grow $small_table
        end
      else
        ;; Table 2: Default case
        local.get $init_func_idx
        i32.const -1
        i32.eq
        if (result i32)
          ref.null func
          local.get $growth_size
          table.grow $limit_table
        else
          ref.func $dummy_func
          local.get $growth_size
          table.grow $limit_table
        end
      end
    end
  )

  ;; Get current size of specified table
  ;; Parameter: table_idx
  ;; Returns: current table size
  (func $get_table_size (type $size_func)
    (param $table_idx i32)
    (result i32)

    local.get $table_idx
    i32.const 0
    i32.eq
    if (result i32)
      table.size $test_table
    else
      local.get $table_idx
      i32.const 1
      i32.eq
      if (result i32)
        table.size $small_table
      else
        table.size $limit_table
      end
    end
  )

  ;; Test zero growth operation
  ;; Parameter: table_idx
  ;; Returns: result of zero growth (should be current size)
  (func $test_zero_growth (type $simple_func)
    (param $table_idx i32)
    (result i32)

    local.get $table_idx
    i32.const 0
    i32.eq
    if (result i32)
      ref.null func
      i32.const 0
      table.grow $test_table
    else
      local.get $table_idx
      i32.const 1
      i32.eq
      if (result i32)
        ref.null func
        i32.const 0
        table.grow $small_table
      else
        ref.null func
        i32.const 0
        table.grow $limit_table
      end
    end
  )

  ;; Test growth that should cause overflow/exceed maximum size
  ;; Parameter: table_idx
  ;; Returns: -1 if growth fails as expected
  (func $test_overflow_growth (type $simple_func)
    (param $table_idx i32)
    (result i32)

    local.get $table_idx
    i32.const 0
    i32.eq
    if (result i32)
      ;; Try to grow table 0 beyond maximum (20)
      ref.null func
      i32.const 50  ;; Large growth that should exceed max size
      table.grow $test_table
    else
      local.get $table_idx
      i32.const 1
      i32.eq
      if (result i32)
        ;; Try to grow table 1 beyond maximum (10)
        ref.null func
        i32.const 25  ;; Large growth that should exceed max size
        table.grow $small_table
      else
        ;; Try to grow table 2 beyond maximum (5)
        ref.null func
        i32.const 15  ;; Large growth that should exceed max size
        table.grow $limit_table
      end
    end
  )

  ;; Test arithmetic overflow in growth calculation
  ;; Parameter: table_idx (ignored for this test)
  ;; Returns: -1 if overflow handling works correctly
  (func $test_arithmetic_overflow (type $simple_func)
    (param $table_idx i32)
    (result i32)

    ;; Try to grow with maximum possible i32 value
    ;; This should trigger overflow protection
    ref.null func
    i32.const 2147483647  ;; INT32_MAX
    table.grow $test_table
  )

  ;; Test null reference initialization specifically
  ;; Returns: original size on success, -1 on failure
  (func $test_null_init (type $simple_func)
    (param $table_idx i32)
    (result i32)

    ;; Grow table with explicit null reference
    ref.null func
    i32.const 1
    table.grow $test_table
  )

  ;; Test function reference initialization
  ;; Returns: original size on success, -1 on failure
  (func $test_func_init (type $simple_func)
    (param $table_idx i32)
    (result i32)

    ;; Grow table with function reference
    ref.func $init_func
    i32.const 1
    table.grow $test_table
  )

  ;; Export test functions for C++ test harness
  (export "test_table_grow" (func $test_table_grow))
  (export "get_table_size" (func $get_table_size))
  (export "test_zero_growth" (func $test_zero_growth))
  (export "test_overflow_growth" (func $test_overflow_growth))
  (export "test_arithmetic_overflow" (func $test_arithmetic_overflow))
  (export "test_null_init" (func $test_null_init))
  (export "test_func_init" (func $test_func_init))

  ;; Export functions for reference type testing
  (export "dummy_func" (func $dummy_func))
  (export "init_func" (func $init_func))
)