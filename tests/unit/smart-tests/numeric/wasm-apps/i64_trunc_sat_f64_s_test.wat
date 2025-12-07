(module
  ;; Test function for i64.trunc_sat_f64_s opcode
  ;; Takes f64 parameter and returns i64 result of saturating signed truncation
  (func (export "test_i64_trunc_sat_f64_s") (param f64) (result i64)
    ;; Load f64 parameter from stack
    local.get 0

    ;; Perform saturating truncation: f64 -> i64 (signed)
    ;; This uses the miscellaneous extension opcode 0xFC 0x00
    i64.trunc_sat_f64_s

    ;; Return i64 result
  )
)