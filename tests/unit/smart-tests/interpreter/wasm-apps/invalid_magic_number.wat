;; This WAT file will be manually corrupted after compilation
;; to test invalid magic number handling
(module
  ;; Minimal module that will be corrupted
  (memory 1)
  (func $dummy
    nop
  )
  (export "dummy" (func $dummy))
)