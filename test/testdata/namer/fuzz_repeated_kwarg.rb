# typed: false
# from fuzzer: https://github.com/sorbet/sorbet/issues/1133
def f1(x, x); end # error-with-dupes: Duplicated argument name `x`
def f2(x, x=0); end # error-with-dupes: Duplicated argument name `x`
def f3(x:, x:); end # error-with-dupes: Duplicated argument name `x`
def f4(x:, x: nil); end # error-with-dupes: Duplicated argument name `x`
def f5(x, x:); end # error-with-dupes: Duplicated argument name `x`
def f6(x, x: nil); end # error-with-dupes: Duplicated argument name `x`
def f7(x=0, x: nil); end # error-with-dupes: Duplicated argument name `x`

def f8(
      this,
      this: # error-with-dupes: Duplicated argument name `this`
    )
end

def f9(x, y, z, x); end # error-with-dupes: Duplicated argument name `x`

l1 = lambda { |y, y| } # error-with-dupes: Duplicated argument name `y`
