# typed: true
# selective-apply-code-action: quickfix

x = nil

1.times { x = 42 }
            # ^^ error: Changing the type of a variable in a loop is not permitted
            # ^^ apply-code-action: [A] Initialize as `T.nilable(Integer)`
