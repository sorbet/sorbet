# typed: true
# spacer for exclude-from-file-update
extend T::Sig

sig {params(x: A).void} # error: Constant `A` is not a class or type alias
def example(x)
  T.reveal_type(x) # error: `T.untyped`
end
