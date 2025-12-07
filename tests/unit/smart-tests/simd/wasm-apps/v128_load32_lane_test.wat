(module
  ;; Memory section with 1 page (64KB) for test data
  (memory (export "memory") 1 1)

  ;; Initialize memory with test data patterns for v128.load32_lane validation
  ;; Basic test data at addresses 0x100-0x10F (4 uint32 values)
  (data (i32.const 0x100) "\DD\CC\BB\AA\11\00\FF\EE\78\56\34\12\21\43\65\87")
  ;; 0x100: 0xAABBCCDD, 0x104: 0xEEFF0011, 0x108: 0x12345678, 0x10C: 0x87654321

  ;; Test data for memory offset tests at 0x400+
  (data (i32.const 0x400) "\00\00\00\10\00\00\00\20\00\00\00\30\00\00\00\40")
  (data (i32.const 0x410) "\00\00\00\50\00\00\00\60\00\00\00\70\00\00\00\80")
  ;; 0x400: 0x10000000, 0x404: 0x20000000, 0x408: 0x30000000, 0x40C: 0x40000000
  ;; 0x410: 0x50000000, 0x414: 0x60000000, 0x418: 0x70000000, 0x41C: 0x80000000

  ;; Boundary values test data at 0x500+
  (data (i32.const 0x500) "\00\00\00\00\FF\FF\FF\FF\FF\FF\FF\7F\00\00\00\80")
  ;; 0x500: 0x00000000, 0x504: 0xFFFFFFFF, 0x508: 0x7FFFFFFF, 0x50C: 0x80000000

  ;; Special float patterns at 0x600+
  (data (i32.const 0x600) "\00\00\C0\7F\00\00\80\7F\00\00\80\FF\00\00\00\00")
  (data (i32.const 0x610) "\00\00\00\80\01\00\80\7F")
  ;; 0x600: 0x7FC00000 (NaN), 0x604: 0x7F800000 (+Inf), 0x608: 0xFF800000 (-Inf), 0x60C: 0x00000000 (+0.0)
  ;; 0x610: 0x80000000 (-0.0), 0x614: 0x7F800001 (Signaling NaN)

  ;; Test data for zero address access at 0x000+
  (data (i32.const 0x000) "\11\11\11\11\22\22\22\22\33\33\33\33\44\44\44\44")
  ;; 0x000: 0x11111111, 0x004: 0x22222222, 0x008: 0x33333333, 0x00C: 0x44444444

  ;; Test data for memory alignment scenarios at 0x200+
  (data (i32.const 0x200) "\EF\BE\AD\DE")
  ;; 0x200: 0xDEADBEEF (aligned access test)

  ;; Test data for unaligned access at 0x300+
  (data (i32.const 0x300) "\00\EF\BE\AD\DE\00\EF\BE\AD\DE\00\00\00\00\00\00")
  ;; 0x300: 0x00, 0x301: 0xDEADBEEF (unaligned +1), 0x302: 0xDEADBEEF (unaligned +2)

  ;; Boundary test data near memory limit (65532 = 65536 - 4)
  (data (i32.const 65532) "\EF\BE\AD\DE")
  ;; 65532: 0xDEADBEEF

  ;; Test function for v128.load32_lane with lane 0
  (func (export "test_load32_lane0")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32) (param $addr i32)
        (result i32 i32 i32 i32)
    (local $vec v128)

    ;; Build the input vector from parameters
    (local.set $vec
      (i32x4.replace_lane 3
        (i32x4.replace_lane 2
          (i32x4.replace_lane 1
            (i32x4.splat (local.get $lane0))
            (local.get $lane1))
          (local.get $lane2))
        (local.get $lane3)))

    ;; Load 32-bit value from memory into lane 0
    (local.set $vec
      (local.get $addr)
      (local.get $vec)
      (v128.load32_lane 0))

    ;; Extract all lanes and return them
    (i32x4.extract_lane 0 (local.get $vec))
    (i32x4.extract_lane 1 (local.get $vec))
    (i32x4.extract_lane 2 (local.get $vec))
    (i32x4.extract_lane 3 (local.get $vec))
  )

  ;; Test function for v128.load32_lane with lane 1
  (func (export "test_load32_lane1")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32) (param $addr i32)
        (result i32 i32 i32 i32)
    (local $vec v128)

    ;; Build the input vector from parameters
    (local.set $vec
      (i32x4.replace_lane 3
        (i32x4.replace_lane 2
          (i32x4.replace_lane 1
            (i32x4.splat (local.get $lane0))
            (local.get $lane1))
          (local.get $lane2))
        (local.get $lane3)))

    ;; Load 32-bit value from memory into lane 1
    (local.set $vec
      (local.get $addr)
      (local.get $vec)
      (v128.load32_lane 1))

    ;; Extract all lanes and return them
    (i32x4.extract_lane 0 (local.get $vec))
    (i32x4.extract_lane 1 (local.get $vec))
    (i32x4.extract_lane 2 (local.get $vec))
    (i32x4.extract_lane 3 (local.get $vec))
  )

  ;; Test function for v128.load32_lane with lane 2
  (func (export "test_load32_lane2")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32) (param $addr i32)
        (result i32 i32 i32 i32)
    (local $vec v128)

    ;; Build the input vector from parameters
    (local.set $vec
      (i32x4.replace_lane 3
        (i32x4.replace_lane 2
          (i32x4.replace_lane 1
            (i32x4.splat (local.get $lane0))
            (local.get $lane1))
          (local.get $lane2))
        (local.get $lane3)))

    ;; Load 32-bit value from memory into lane 2
    (local.set $vec
      (local.get $addr)
      (local.get $vec)
      (v128.load32_lane 2))

    ;; Extract all lanes and return them
    (i32x4.extract_lane 0 (local.get $vec))
    (i32x4.extract_lane 1 (local.get $vec))
    (i32x4.extract_lane 2 (local.get $vec))
    (i32x4.extract_lane 3 (local.get $vec))
  )

  ;; Test function for v128.load32_lane with lane 3
  (func (export "test_load32_lane3")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32) (param $addr i32)
        (result i32 i32 i32 i32)
    (local $vec v128)

    ;; Build the input vector from parameters
    (local.set $vec
      (i32x4.replace_lane 3
        (i32x4.replace_lane 2
          (i32x4.replace_lane 1
            (i32x4.splat (local.get $lane0))
            (local.get $lane1))
          (local.get $lane2))
        (local.get $lane3)))

    ;; Load 32-bit value from memory into lane 3
    (local.set $vec
      (local.get $addr)
      (local.get $vec)
      (v128.load32_lane 3))

    ;; Extract all lanes and return them
    (i32x4.extract_lane 0 (local.get $vec))
    (i32x4.extract_lane 1 (local.get $vec))
    (i32x4.extract_lane 2 (local.get $vec))
    (i32x4.extract_lane 3 (local.get $vec))
  )

  ;; Load with memarg offset 0 (lane 0)
  (func (export "test_load32_lane0_offset0")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32) (param $addr i32)
        (result i32 i32 i32 i32)
    (local $vec v128)

    ;; Build the input vector from parameters
    (local.set $vec
      (i32x4.replace_lane 3
        (i32x4.replace_lane 2
          (i32x4.replace_lane 1
            (i32x4.splat (local.get $lane0))
            (local.get $lane1))
          (local.get $lane2))
        (local.get $lane3)))

    ;; Load with offset 0
    (local.set $vec
      (local.get $addr)
      (local.get $vec)
      (v128.load32_lane offset=0 0))

    ;; Extract all lanes and return them
    (i32x4.extract_lane 0 (local.get $vec))
    (i32x4.extract_lane 1 (local.get $vec))
    (i32x4.extract_lane 2 (local.get $vec))
    (i32x4.extract_lane 3 (local.get $vec))
  )

  ;; Load with memarg offset 4 (lane 0)
  (func (export "test_load32_lane0_offset4")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32) (param $addr i32)
        (result i32 i32 i32 i32)
    (local $vec v128)

    ;; Build the input vector from parameters
    (local.set $vec
      (i32x4.replace_lane 3
        (i32x4.replace_lane 2
          (i32x4.replace_lane 1
            (i32x4.splat (local.get $lane0))
            (local.get $lane1))
          (local.get $lane2))
        (local.get $lane3)))

    ;; Load with offset 4
    (local.set $vec
      (local.get $addr)
      (local.get $vec)
      (v128.load32_lane offset=4 0))

    ;; Extract all lanes and return them
    (i32x4.extract_lane 0 (local.get $vec))
    (i32x4.extract_lane 1 (local.get $vec))
    (i32x4.extract_lane 2 (local.get $vec))
    (i32x4.extract_lane 3 (local.get $vec))
  )

  ;; Load with memarg offset 8 (lane 0)
  (func (export "test_load32_lane0_offset8")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32) (param $addr i32)
        (result i32 i32 i32 i32)
    (local $vec v128)

    ;; Build the input vector from parameters
    (local.set $vec
      (i32x4.replace_lane 3
        (i32x4.replace_lane 2
          (i32x4.replace_lane 1
            (i32x4.splat (local.get $lane0))
            (local.get $lane1))
          (local.get $lane2))
        (local.get $lane3)))

    ;; Load with offset 8
    (local.set $vec
      (local.get $addr)
      (local.get $vec)
      (v128.load32_lane offset=8 0))

    ;; Extract all lanes and return them
    (i32x4.extract_lane 0 (local.get $vec))
    (i32x4.extract_lane 1 (local.get $vec))
    (i32x4.extract_lane 2 (local.get $vec))
    (i32x4.extract_lane 3 (local.get $vec))
  )

  ;; Load with memarg offset 16 (lane 0)
  (func (export "test_load32_lane0_offset16")
        (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32) (param $addr i32)
        (result i32 i32 i32 i32)
    (local $vec v128)

    ;; Build the input vector from parameters
    (local.set $vec
      (i32x4.replace_lane 3
        (i32x4.replace_lane 2
          (i32x4.replace_lane 1
            (i32x4.splat (local.get $lane0))
            (local.get $lane1))
          (local.get $lane2))
        (local.get $lane3)))

    ;; Load with offset 16
    (local.set $vec
      (local.get $addr)
      (local.get $vec)
      (v128.load32_lane offset=16 0))

    ;; Extract all lanes and return them
    (i32x4.extract_lane 0 (local.get $vec))
    (i32x4.extract_lane 1 (local.get $vec))
    (i32x4.extract_lane 2 (local.get $vec))
    (i32x4.extract_lane 3 (local.get $vec))
  )
)