(module
  ;; Linear memory with 1 page (64KB) for testing
  (memory (export "memory") 1 1)

  ;; Initialize memory with test patterns for v128.load16_lane validation
  ;; Sequential 16-bit values for comprehensive testing (little-endian)
  (data (i32.const 0x00) "\00\01\00\02\00\03\00\04\00\05\00\06\00\07\00\08")  ;; 0x0100-0x0800

  ;; Test values for basic functionality
  (data (i32.const 0x10) "\34\12")                     ;; Test value 0x1234 at address 0x10
  (data (i32.const 0x18) "\78\56")                     ;; Test value 0x5678 at address 0x18
  (data (i32.const 0x1E) "\CD\AB")                     ;; Test value 0xABCD at address 0x1E

  ;; Test patterns for unaligned access (odd addresses)
  (data (i32.const 0x20) "\12\21\43\43\65\65\87\87\A9")  ;; Unaligned test data

  ;; Test value for lane preservation test
  (data (i32.const 0x30) "\99\99")                     ;; Test value 0x9999 at address 0x30

  ;; Boundary test data
  (data (i32.const 0x1000) "\34\12")                   ;; Page boundary test at 0x1000
  (data (i32.const 0xFFFE) "\FF\EE")                   ;; Near page boundary

  ;; Test pattern for extreme 16-bit values
  (data (i32.const 0x200) "\00\00\FF\FF\AA\AA\55\55") ;; Extreme values: 0x0000, 0xFFFF, 0xAAAA, 0x5555

  ;;
  ;; Test function: load 16-bit value into lane 0
  ;;
  (func (export "test_load_lane_0") (param $addr i32) (result v128)
    (local.get $addr)               ;; Address for load
    (i32.const 0) (i16x8.splat)    ;; Create zero vector
    (v128.load16_lane 0)            ;; Load into lane 0
  )

  ;;
  ;; Test function: load 16-bit value into lane 1
  ;;
  (func (export "test_load_lane_1") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 1)
  )

  ;;
  ;; Test function: load 16-bit value into lane 2
  ;;
  (func (export "test_load_lane_2") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 2)
  )

  ;;
  ;; Test function: load 16-bit value into lane 3
  ;;
  (func (export "test_load_lane_3") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 3)
  )

  ;;
  ;; Test function: load 16-bit value into lane 4
  ;;
  (func (export "test_load_lane_4") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 4)
  )

  ;;
  ;; Test function: load 16-bit value into lane 5
  ;;
  (func (export "test_load_lane_5") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 5)
  )

  ;;
  ;; Test function: load 16-bit value into lane 6
  ;;
  (func (export "test_load_lane_6") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 6)
  )

  ;;
  ;; Test function: load 16-bit value into lane 7
  ;;
  (func (export "test_load_lane_7") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 7)
  )

  ;;
  ;; Test function: load into lane with pre-initialized vector
  ;;
  (func (export "test_load_lane_preserve") (param $addr i32) (result v128)
    ;; Create pattern vector: 0xA000, 0xA001, 0xA002, ..., 0xA007
    (local.get $addr)
    (i32.const 0xa000) (i16x8.splat)
    (i32.const 0xa001) (i16x8.replace_lane 1)
    (i32.const 0xa002) (i16x8.replace_lane 2)
    (i32.const 0xa003) (i16x8.replace_lane 3)
    (i32.const 0xa004) (i16x8.replace_lane 4)
    (i32.const 0xa005) (i16x8.replace_lane 5)
    (i32.const 0xa006) (i16x8.replace_lane 6)
    (i32.const 0xa007) (i16x8.replace_lane 7)
    ;; Load into lane 3
    (v128.load16_lane 3)
  )

  ;;
  ;; Test function: all lanes coverage (loads into each lane 0-7)
  ;;
  (func (export "test_all_lanes") (param $base_addr i32) (result v128)
    (local $result v128)

    ;; Start with zero vector
    (i32.const 0) (i16x8.splat)
    (local.set $result)

    ;; Load sequential 16-bit values into each lane
    (local.get $base_addr)
    (local.get $result)
    (v128.load16_lane 0)
    (local.set $result)

    (local.get $base_addr) (i32.const 2) (i32.add)
    (local.get $result)
    (v128.load16_lane 1)
    (local.set $result)

    (local.get $base_addr) (i32.const 4) (i32.add)
    (local.get $result)
    (v128.load16_lane 2)
    (local.set $result)

    (local.get $base_addr) (i32.const 6) (i32.add)
    (local.get $result)
    (v128.load16_lane 3)
    (local.set $result)

    (local.get $base_addr) (i32.const 8) (i32.add)
    (local.get $result)
    (v128.load16_lane 4)
    (local.set $result)

    (local.get $base_addr) (i32.const 10) (i32.add)
    (local.get $result)
    (v128.load16_lane 5)
    (local.set $result)

    (local.get $base_addr) (i32.const 12) (i32.add)
    (local.get $result)
    (v128.load16_lane 6)
    (local.set $result)

    (local.get $base_addr) (i32.const 14) (i32.add)
    (local.get $result)
    (v128.load16_lane 7)
  )

  ;;
  ;; Test function: boundary access tests
  ;;
  (func (export "test_boundary") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 0)
  )

  ;;
  ;; Test function: unaligned access tests
  ;;
  (func (export "test_unaligned") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 0)
  )

  ;;
  ;; Test function: extreme values
  ;;
  (func (export "test_extreme_values") (param $offset i32) (result v128)
    (i32.const 0x200) (local.get $offset) (i32.add)  ;; Base address + offset
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 0)
  )

  ;;
  ;; Test function: out-of-bounds access (will trap)
  ;;
  (func (export "test_out_of_bounds") (param $addr i32) (result v128)
    (local.get $addr)  ;; This should be an out-of-bounds address
    (i32.const 0) (i16x8.splat)
    (v128.load16_lane 0)
  )

  ;;
  ;; Helper function: extract 16-bit value from v128 vector at specified lane
  ;;
  (func (export "extract_i16") (param $vec v128) (param $lane i32) (result i32)
    (local.get $lane)
    (i32.const 0) (i32.eq)
    (if (result i32) (then
      (local.get $vec) (i16x8.extract_lane_u 0)
    ) (else
      (local.get $lane)
      (i32.const 1) (i32.eq)
      (if (result i32) (then
        (local.get $vec) (i16x8.extract_lane_u 1)
      ) (else
        (local.get $lane)
        (i32.const 2) (i32.eq)
        (if (result i32) (then
          (local.get $vec) (i16x8.extract_lane_u 2)
        ) (else
          (local.get $lane)
          (i32.const 3) (i32.eq)
          (if (result i32) (then
            (local.get $vec) (i16x8.extract_lane_u 3)
          ) (else
            (local.get $lane)
            (i32.const 4) (i32.eq)
            (if (result i32) (then
              (local.get $vec) (i16x8.extract_lane_u 4)
            ) (else
              (local.get $lane)
              (i32.const 5) (i32.eq)
              (if (result i32) (then
                (local.get $vec) (i16x8.extract_lane_u 5)
              ) (else
                (local.get $lane)
                (i32.const 6) (i32.eq)
                (if (result i32) (then
                  (local.get $vec) (i16x8.extract_lane_u 6)
                ) (else
                  ;; Default to lane 7
                  (local.get $vec) (i16x8.extract_lane_u 7)
                ))
              ))
            ))
          ))
        ))
      ))
    ))
  )
)