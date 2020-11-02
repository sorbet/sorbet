# typed: true

extend T::Sig

sig {params(x: Integer, y:Integer).void}
def takes_kwargs(x, y:)
end

arghash = {y: 42}

# NOTE: This currently fails with "Passing a hash where the specific keys are
# unknown to a method taking keyword arguments". This is a bug in Sorbet. Once
# that bug is fixed, goodsplat.rb should exhibit no errors, so the expected
# output of this test will need to be updated and this comment removed.
takes_kwargs(99, **arghash)
