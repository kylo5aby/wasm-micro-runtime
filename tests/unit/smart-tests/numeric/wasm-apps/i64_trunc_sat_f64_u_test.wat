(module
  ;; Test function for i64.trunc_sat_f64_u opcode
  ;; Takes f64 parameter and returns i64 result of saturating unsigned truncation
  (func (export "test_i64_trunc_sat_f64_u") (param f64) (result i64)
    ;; Load f64 parameter from stack
    local.get 0

    ;; Perform saturating truncation: f64 -> i64 (unsigned)
    ;; This uses the miscellaneous extension opcode 0xFC 0x01
    i64.trunc_sat_f64_u

    ;; Return i64 result
  )
)