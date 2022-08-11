# typed: true
extend T::Sig

Alias1 = T.type_alias {Integer}

sig {params(x: Alias1).void}
def example1(x)
  T.reveal_type(x) # error: `Integer`
end
