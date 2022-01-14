# typed: strict
# Found by fuzzer: https://github.com/sorbet/sorbet/issues/1258
def c(x)b(x)end # error: This function does not have a `sig`
      # ^ error: Method `b` does not exist on `Object`

