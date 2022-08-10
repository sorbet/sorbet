# typed: strict
# spacer for exclude-from-file-update
extend T::Sig

sig {returns(String)}
def foo
  T.reveal_type(STATIC_FIELD) # error: Revealed type: `T::Hash[Symbol, String]`
  fetched = STATIC_FIELD.fetch(:key, "default")
  result = fetched + "suffix"
  result
end
