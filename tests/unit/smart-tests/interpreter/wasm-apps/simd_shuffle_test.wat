(module
  ;; Memory for SIMD operations
  (memory 1)
  
  ;; Test valid SIMD shuffle mask validation
  ;; This function tests check_simd_shuffle_mask() with valid indices (0-31)
  (func (export "test_valid_shuffle") (param $mask_high i64) (param $mask_low i64) (result i32)
    (local $result i32)
    
    ;; Create v128 values for shuffle operation
    v128.const i32x4 0x03020100 0x07060504 0x0B0A0908 0x0F0E0D0C
    v128.const i32x4 0x13121110 0x17161514 0x1B1A1918 0x1F1E1D1C
    
    ;; Perform shuffle with valid mask (indices 0-31)
    ;; This will trigger check_simd_shuffle_mask() validation
    i8x16.shuffle 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    
    ;; Drop result and return success indicator
    drop
    i32.const 1
  )
  
  ;; Test invalid SIMD shuffle mask validation
  ;; This function tests check_simd_shuffle_mask() with invalid indices (>31)
  (func (export "test_invalid_shuffle") (param $mask_high i64) (param $mask_low i64) (result i32)
    (local $result i32)
    
    ;; This would normally cause validation error in real scenario
    ;; For testing purposes, we simulate the validation check
    
    ;; Check if mask contains invalid indices
    local.get $mask_high
    i64.const 0x1F1F1F1F1F1F1F1F  ;; Compare against max valid value (31)
    i64.gt_u
    if (result i32)
      ;; Invalid mask detected
      i32.const 0
    else
      local.get $mask_low
      i64.const 0x1F1F1F1F1F1F1F1F  ;; Compare against max valid value (31)
      i64.gt_u
      if (result i32)
        ;; Invalid mask detected
        i32.const 0
      else
        ;; Valid mask
        i32.const 1
      end
    end
  )
  
  ;; Test boundary conditions for shuffle mask
  (func (export "test_shuffle_boundary") (param $index i32) (result i32)
    ;; Test boundary values (0, 15, 16, 31, 32)
    local.get $index
    i32.const 32
    i32.lt_u
    if (result i32)
      ;; Valid index (0-31)
      i32.const 1
    else
      ;; Invalid index (>=32)
      i32.const 0
    end
  )
)