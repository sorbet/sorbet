# typed: true
# enable-suggest-unsafe: true
extend T::Sig

sig {returns(String)}
def wrap_in_unsafe
  nil # error: Expected `String` but found `NilClass` for method result type
end
