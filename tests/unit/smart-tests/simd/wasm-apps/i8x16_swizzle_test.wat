(module
  ;; Import memory for test data
  (memory 1)

  ;; Test function: Basic swizzle operation
  ;; Tests standard lane rearrangement with typical indices
  (func (export "test_basic_swizzle") (result i32)
    ;; Source vector: [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
    v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
    ;; Index vector: [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15] - identity
    v128.const i8x16 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    ;; Perform swizzle operation
    i8x16.swizzle
    ;; Expected result: [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
    ;; Validate first lane (should be 1)
    i8x16.extract_lane_s 0
    i32.const 1
    i32.eq
    if (result i32)
      ;; Validate last lane (should be 16)
      v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
      v128.const i8x16 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      i8x16.swizzle
      i8x16.extract_lane_s 15
      i32.const 16
      i32.eq
    else
      i32.const 0
    end
  )

  ;; Test function: Boundary indices validation
  ;; Tests valid (0-15) and invalid (>=16) indices
  (func (export "test_boundary_indices") (result i32)
    ;; Source vector: [10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160]
    v128.const i8x16 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160
    ;; Index vector: [0,15,16,255,1,14,17,254,2,13,18,253,3,12,19,252]
    ;; Valid: 0->10, 15->160, 1->20, 14->150, 2->30, 13->140, 3->40, 12->130
    ;; Invalid: 16->0, 255->0, 17->0, 254->0, 18->0, 253->0, 19->0, 252->0
    v128.const i8x16 0 15 16 255 1 14 17 254 2 13 18 253 3 12 19 252
    i8x16.swizzle
    ;; Validate first lane (index 0 -> value 10)
    i8x16.extract_lane_s 0
    i32.const 10
    i32.eq
    if (result i32)
      ;; Validate invalid index produces zero (lane 2: index 16 -> 0)
      v128.const i8x16 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160
      v128.const i8x16 0 15 16 255 1 14 17 254 2 13 18 253 3 12 19 252
      i8x16.swizzle
      i8x16.extract_lane_s 2
      i32.const 0
      i32.eq
    else
      i32.const 0
    end
  )

  ;; Test function: Identity operation
  ;; Tests that indices [0,1,2,...,15] return original vector
  (func (export "test_identity_operation") (result i32)
    ;; Source vector: [11,22,33,44,55,66,77,88,99,100,101,102,31,41,51,61] - all values ≤ 127
    v128.const i8x16 11 22 33 44 55 66 77 88 99 100 101 102 31 41 51 61
    ;; Identity indices: [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]
    v128.const i8x16 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    i8x16.swizzle
    ;; Validate multiple lanes for identity
    ;; Check lane 0 (should be 11)
    i8x16.extract_lane_s 0
    i32.const 11
    i32.eq
    if (result i32)
      ;; Check lane 7 (should be 88)
      v128.const i8x16 11 22 33 44 55 66 77 88 99 100 101 102 31 41 51 61
      v128.const i8x16 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      i8x16.swizzle
      i8x16.extract_lane_s 7
      i32.const 88
      i32.eq
      if (result i32)
        ;; Check lane 15 (should be 61)
        v128.const i8x16 11 22 33 44 55 66 77 88 99 100 101 102 31 41 51 61
        v128.const i8x16 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
        i8x16.swizzle
        i8x16.extract_lane_s 15
        i32.const 61
        i32.eq
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: Reverse operation
  ;; Tests that indices [15,14,13,...,0] reverse the vector
  (func (export "test_reverse_operation") (result i32)
    ;; Source vector: [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
    v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
    ;; Reverse indices: [15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0]
    v128.const i8x16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
    i8x16.swizzle
    ;; Expected result: [16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1]
    ;; Validate first lane (should be 16 from source[15])
    i8x16.extract_lane_s 0
    i32.const 16
    i32.eq
    if (result i32)
      ;; Validate middle lane (should be 9 from source[7])
      v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
      v128.const i8x16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
      i8x16.swizzle
      i8x16.extract_lane_s 8
      i32.const 8
      i32.eq
      if (result i32)
        ;; Validate last lane (should be 1 from source[0])
        v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
        v128.const i8x16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
        i8x16.swizzle
        i8x16.extract_lane_s 15
        i32.const 1
        i32.eq
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: Zero vectors handling
  ;; Tests behavior with zero source vector and zero indices
  (func (export "test_zero_vectors") (result i32)
    ;; Test 1: Zero source vector with valid indices
    v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    v128.const i8x16 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    i8x16.swizzle
    ;; All lanes should be zero
    i8x16.extract_lane_s 0
    i32.const 0
    i32.eq
    if (result i32)
      ;; Test 2: Non-zero source with all-zero indices (selects first lane repeatedly)
      v128.const i8x16 42 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
      i8x16.swizzle
      ;; All lanes should be 42 (first lane of source)
      i8x16.extract_lane_s 0
      i32.const 42
      i32.eq
      if (result i32)
        ;; Check another lane
        v128.const i8x16 42 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
        v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        i8x16.swizzle
        i8x16.extract_lane_s 7
        i32.const 42
        i32.eq
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: All invalid indices
  ;; Tests that indices >=16 produce all-zero result
  (func (export "test_all_invalid_indices") (result i32)
    ;; Source vector: [10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160]
    v128.const i8x16 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160
    ;; All invalid indices: [16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31]
    v128.const i8x16 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
    i8x16.swizzle
    ;; All lanes should be zero
    ;; Check first lane
    i8x16.extract_lane_s 0
    i32.const 0
    i32.eq
    if (result i32)
      ;; Check middle lane
      v128.const i8x16 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160
      v128.const i8x16 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
      i8x16.swizzle
      i8x16.extract_lane_s 8
      i32.const 0
      i32.eq
      if (result i32)
        ;; Check last lane
        v128.const i8x16 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160
        v128.const i8x16 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
        i8x16.swizzle
        i8x16.extract_lane_s 15
        i32.const 0
        i32.eq
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )
)