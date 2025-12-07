;; Simple invalid index test
(module
  (func (export "test_boundary_access") (result i32)
    i32.const 0
  )
)
