# typed: true
# selective-apply-code-action: quickfix

x = nil

1.times { x = 42 }
            # ^^ error: Changing the type of a variable is not permitted in loops and blocks
            # ^^ apply-code-action: [A] Initialize as `T.nilable(Integer)`
