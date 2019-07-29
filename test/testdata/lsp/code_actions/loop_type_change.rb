# typed: true

x = nil

1.times { x = 42 }
            # ^^ error: Changing the type of a variable in a loop is not permitted
            # ^^ apply-code-action: A Changing the type of a variable in a loop is not permitted
