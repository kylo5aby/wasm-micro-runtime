(module
  ;; Import memory for module operation
  (memory (export "memory") 1)

  ;; Basic conversion function for f64x2.convert_low_i32x4_s testing
  ;; Takes 4 i32 parameters, constructs i32x4 vector, converts low lanes to f64x2
  (func (export "convert_low_i32x4_s_basic")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
        (result f64 f64)
    (local $vec v128)

    ;; Build i32x4 vector using replace_lane operations
    (i32.const 0)       ;; Dummy value for initial splat
    i32x4.splat         ;; Create initial vector [0,0,0,0]
    (local.get $lane0)  ;; Get lane 0 value
    i32x4.replace_lane 0  ;; Replace lane 0: [lane0,0,0,0]
    (local.get $lane1)  ;; Get lane 1 value
    i32x4.replace_lane 1  ;; Replace lane 1: [lane0,lane1,0,0]
    (local.get $lane2)  ;; Get lane 2 value
    i32x4.replace_lane 2  ;; Replace lane 2: [lane0,lane1,lane2,0]
    (local.get $lane3)  ;; Get lane 3 value
    i32x4.replace_lane 3  ;; Replace lane 3: [lane0,lane1,lane2,lane3]
    local.set $vec      ;; Store the complete i32x4 vector

    ;; Convert low lanes (0,1) of i32x4 to f64x2
    (local.get $vec)
    f64x2.convert_low_i32x4_s
    local.set $vec      ;; Store the f64x2 result

    ;; Extract individual f64 values to return as separate results
    (local.get $vec)
    f64x2.extract_lane 0  ;; Extract lane 0 (converted from i32 lane 0)
    (local.get $vec)
    f64x2.extract_lane 1  ;; Extract lane 1 (converted from i32 lane 1)
  )

  ;; Boundary value conversion function
  ;; Specifically designed for testing INT32_MIN/MAX boundary conversions
  (func (export "convert_low_i32x4_s_boundary")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
        (result f64 f64)
    (local $vec v128)

    ;; Build i32x4 vector with boundary values
    (i32.const 0)       ;; Dummy value for initial splat
    i32x4.splat         ;; Create initial vector [0,0,0,0]
    (local.get $lane0)  ;; Get lane 0 value (boundary value)
    i32x4.replace_lane 0  ;; Replace lane 0
    (local.get $lane1)  ;; Get lane 1 value (boundary value)
    i32x4.replace_lane 1  ;; Replace lane 1
    (local.get $lane2)  ;; Get lane 2 value (ignored)
    i32x4.replace_lane 2  ;; Replace lane 2
    (local.get $lane3)  ;; Get lane 3 value (ignored)
    i32x4.replace_lane 3  ;; Replace lane 3
    local.set $vec      ;; Store the complete i32x4 vector

    ;; Convert low lanes of i32x4 to f64x2
    (local.get $vec)
    f64x2.convert_low_i32x4_s
    local.set $vec      ;; Store the f64x2 result

    ;; Extract and return f64 results
    (local.get $vec)
    f64x2.extract_lane 0  ;; Extract converted lane 0
    (local.get $vec)
    f64x2.extract_lane 1  ;; Extract converted lane 1
  )

  ;; Lane selection validation function
  ;; Tests that only low lanes (0,1) are converted, high lanes (2,3) ignored
  (func (export "convert_low_i32x4_s_lanes")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
        (result f64 f64)
    (local $vec v128)

    ;; Build i32x4 vector with distinctive values in each lane
    (i32.const 0)       ;; Dummy value for initial splat
    i32x4.splat         ;; Create initial vector [0,0,0,0]
    (local.get $lane0)  ;; Low lane value (should convert)
    i32x4.replace_lane 0  ;; Replace lane 0
    (local.get $lane1)  ;; Low lane value (should convert)
    i32x4.replace_lane 1  ;; Replace lane 1
    (local.get $lane2)  ;; High lane value (should be ignored)
    i32x4.replace_lane 2  ;; Replace lane 2
    (local.get $lane3)  ;; High lane value (should be ignored)
    i32x4.replace_lane 3  ;; Replace lane 3
    local.set $vec      ;; Store the complete i32x4 vector

    ;; Convert only low lanes (0,1) to f64x2
    (local.get $vec)
    f64x2.convert_low_i32x4_s
    local.set $vec      ;; Store the f64x2 result

    ;; Extract converted values (only from original lanes 0,1)
    (local.get $vec)
    f64x2.extract_lane 0  ;; Extract converted lane 0
    (local.get $vec)
    f64x2.extract_lane 1  ;; Extract converted lane 1
  )

  ;; Sign preservation function
  ;; Tests proper handling of negative integers in signed conversion
  (func (export "convert_low_i32x4_s_signs")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
        (result f64 f64)
    (local $vec v128)

    ;; Build i32x4 vector emphasizing sign handling
    (i32.const 0)       ;; Dummy value for initial splat
    i32x4.splat         ;; Create initial vector [0,0,0,0]
    (local.get $lane0)  ;; Lane 0 value (may be negative)
    i32x4.replace_lane 0  ;; Replace lane 0
    (local.get $lane1)  ;; Lane 1 value (may be negative)
    i32x4.replace_lane 1  ;; Replace lane 1
    (local.get $lane2)  ;; Lane 2 value (ignored, may be any value)
    i32x4.replace_lane 2  ;; Replace lane 2
    (local.get $lane3)  ;; Lane 3 value (ignored, may be any value)
    i32x4.replace_lane 3  ;; Replace lane 3
    local.set $vec      ;; Store the complete i32x4 vector

    ;; Convert low lanes preserving sign information
    (local.get $vec)
    f64x2.convert_low_i32x4_s
    local.set $vec      ;; Store the f64x2 result

    ;; Extract signed converted values
    (local.get $vec)
    f64x2.extract_lane 0  ;; Extract signed converted lane 0
    (local.get $vec)
    f64x2.extract_lane 1  ;; Extract signed converted lane 1
  )

  ;; Stress testing function for resource management validation
  ;; Simple conversion suitable for repeated execution
  (func (export "convert_low_i32x4_s_stress")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
        (result f64 f64)
    (local $vec v128)

    ;; Simple, efficient i32x4 vector construction
    (i32.const 0)       ;; Dummy value for initial splat
    i32x4.splat         ;; Create initial vector [0,0,0,0]
    (local.get $lane0)  ;; Lane 0 value
    i32x4.replace_lane 0  ;; Replace lane 0
    (local.get $lane1)  ;; Lane 1 value
    i32x4.replace_lane 1  ;; Replace lane 1
    (local.get $lane2)  ;; Lane 2 value
    i32x4.replace_lane 2  ;; Replace lane 2
    (local.get $lane3)  ;; Lane 3 value
    i32x4.replace_lane 3  ;; Replace lane 3
    local.set $vec      ;; Store the complete i32x4 vector

    ;; Perform conversion operation
    (local.get $vec)
    f64x2.convert_low_i32x4_s
    local.set $vec      ;; Store the f64x2 result

    ;; Extract results efficiently
    (local.get $vec)
    f64x2.extract_lane 0  ;; Extract converted lane 0
    (local.get $vec)
    f64x2.extract_lane 1  ;; Extract converted lane 1
  )
)