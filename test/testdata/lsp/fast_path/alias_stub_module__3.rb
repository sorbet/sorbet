# typed: true
# spacer for exclude-from-file-update
extend T::Sig

sig {params(x: A).void}
def example(x)
  T.reveal_type(x) # error: `T.untyped`
end
