(module
  ;; Linear memory with initial size of 1 page (64KB)
  (memory (export "memory") 1)

  ;; Test function: i64.load16_u - Load unsigned 16-bit integer and zero-extend to i64
  ;; @param i32 offset - Memory offset to load from
  ;; @return i64 - Zero-extended 64-bit unsigned integer
  (func (export "i64_load16_u") (param i32) (result i64)
    (i64.load16_u (local.get 0))
  )

  ;; Test function: i64.load16_u with immediate offset
  ;; @param i32 base_addr - Base memory address
  ;; @return i64 - Loaded value from base_addr + 4
  (func (export "i64_load16_u_offset") (param i32) (result i64)
    (i64.load16_u offset=4 (local.get 0))
  )

  ;; Test function: i64.load16_u with alignment specification
  ;; @param i32 offset - Memory offset to load from
  ;; @return i64 - Loaded value with explicit 2-byte alignment
  (func (export "i64_load16_u_aligned") (param i32) (result i64)
    (i64.load16_u align=2 (local.get 0))
  )

  ;; Helper function: Store 16-bit value to memory for testing
  ;; @param i32 offset - Memory offset to store at
  ;; @param i32 value - 16-bit value to store (using i32 param)
  (func (export "store_u16") (param i32) (param i32)
    (i32.store16 (local.get 0) (local.get 1))
  )

  ;; Helper function: Store multiple 16-bit values for batch testing
  ;; @param i32 base_offset - Starting memory offset
  ;; @param i32 val1 - First 16-bit value
  ;; @param i32 val2 - Second 16-bit value
  ;; @param i32 val3 - Third 16-bit value
  (func (export "store_multiple_u16") (param i32) (param i32) (param i32) (param i32)
    (i32.store16 (local.get 0) (local.get 1))
    (i32.store16 (i32.add (local.get 0) (i32.const 2)) (local.get 2))
    (i32.store16 (i32.add (local.get 0) (i32.const 4)) (local.get 3))
  )

  ;; Test function: Load and compare with signed variant
  ;; Loads same memory location with both i64.load16_u and i64.load16_s for comparison
  ;; @param i32 offset - Memory offset to load from
  ;; @return i64 - Returns difference between unsigned and signed results (for 0x8000+)
  (func (export "compare_signed_unsigned") (param i32) (result i64)
    (local $unsigned i64) (local $signed i64)
    ;; Load as unsigned
    (local.set $unsigned (i64.load16_u (local.get 0)))
    ;; Load as signed
    (local.set $signed (i64.load16_s (local.get 0)))
    ;; Return difference (will be non-zero for values >= 0x8000)
    (i64.sub (local.get $unsigned) (local.get $signed))
  )

  ;; Test function: Verify zero extension property
  ;; @param i32 offset - Memory offset to load from
  ;; @return i32 - Returns 1 if upper 48 bits are zero, 0 otherwise
  (func (export "verify_zero_extension") (param i32) (result i32)
    (local $loaded i64)
    (local.set $loaded (i64.load16_u (local.get 0)))
    ;; Check if upper 48 bits are zero by right-shifting 16 bits
    (i64.eqz (i64.shr_u (local.get $loaded) (i64.const 16)))
  )

  ;; Boundary test function: Load from memory boundaries
  ;; @return i64 - Loaded value from last valid 16-bit position (memory.size - 2)
  (func (export "load_from_end") (result i64)
    (i64.load16_u (i32.sub (memory.size) (i32.const 2)))
  )

  ;; Error test function: Attempt out-of-bounds load
  ;; @return i64 - Should trap when called due to memory bounds violation
  (func (export "load_out_of_bounds") (result i64)
    (i64.load16_u (memory.size))
  )

  ;; Initialize memory with test data
  (func (export "init_test_data")
    ;; Store test values at various offsets
    (i32.store16 (i32.const 0) (i32.const 0x1234))   ;; offset 0: 0x1234
    (i32.store16 (i32.const 2) (i32.const 0x5678))   ;; offset 2: 0x5678
    (i32.store16 (i32.const 4) (i32.const 0xABCD))   ;; offset 4: 0xABCD
    (i32.store16 (i32.const 6) (i32.const 0x0000))   ;; offset 6: 0x0000 (MIN)
    (i32.store16 (i32.const 8) (i32.const 0x7FFF))   ;; offset 8: 0x7FFF (max positive signed)
    (i32.store16 (i32.const 10) (i32.const 0x8000))  ;; offset 10: 0x8000 (signed boundary)
    (i32.store16 (i32.const 12) (i32.const 0xFFFF))  ;; offset 12: 0xFFFF (MAX)

    ;; Store test data at memory end for boundary testing
    (i32.store16 (i32.sub (memory.size) (i32.const 2)) (i32.const 0x9999))
  )
)