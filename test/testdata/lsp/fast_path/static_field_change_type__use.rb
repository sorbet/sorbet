# typed: strict
extend T::Sig

sig {returns(String)}
def foo
  STATIC_FIELD.fetch(:key, "default") + "suffix"
end
