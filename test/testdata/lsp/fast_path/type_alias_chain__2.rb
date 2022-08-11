# typed: true
# spacer for exclude-from-file-update
extend T::Sig

Alias2 = T.type_alias {Alias1}

sig {params(x: Alias2).void}
def example2(x)
  T.reveal_type(x) # error: `Integer`
end
