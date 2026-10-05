# typed: strict

module Consumer
  T.reveal_type(Regexp::FromExistingPrefix) # error: Revealed type: `T.class_of(Regexp::FromExistingPrefix)`
  T.reveal_type(MissingPrefix::FromSyntheticPrefix) # error: Revealed type: `T.class_of(MissingPrefix::FromSyntheticPrefix)`
end
