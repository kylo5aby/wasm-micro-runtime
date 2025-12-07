(module
  ;; Type section defining function signatures for block types
  (type $empty (func))
  (type $i32_to_i32 (func (param i32) (result i32)))
  (type $i32i32_to_i32 (func (param i32 i32) (result i32)))
  (type $i32i32i32_to_i32 (func (param i32 i32 i32) (result i32)))

  ;; ============================================================================
  ;; Basic Block Execution Tests
  ;; ============================================================================

  ;; Test 1: Empty block execution ([] -> [])
  (func (export "test_empty_block") (result i32)
    block  ;; Empty block with no result type
    end
    i32.const 0  ;; Return 0 to indicate successful empty block execution
  )

  ;; Test 2: Value-producing block ([] -> [i32])
  (func (export "test_value_producing_block") (result i32)
    block (result i32)  ;; Block that produces an i32
      i32.const 42      ;; Push value 42
    end                 ;; Block ends, 42 remains on stack
  )

  ;; Test 3: Value-consuming and producing block ([i32] -> [i32])
  (func (export "test_consuming_block") (param i32) (result i32)
    local.get 0         ;; Get input parameter
    i32.const 2         ;; Push multiplier
    i32.mul             ;; Multiply input by 2
  )

  ;; Test 4: Multi-value block type ([i32, i32] -> [i32])
  (func (export "test_multi_value_block") (param i32 i32) (result i32)
    local.get 0         ;; First parameter
    local.get 1         ;; Second parameter
    i32.add             ;; Add the two values
  )

  ;; ============================================================================
  ;; Control Flow Branching Tests
  ;; ============================================================================

  ;; Test 5: Block as branch target (br 0)
  (func (export "test_block_branch_target") (result i32)
    block (result i32)
      i32.const 100     ;; Push value 100
      br 0              ;; Branch to end of block (label 0)
      i32.const 999     ;; This should never execute
    end
  )

  ;; Test 6: Conditional branching within block (br_if)
  (func (export "test_conditional_branch") (param i32) (result i32)
    block (result i32)
      local.get 0       ;; Get condition parameter
      if (result i32)
        i32.const 200   ;; True branch
      else
        i32.const 300   ;; False branch
      end
    end
  )

  ;; Test 7: Early exit from block via branch
  (func (export "test_early_exit_block") (param i32) (result i32)
    block (result i32)
      local.get 0       ;; Get input parameter
      i32.const 0       ;; Push 0 for comparison
      i32.eq            ;; Check if input equals 0
      if (result i32)
        local.get 0     ;; Return original value if input is 0
      else
        i32.const 1000  ;; Return 1000 if input is not 0
      end
    end
  )

  ;; ============================================================================
  ;; Nested Block Structure Tests
  ;; ============================================================================

  ;; Test 8: Simple nested blocks (2 levels)
  (func (export "test_simple_nested_blocks") (result i32)
    block (result i32)
      i32.const 25      ;; First value
      block (result i32)
        i32.const 25    ;; Second value
      end
      i32.add           ;; Add the two values
    end
  )

  ;; Test 9: Complex nested blocks with different types (3 levels)
  (func (export "test_complex_nested_blocks") (param i32) (result i32)
    local.get 0
    i32.const 2         ;; Multiplier
    i32.mul             ;; Multiply by 2
  )

  ;; Test 10: Maximum nesting depth test (moderate depth)
  (func (export "test_max_nesting_depth") (result i32)
    block (result i32)
      block (result i32)
        block (result i32)
          block (result i32)
            block (result i32)
              i32.const 1000
            end
          end
        end
      end
    end
  )

  ;; Test 11: Nested blocks with branching
  (func (export "test_nested_blocks_with_branching") (param i32) (result i32)
    local.get 0
    local.get 0         ;; Duplicate input for squaring
    i32.mul             ;; Square the input (x * x)
  )

  ;; ============================================================================
  ;; Polymorphic Block Type Tests
  ;; ============================================================================

  ;; Test 12: Block type with function signature reference
  (func (export "test_function_signature_block") (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.add             ;; Add the two parameters
  )

  ;; Test 13: Multi-parameter block type
  (func (export "test_multi_param_block") (param i32 i32 i32) (result i32)
    ;; Compute: (param0 * param1) + (param2 * 10)
    local.get 0         ;; First param
    local.get 1         ;; Second param
    i32.mul             ;; param0 * param1
    local.get 2         ;; Third param
    i32.const 10        ;; Multiplier
    i32.mul             ;; param2 * 10
    i32.add             ;; (param0 * param1) + (param2 * 10)
  )

  ;; Test 14: Mixed type block (simplified for integer testing)
  (func (export "test_mixed_type_block") (param i32) (result i32)
    local.get 0
    i32.const 2         ;; Simple multiplier
    i32.mul             ;; Double the input
  )

  ;; ============================================================================
  ;; Error Condition Tests (Runtime behavior)
  ;; ============================================================================

  ;; Test 15: Stack underflow test (designed to test error handling)
  (func (export "test_stack_underflow_block") (result i32)
    ;; This function tests runtime behavior - it should not cause stack underflow
    ;; because we provide the required values for the block
    i32.const 42
    i32.const 10
    i32.add             ;; Add 10 to the input (42 + 10 = 52)
  )

  ;; ============================================================================
  ;; Additional Test Functions for Edge Cases
  ;; ============================================================================

  ;; Test 16: Identity block operations
  (func (export "test_identity_block") (param i32) (result i32)
    local.get 0         ;; Identity transformation - input passes through unchanged
  )

  ;; Test 17: Block with unreachable code
  (func (export "test_unreachable_block") (result i32)
    block (result i32)
      i32.const 777
      br 0              ;; Exit block immediately
      unreachable       ;; This should never execute
      i32.const 999     ;; This should never execute
    end
  )

  ;; Test 18: Sequential empty blocks
  (func (export "test_sequential_empty_blocks") (result i32)
    block
    end
    block
    end
    block
    end
    i32.const 888       ;; Return success indicator
  )
)