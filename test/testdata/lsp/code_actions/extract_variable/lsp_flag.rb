# typed: true
# selective-apply-code-action: refactor.extract
# assert-no-code-action: refactor.extract
# Tests that the code action is not shown when the flag is not enabled.

def example(x)
  xs = []
#      ^^ apply-code-action: [A] Extract Variable
end
