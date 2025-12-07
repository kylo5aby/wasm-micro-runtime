;; WebAssembly Text Format (.wat) for memory.init Opcode Testing
;; Comprehensive test module for memory initialization operation from data segments
;;
;; This module provides multiple test functions to validate memory.init behavior
;; across various scenarios including boundary conditions, error cases, and data segment management.

(module
  ;; Define linear memory with 1 page (64KB) for testing
  (memory (export "memory") 1)

  ;; Define passive data segments for memory.init testing (no memory offset - passive)
  ;; Data segment 0: Basic test string
  (data $seg0 "Hello, World! This is test data for memory.init validation.")

  ;; Data segment 1: Large data block for boundary testing (1024 bytes)
  (data $seg1
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF")

  ;; Data segment 2: Binary data for edge case testing
  (data $seg2
    "\00\01\02\03\04\05\06\07\08\09\0A\0B\0C\0D\0E\0F"
    "\10\11\12\13\14\15\16\17\18\19\1A\1B\1C\1D\1E\1F"
    "\20\21\22\23\24\25\26\27\28\29\2A\2B\2C\2D\2E\2F"
    "\30\31\32\33\34\35\36\37\38\39\3A\3B\3C\3D\3E\3F")

  ;; Primary test function for memory.init opcode
  ;; Parameters: dest_addr, src_offset, length, segment_index
  (func (export "test_memory_init")
    (param $dest i32) (param $src_offset i32) (param $length i32) (param $segment i32)

    ;; Conditional execution based on segment index
    (if (i32.eq (local.get $segment) (i32.const 0))
      (then
        ;; Initialize from data segment 0
        local.get $dest         ;; Destination address in memory
        local.get $src_offset   ;; Source offset within data segment
        local.get $length       ;; Number of bytes to copy
        memory.init 0           ;; Initialize from data segment 0
      )
      (else
        (if (i32.eq (local.get $segment) (i32.const 1))
          (then
            ;; Initialize from data segment 1
            local.get $dest
            local.get $src_offset
            local.get $length
            memory.init 1       ;; Initialize from data segment 1
          )
          (else
            ;; Initialize from data segment 2
            local.get $dest
            local.get $src_offset
            local.get $length
            memory.init 2       ;; Initialize from data segment 2
          )
        )
      )
    )
  )

  ;; Helper function to read a single byte from memory
  ;; Used for validation in tests
  (func (export "get_memory_byte") (param $address i32) (result i32)
    local.get $address
    i32.load8_u  ;; Load unsigned 8-bit value (byte) from memory
  )

  ;; Helper function to read multiple bytes from memory
  ;; Returns i32 value constructed from up to 4 bytes
  (func (export "get_memory_i32") (param $address i32) (result i32)
    local.get $address
    i32.load  ;; Load 32-bit value from memory (little-endian)
  )

  ;; Function to test data.drop functionality
  ;; Used to create dropped segment scenarios for error testing
  (func (export "test_data_drop") (param $segment i32)
    (if (i32.eq (local.get $segment) (i32.const 0))
      (then (data.drop 0))   ;; Drop data segment 0
      (else
        (if (i32.eq (local.get $segment) (i32.const 1))
          (then (data.drop 1)) ;; Drop data segment 1
          (else (data.drop 2)) ;; Drop data segment 2
        )
      )
    )
  )

  ;; Test function for basic memory initialization (segment 0)
  (func (export "test_basic_init") (result i32)
    ;; Initialize 16 bytes from segment 0 to memory address 100
    i32.const 100    ;; destination address
    i32.const 0      ;; source offset in segment 0
    i32.const 16     ;; number of bytes to copy
    memory.init 0    ;; initialize from data segment 0

    ;; Return first byte at destination for verification
    i32.const 100
    i32.load8_u
  )

  ;; Test function for zero-byte copy (should succeed without effect)
  (func (export "test_zero_copy") (result i32)
    ;; Copy 0 bytes (no-op)
    i32.const 200    ;; destination address
    i32.const 0      ;; source offset
    i32.const 0      ;; zero bytes to copy
    memory.init 0    ;; should succeed without copying anything

    ;; Return success indicator
    i32.const 1
  )

  ;; Test function for large transfer (512 bytes from segment 1)
  (func (export "test_large_transfer") (result i32)
    ;; Copy 512 bytes from large data segment
    i32.const 4096   ;; destination address (page 1)
    i32.const 0      ;; source offset
    i32.const 512    ;; large transfer size
    memory.init 1    ;; use large data segment

    ;; Return first byte for verification
    i32.const 4096
    i32.load8_u
  )

  ;; Test function for boundary operation (copy to end of memory)
  (func (export "test_boundary_copy") (result i32)
    ;; Copy 8 bytes to near end of memory (64KB - 16 bytes)
    i32.const 65520  ;; near end of 64KB memory
    i32.const 0      ;; source offset
    i32.const 8      ;; small copy size
    memory.init 0    ;; copy from segment 0

    ;; Return byte at boundary address
    i32.const 65520
    i32.load8_u
  )

  ;; Test function for partial segment copy with offset
  (func (export "test_offset_copy") (result i32)
    ;; Copy 12 bytes starting from offset 7 in segment 0 ("World! This ")
    i32.const 300    ;; destination address
    i32.const 7      ;; source offset (skip "Hello, ")
    i32.const 12     ;; copy 12 bytes
    memory.init 0    ;; copy from segment 0

    ;; Return first byte of copied data ('W' = 0x57)
    i32.const 300
    i32.load8_u
  )

  ;; Test function designed to trigger memory out-of-bounds trap
  (func (export "test_memory_overflow")
    ;; Attempt to copy beyond memory boundary (should trap)
    i32.const 65530  ;; destination near end of memory
    i32.const 0      ;; source offset
    i32.const 20     ;; too many bytes (65530 + 20 > 65536)
    memory.init 0    ;; should cause out-of-bounds trap
  )

  ;; Test function designed to trigger data segment out-of-bounds trap
  (func (export "test_segment_overflow")
    ;; Attempt to copy beyond segment boundary (should trap)
    i32.const 1000   ;; destination address
    i32.const 50     ;; source offset near end of segment 0 (~60 bytes)
    i32.const 20     ;; too many bytes for segment 0
    memory.init 0    ;; should cause segment out-of-bounds trap
  )

  ;; Test memory access after data.drop (should trap)
  (func (export "test_dropped_segment_access")
    ;; Drop segment 0 first
    data.drop 0

    ;; Attempt to use dropped segment (should trap)
    i32.const 500    ;; destination address
    i32.const 0      ;; source offset
    i32.const 8      ;; byte count
    memory.init 0    ;; should trap - segment is dropped
  )

  ;; Helper function to set memory pattern for validation
  (func (export "set_memory_pattern") (param $address i32) (param $value i32)
    local.get $address
    local.get $value
    i32.store8  ;; Store single byte at address
  )

  ;; Helper function to clear memory region
  (func (export "clear_memory") (param $address i32) (param $length i32)
    local.get $address
    i32.const 0      ;; fill value (zero)
    local.get $length
    memory.fill      ;; Clear memory region
  )
)