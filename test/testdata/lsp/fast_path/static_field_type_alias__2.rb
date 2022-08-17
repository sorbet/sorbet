# typed: true
# spacer for exclude-from-file-update

# This file only mentions the name of the type alias,
# not the name of the method that the type alias was used in

extend T::Sig

sig {params(x: AliasContainer::ContainedThing).void}
def another_example(x)
  T.reveal_type(x) # error: `T.any(Float, Symbol)`
end
