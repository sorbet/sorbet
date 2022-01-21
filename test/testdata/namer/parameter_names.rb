# typed: true
extend T::Sig

sig { params(arg: T.untyped).void}
def m1(arg); end

sig { params(argX: T.untyped).void}
def m1(argX); end # ok

sig { params(arg: T.untyped).void}
def m2(*arg); end

sig { params(argX: T.untyped).void}
def m2(*argX); end # ok

sig { params(block: T.untyped).void}
def m3(&block); end

sig { params(blockX: T.untyped).void}
def m3(&blockX); end # ok

sig { params(kwarg: T.untyped).void}
def m4(kwarg:); end

sig { params(kwarg1: T.untyped).void}
def m4(kwarg1:); end # error: Method `Object#m4` redefined with mismatched keyword argument name. Expected: `kwarg`, got: `kwarg1`

sig { params(kwargs: T.untyped).void}
def m5(**kwargs); end

sig { params(kwargs1: T.untyped).void}
def m5(**kwargs1); end # ok
