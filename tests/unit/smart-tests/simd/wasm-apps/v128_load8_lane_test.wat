(module
  ;; Linear memory with 1 page (64KB) for testing
  (memory (export "memory") 1 1)

  ;; Initialize memory with test patterns for v128.load8_lane validation
  (data (i32.const 0x00) "\00\01\02\03\04\05\06\07\08\09\0a\0b\0c\0d\0e\0f")
  (data (i32.const 0x10) "\42")                     ;; Test byte 0x42 at address 0x10
  (data (i32.const 0x18) "\85")                     ;; Test byte 0x85 at address 0x18
  (data (i32.const 0x1F) "\ff")                     ;; Test byte 0xFF at address 0x1F
  (data (i32.const 0x25) "\99")                     ;; Test byte 0x99 at address 0x25
  (data (i32.const 0x100) "\42")                    ;; Page boundary test at 0x100
  (data (i32.const 0xFF) "\ff")                     ;; Near page boundary

  ;; Test pattern for extreme values
  (data (i32.const 0x200) "\00\ff\aa\55")          ;; Extreme byte values: 0x00, 0xFF, 0xAA, 0x55

  ;;
  ;; Test function: load byte into lane 0
  ;;
  (func (export "test_load_lane_0") (param $addr i32) (result v128)
    (local.get $addr)               ;; Address for load
    (i32.const 0) (i8x16.splat)    ;; Create zero vector
    (v128.load8_lane 0)             ;; Load into lane 0
  )

  ;;
  ;; Test function: load byte into lane 1
  ;;
  (func (export "test_load_lane_1") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i8x16.splat)
    (v128.load8_lane 1)
  )

  ;;
  ;; Test function: load byte into lane 8
  ;;
  (func (export "test_load_lane_8") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i8x16.splat)
    (v128.load8_lane 8)
  )

  ;;
  ;; Test function: load byte into lane 15
  ;;
  (func (export "test_load_lane_15") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i8x16.splat)
    (v128.load8_lane 15)
  )

  ;;
  ;; Test function: load into lane with pre-initialized vector
  ;;
  (func (export "test_load_lane_preserve") (param $addr i32) (result v128)
    ;; Create pattern vector: 0xA0, 0xA1, 0xA2, ..., 0xAF
    (local.get $addr)
    (i32.const 0xa0) (i8x16.splat)
    (i32.const 0xa1) (i8x16.replace_lane 1)
    (i32.const 0xa2) (i8x16.replace_lane 2)
    (i32.const 0xa3) (i8x16.replace_lane 3)
    (i32.const 0xa4) (i8x16.replace_lane 4)
    (i32.const 0xa5) (i8x16.replace_lane 5)
    (i32.const 0xa6) (i8x16.replace_lane 6)
    (i32.const 0xa7) (i8x16.replace_lane 7)
    (i32.const 0xa8) (i8x16.replace_lane 8)
    (i32.const 0xa9) (i8x16.replace_lane 9)
    (i32.const 0xaa) (i8x16.replace_lane 10)
    (i32.const 0xab) (i8x16.replace_lane 11)
    (i32.const 0xac) (i8x16.replace_lane 12)
    (i32.const 0xad) (i8x16.replace_lane 13)
    (i32.const 0xae) (i8x16.replace_lane 14)
    (i32.const 0xaf) (i8x16.replace_lane 15)
    ;; Load into lane 5
    (v128.load8_lane 5)
  )

  ;;
  ;; Test function: all lanes coverage (loads into each lane 0-15)
  ;;
  (func (export "test_all_lanes") (param $base_addr i32) (result v128)
    (local $result v128)

    ;; Start with zero vector
    (i32.const 0) (i8x16.splat)
    (local.set $result)

    ;; Load sequential bytes into each lane
    (local.get $base_addr)
    (local.get $result)
    (v128.load8_lane 0)
    (local.set $result)

    (local.get $base_addr) (i32.const 1) (i32.add)
    (local.get $result)
    (v128.load8_lane 1)
    (local.set $result)

    (local.get $base_addr) (i32.const 2) (i32.add)
    (local.get $result)
    (v128.load8_lane 2)
    (local.set $result)

    (local.get $base_addr) (i32.const 3) (i32.add)
    (local.get $result)
    (v128.load8_lane 3)
    (local.set $result)

    (local.get $base_addr) (i32.const 4) (i32.add)
    (local.get $result)
    (v128.load8_lane 4)
    (local.set $result)

    (local.get $base_addr) (i32.const 5) (i32.add)
    (local.get $result)
    (v128.load8_lane 5)
    (local.set $result)

    (local.get $base_addr) (i32.const 6) (i32.add)
    (local.get $result)
    (v128.load8_lane 6)
    (local.set $result)

    (local.get $base_addr) (i32.const 7) (i32.add)
    (local.get $result)
    (v128.load8_lane 7)
    (local.set $result)

    (local.get $base_addr) (i32.const 8) (i32.add)
    (local.get $result)
    (v128.load8_lane 8)
    (local.set $result)

    (local.get $base_addr) (i32.const 9) (i32.add)
    (local.get $result)
    (v128.load8_lane 9)
    (local.set $result)

    (local.get $base_addr) (i32.const 10) (i32.add)
    (local.get $result)
    (v128.load8_lane 10)
    (local.set $result)

    (local.get $base_addr) (i32.const 11) (i32.add)
    (local.get $result)
    (v128.load8_lane 11)
    (local.set $result)

    (local.get $base_addr) (i32.const 12) (i32.add)
    (local.get $result)
    (v128.load8_lane 12)
    (local.set $result)

    (local.get $base_addr) (i32.const 13) (i32.add)
    (local.get $result)
    (v128.load8_lane 13)
    (local.set $result)

    (local.get $base_addr) (i32.const 14) (i32.add)
    (local.get $result)
    (v128.load8_lane 14)
    (local.set $result)

    (local.get $base_addr) (i32.const 15) (i32.add)
    (local.get $result)
    (v128.load8_lane 15)
  )

  ;;
  ;; Test function: boundary access tests
  ;;
  (func (export "test_boundary") (param $addr i32) (result v128)
    (local.get $addr)
    (i32.const 0) (i8x16.splat)
    (v128.load8_lane 0)
  )

  ;;
  ;; Test function: extreme values
  ;;
  (func (export "test_extreme_values") (param $offset i32) (result v128)
    (i32.const 0x200) (local.get $offset) (i32.add)  ;; Base address + offset
    (i32.const 0) (i8x16.splat)
    (v128.load8_lane 0)
  )

  ;;
  ;; Test function: out-of-bounds access (will trap)
  ;;
  (func (export "test_out_of_bounds") (param $addr i32) (result v128)
    (local.get $addr)  ;; This should be an out-of-bounds address
    (i32.const 0) (i8x16.splat)
    (v128.load8_lane 0)
  )

  ;;
  ;; Helper function: extract byte from v128 vector at specified lane
  ;;
  (func (export "extract_byte") (param $vec v128) (param $lane i32) (result i32)
    (local.get $lane)
    (i32.const 0) (i32.eq)
    (if (result i32) (then
      (local.get $vec) (i8x16.extract_lane_u 0)
    ) (else
      (local.get $lane)
      (i32.const 1) (i32.eq)
      (if (result i32) (then
        (local.get $vec) (i8x16.extract_lane_u 1)
      ) (else
        (local.get $lane)
        (i32.const 2) (i32.eq)
        (if (result i32) (then
          (local.get $vec) (i8x16.extract_lane_u 2)
        ) (else
          (local.get $lane)
          (i32.const 3) (i32.eq)
          (if (result i32) (then
            (local.get $vec) (i8x16.extract_lane_u 3)
          ) (else
            (local.get $lane)
            (i32.const 4) (i32.eq)
            (if (result i32) (then
              (local.get $vec) (i8x16.extract_lane_u 4)
            ) (else
              (local.get $lane)
              (i32.const 5) (i32.eq)
              (if (result i32) (then
                (local.get $vec) (i8x16.extract_lane_u 5)
              ) (else
                (local.get $lane)
                (i32.const 6) (i32.eq)
                (if (result i32) (then
                  (local.get $vec) (i8x16.extract_lane_u 6)
                ) (else
                  (local.get $lane)
                  (i32.const 7) (i32.eq)
                  (if (result i32) (then
                    (local.get $vec) (i8x16.extract_lane_u 7)
                  ) (else
                    (local.get $lane)
                    (i32.const 8) (i32.eq)
                    (if (result i32) (then
                      (local.get $vec) (i8x16.extract_lane_u 8)
                    ) (else
                      (local.get $lane)
                      (i32.const 9) (i32.eq)
                      (if (result i32) (then
                        (local.get $vec) (i8x16.extract_lane_u 9)
                      ) (else
                        (local.get $lane)
                        (i32.const 10) (i32.eq)
                        (if (result i32) (then
                          (local.get $vec) (i8x16.extract_lane_u 10)
                        ) (else
                          (local.get $lane)
                          (i32.const 11) (i32.eq)
                          (if (result i32) (then
                            (local.get $vec) (i8x16.extract_lane_u 11)
                          ) (else
                            (local.get $lane)
                            (i32.const 12) (i32.eq)
                            (if (result i32) (then
                              (local.get $vec) (i8x16.extract_lane_u 12)
                            ) (else
                              (local.get $lane)
                              (i32.const 13) (i32.eq)
                              (if (result i32) (then
                                (local.get $vec) (i8x16.extract_lane_u 13)
                              ) (else
                                (local.get $lane)
                                (i32.const 14) (i32.eq)
                                (if (result i32) (then
                                  (local.get $vec) (i8x16.extract_lane_u 14)
                                ) (else
                                  ;; Default to lane 15
                                  (local.get $vec) (i8x16.extract_lane_u 15)
                                ))
                              ))
                            ))
                          ))
                        ))
                      ))
                    ))
                  ))
                ))
              ))
            ))
          ))
        ))
      ))
    ))
  )
)