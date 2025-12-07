(module
  ;; 5-page initial memory with strict limit of 10 pages maximum
  (memory (export "memory") 5 10)

  ;; Function to get current memory size
  (func (export "get_memory_size") (result i32)
    memory.size
  )

  ;; Function to grow memory by specified pages and return new size
  (func (export "grow_and_get_size") (param $pages i32) (result i32)
    (local $old_size i32)
    (local.set $old_size (memory.size))
    (drop (memory.grow (local.get $pages)))
    memory.size
  )

  ;; Function to attempt growth beyond limit and return size
  ;; This will fail when trying to grow beyond max limit
  (func (export "try_grow_beyond_limit") (param $pages i32) (result i32)
    (local $growth_result i32)
    (local $size_before i32)
    (local $size_after i32)

    ;; Store size before growth attempt
    (local.set $size_before (memory.size))

    ;; Attempt to grow memory (will fail if beyond limit)
    (local.set $growth_result (memory.grow (local.get $pages)))

    ;; Check if growth failed (-1) and return appropriate result
    (if (result i32) (i32.eq (local.get $growth_result) (i32.const -1))
      (then
        ;; Growth failed, return -1 to indicate failure
        (i32.const -1)
      )
      (else
        ;; Growth succeeded, return new memory size
        memory.size
      )
    )
  )
)