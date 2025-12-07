(module
  ;; Test functions for drop opcode validation

  ;; Test dropping i32 value - should return 42 (preserved value)
  (func (export "test_drop_i32") (result i32)
    i32.const 42      ;; Value to be returned
    i32.const 999     ;; Value to be dropped
    drop              ;; Drop the 999, keep 42
  )

  ;; Test dropping i64 value - should return 64 (preserved value)
  (func (export "test_drop_i64") (result i32)
    i32.const 64            ;; Return value
    i64.const 999999999999  ;; Large i64 value to be dropped
    drop                    ;; Drop the i64, keep i32
  )

  ;; Test dropping f32 value - should return 32 (preserved value)
  (func (export "test_drop_f32") (result i32)
    i32.const 32       ;; Return value
    f32.const 3.14159  ;; f32 value to be dropped
    drop               ;; Drop the f32, keep i32
  )

  ;; Test dropping f64 value - should return 64 (preserved value)
  (func (export "test_drop_f64") (result i32)
    i32.const 64       ;; Return value
    f64.const 2.71828  ;; f64 value to be dropped
    drop               ;; Drop the f64, keep i32
  )

  ;; Test sequential drops - should return 100 (bottom stack value)
  (func (export "test_sequential_drops") (result i32)
    i32.const 100      ;; Bottom value - should be preserved
    i32.const 200      ;; Will be dropped
    i32.const 300      ;; Will be dropped
    i32.const 400      ;; Will be dropped
    drop               ;; Drop 400
    drop               ;; Drop 300
    drop               ;; Drop 200
                       ;; Return 100
  )

  ;; Test dropping INT32_MAX value
  (func (export "test_drop_i32_max") (result i32)
    i32.const 1           ;; Success indicator
    i32.const 2147483647  ;; INT32_MAX
    drop                  ;; Drop INT32_MAX
  )

  ;; Test dropping INT32_MIN value
  (func (export "test_drop_i32_min") (result i32)
    i32.const 1           ;; Success indicator
    i32.const -2147483648 ;; INT32_MIN
    drop                  ;; Drop INT32_MIN
  )

  ;; Test dropping large i64 value
  (func (export "test_drop_i64_large") (result i32)
    i32.const 1                    ;; Success indicator
    i64.const 9223372036854775807  ;; Large i64 value
    drop                           ;; Drop large i64
  )

  ;; Test dropping extreme float values
  (func (export "test_drop_float_extreme") (result i32)
    i32.const 1              ;; Success indicator
    f32.const 3.4028235e+38  ;; FLT_MAX
    f64.const 1.7976931348623157e+308  ;; DBL_MAX
    drop                     ;; Drop DBL_MAX
    drop                     ;; Drop FLT_MAX
  )

  ;; Test dropping NaN values
  (func (export "test_drop_nan") (result i32)
    i32.const 1       ;; Success indicator
    f32.const nan     ;; f32 NaN
    f64.const nan     ;; f64 NaN
    drop              ;; Drop f64 NaN
    drop              ;; Drop f32 NaN
  )

  ;; Test dropping infinity values
  (func (export "test_drop_infinity") (result i32)
    i32.const 1       ;; Success indicator
    f32.const inf     ;; f32 positive infinity
    f32.const -inf    ;; f32 negative infinity
    f64.const inf     ;; f64 positive infinity
    f64.const -inf    ;; f64 negative infinity
    drop              ;; Drop f64 -inf
    drop              ;; Drop f64 +inf
    drop              ;; Drop f32 -inf
    drop              ;; Drop f32 +inf
  )

  ;; Test dropping signed zero values
  (func (export "test_drop_signed_zero") (result i32)
    i32.const 1       ;; Success indicator
    f32.const 0.0     ;; f32 positive zero
    f32.const -0.0    ;; f32 negative zero
    f64.const 0.0     ;; f64 positive zero
    f64.const -0.0    ;; f64 negative zero
    drop              ;; Drop f64 -0.0
    drop              ;; Drop f64 +0.0
    drop              ;; Drop f32 -0.0
    drop              ;; Drop f32 +0.0
  )

  ;; Test drop in loop context
  (func (export "test_drop_in_loop") (result i32)
    (local $i i32)
    (local $count i32)

    i32.const 0
    local.set $count

    i32.const 0
    local.set $i

    loop $loop
      local.get $i
      i32.const 5
      i32.lt_s
      if
        i32.const 999     ;; Value to drop in each iteration
        drop              ;; Drop the value

        local.get $count
        i32.const 1
        i32.add
        local.set $count

        local.get $i
        i32.const 1
        i32.add
        local.set $i
        br $loop
      end
    end

    local.get $count      ;; Return count (should be 5)
  )

  ;; Test drop in conditional branch
  (func (export "test_drop_in_branch") (result i32)
    i32.const 1           ;; Condition value
    if (result i32)
      i32.const 1         ;; Return value for true branch
      i32.const 999       ;; Value to drop in true branch
      drop                ;; Drop 999
    else
      i32.const 0         ;; Return value for false branch
      i32.const 777       ;; Value to drop in false branch
      drop                ;; Drop 777
    end
  )

  ;; Helper function for testing drops with function calls
  (func $helper_function (result i32)
    i32.const 42          ;; Actual return value
    i32.const 999         ;; Value to be dropped by caller
    drop                  ;; Drop 999, return 42
  )

  ;; Test drop with function calls
  (func (export "test_drop_with_calls") (result i32)
    call $helper_function ;; Call helper (returns 42)
    i32.const 777         ;; Additional value
    drop                  ;; Drop 777
                          ;; Return 42 from helper
  )
)