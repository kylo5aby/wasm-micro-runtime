(module
  ;; Import memory for test data
  (memory 1)

  ;; Test function: Basic shuffle operation
  ;; Tests standard lane reordering with mixed vector selection
  (func (export "test_basic_shuffle") (result i32)
    ;; First vector: [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
    v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
    ;; Second vector: [17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32]
    v128.const i8x16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
    ;; Shuffle mask: Select lanes [0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23]
    ;; Expected result: [1,17,2,18,3,19,4,20,5,21,6,22,7,23]
    i8x16.shuffle 0 16 1 17 2 18 3 19 4 20 5 21 6 22 7 23

    ;; Validate first lane (should be 1)
    i8x16.extract_lane_s 0
    i32.const 1
    i32.eq
    if (result i32)
      ;; Re-execute for validation of second lane (should be 17)
      v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
      v128.const i8x16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
      i8x16.shuffle 0 16 1 17 2 18 3 19 4 20 5 21 6 22 7 23
      i8x16.extract_lane_s 1
      i32.const 17
      i32.eq
      if (result i32)
        ;; Validate lane 4 (should be 5)
        v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
        v128.const i8x16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
        i8x16.shuffle 0 16 1 17 2 18 3 19 4 20 5 21 6 22 7 23
        i8x16.extract_lane_s 8
        i32.const 5
        i32.eq
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: Boundary lane indices
  ;; Tests valid boundary indices (0, 15, 16, 31)
  (func (export "test_boundary_lanes") (result i32)
    ;; First vector: [10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160]
    v128.const i8x16 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160
    ;; Second vector: [170,180,190,200,210,220,230,240,250,11,12,13,14,15,17,19]
    v128.const i8x16 170 180 190 200 210 220 230 240 250 11 12 13 14 15 17 19
    ;; Shuffle mask: Use boundary indices [0,15,16,31,0,15,16,31,0,15,16,31,0,15,16,31]
    ;; Expected result: [10,160,170,19,10,160,170,19,10,160,170,19,10,160,170,19]
    i8x16.shuffle 0 15 16 31 0 15 16 31 0 15 16 31 0 15 16 31

    ;; Validate first boundary (index 0 -> value 10)
    i8x16.extract_lane_u 0
    i32.const 10
    i32.eq
    if (result i32)
      ;; Re-execute for validation of last lane from first vector (index 15 -> value 160)
      v128.const i8x16 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160
      v128.const i8x16 170 180 190 200 210 220 230 240 250 11 12 13 14 15 17 19
      i8x16.shuffle 0 15 16 31 0 15 16 31 0 15 16 31 0 15 16 31
      i8x16.extract_lane_u 1
      i32.const 160
      i32.eq
      if (result i32)
        ;; Validate first lane from second vector (index 16 -> value 170)
        v128.const i8x16 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160
        v128.const i8x16 170 180 190 200 210 220 230 240 250 11 12 13 14 15 17 19
        i8x16.shuffle 0 15 16 31 0 15 16 31 0 15 16 31 0 15 16 31
        i8x16.extract_lane_u 2
        i32.const 170
        i32.eq
        if (result i32)
          ;; Validate last boundary (index 31 -> value 19)
          v128.const i8x16 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160
          v128.const i8x16 170 180 190 200 210 220 230 240 250 11 12 13 14 15 17 19
          i8x16.shuffle 0 15 16 31 0 15 16 31 0 15 16 31 0 15 16 31
          i8x16.extract_lane_u 3
          i32.const 19
          i32.eq
        else
          i32.const 0
        end
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: Identity shuffle operations
  ;; Tests identity transformations for both vectors
  (func (export "test_identity_shuffle") (result i32)
    ;; First vector: [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
    v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
    ;; Second vector: [17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32]
    v128.const i8x16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
    ;; Identity shuffle for first vector: [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]
    i8x16.shuffle 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

    ;; Validate first lane preserved (should be 1)
    i8x16.extract_lane_s 0
    i32.const 1
    i32.eq
    if (result i32)
      ;; Re-execute for validation of last lane preserved (should be 16)
      v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
      v128.const i8x16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
      i8x16.shuffle 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
      i8x16.extract_lane_s 15
      i32.const 16
      i32.eq
      if (result i32)
        ;; Test second vector identity: [16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31]
        v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
        v128.const i8x16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
        i8x16.shuffle 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
        i8x16.extract_lane_s 0
        i32.const 17
        i32.eq
        if (result i32)
          ;; Validate last lane of second vector identity (should be 32)
          v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
          v128.const i8x16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
          i8x16.shuffle 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
          i8x16.extract_lane_s 15
          i32.const 32
          i32.eq
        else
          i32.const 0
        end
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: Zero vector handling
  ;; Tests shuffle operations with zero vectors and zero-selecting patterns
  (func (export "test_zero_vectors") (result i32)
    ;; All-zero first vector
    v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    ;; All-zero second vector
    v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    ;; Any shuffle mask should produce all zeros
    i8x16.shuffle 5 21 7 23 1 17 3 19 9 25 11 27 13 29 15 31

    ;; Validate all lanes are zero
    i8x16.extract_lane_s 0
    i32.const 0
    i32.eq
    if (result i32)
      ;; Re-execute for validation of different lane
      v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
      v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
      i8x16.shuffle 5 21 7 23 1 17 3 19 9 25 11 27 13 29 15 31
      i8x16.extract_lane_s 8
      i32.const 0
      i32.eq
      if (result i32)
        ;; Test non-zero vectors with zero-selecting shuffle (selecting index 0 repeatedly)
        v128.const i8x16 100 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
        v128.const i8x16 200 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
        ;; Select first lanes repeatedly: [0,0,0,0,16,16,16,16,0,0,0,0,16,16,16,16]
        i8x16.shuffle 0 0 0 0 16 16 16 16 0 0 0 0 16 16 16 16
        ;; Should get pattern of [100,100,100,100,200,200,200,200,100,100,100,100,200,200,200,200]
        i8x16.extract_lane_s 0
        i32.const 100
        i32.eq
        if (result i32)
          ;; Validate second vector selection
          v128.const i8x16 100 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
          v128.const i8x16 200 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
          i8x16.shuffle 0 0 0 0 16 16 16 16 0 0 0 0 16 16 16 16
          i8x16.extract_lane_u 4
          i32.const 200
          i32.eq
        else
          i32.const 0
        end
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: Extreme patterns and values
  ;; Tests with maximum values and complex shuffle patterns
  (func (export "test_extreme_patterns") (result i32)
    ;; First vector with extreme values
    v128.const i8x16 255 254 253 252 251 250 249 248 247 246 245 244 243 242 241 240
    ;; Second vector with mixed extreme values
    v128.const i8x16 1 255 2 254 3 253 4 252 5 251 6 250 7 249 8 248
    ;; Complex shuffle: complete reversal of combined vectors
    ;; Reverse order: [31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16]
    i8x16.shuffle 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16

    ;; Validate first lane (should be 248 from second vector index 15)
    i8x16.extract_lane_u 0
    i32.const 248
    i32.eq
    if (result i32)
      ;; Re-execute for validation of middle position
      v128.const i8x16 255 254 253 252 251 250 249 248 247 246 245 244 243 242 241 240
      v128.const i8x16 1 255 2 254 3 253 4 252 5 251 6 250 7 249 8 248
      i8x16.shuffle 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16
      i8x16.extract_lane_u 8
      i32.const 250
      i32.eq
      if (result i32)
        ;; Validate last position (should be 1 from second vector index 0)
        v128.const i8x16 255 254 253 252 251 250 249 248 247 246 245 244 243 242 241 240
        v128.const i8x16 1 255 2 254 3 253 4 252 5 251 6 250 7 249 8 248
        i8x16.shuffle 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16
        i8x16.extract_lane_u 15
        i32.const 1
        i32.eq
        if (result i32)
          ;; Test broadcast pattern (all lanes select maximum value from first vector)
          v128.const i8x16 255 254 253 252 251 250 249 248 247 246 245 244 243 242 241 240
          v128.const i8x16 1 255 2 254 3 253 4 252 5 251 6 250 7 249 8 248
          ;; Broadcast first lane (255) to all positions
          i8x16.shuffle 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
          i8x16.extract_lane_u 7
          i32.const 255
          i32.eq
        else
          i32.const 0
        end
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: Interleave patterns
  ;; Tests various interleaving patterns between two vectors
  (func (export "test_interleave_patterns") (result i32)
    ;; First vector: Odd numbers [1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31]
    v128.const i8x16 1 3 5 7 9 11 13 15 17 19 21 23 25 27 29 31
    ;; Second vector: Even numbers [2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32]
    v128.const i8x16 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32
    ;; Interleave pattern: [odd0,even0,odd1,even1,odd2,even2,odd3,even3,odd4,even4,odd5,even5,odd6,even6,odd7,even7]
    ;; Result should be: [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
    i8x16.shuffle 0 16 1 17 2 18 3 19 4 20 5 21 6 22 7 23

    ;; Validate first pair (should be 1,2)
    i8x16.extract_lane_s 0
    i32.const 1
    i32.eq
    if (result i32)
      v128.const i8x16 1 3 5 7 9 11 13 15 17 19 21 23 25 27 29 31
      v128.const i8x16 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32
      i8x16.shuffle 0 16 1 17 2 18 3 19 4 20 5 21 6 22 7 23
      i8x16.extract_lane_s 1
      i32.const 2
      i32.eq
      if (result i32)
        ;; Validate middle interleave (should be 5,6)
        v128.const i8x16 1 3 5 7 9 11 13 15 17 19 21 23 25 27 29 31
        v128.const i8x16 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32
        i8x16.shuffle 0 16 1 17 2 18 3 19 4 20 5 21 6 22 7 23
        i8x16.extract_lane_s 4
        i32.const 5
        i32.eq
        if (result i32)
          v128.const i8x16 1 3 5 7 9 11 13 15 17 19 21 23 25 27 29 31
          v128.const i8x16 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32
          i8x16.shuffle 0 16 1 17 2 18 3 19 4 20 5 21 6 22 7 23
          i8x16.extract_lane_s 5
          i32.const 6
          i32.eq
          if (result i32)
            ;; Validate last interleave pair (should be 15,16)
            v128.const i8x16 1 3 5 7 9 11 13 15 17 19 21 23 25 27 29 31
            v128.const i8x16 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32
            i8x16.shuffle 0 16 1 17 2 18 3 19 4 20 5 21 6 22 7 23
            i8x16.extract_lane_s 15
            i32.const 16
            i32.eq
          else
            i32.const 0
          end
        else
          i32.const 0
        end
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )
)