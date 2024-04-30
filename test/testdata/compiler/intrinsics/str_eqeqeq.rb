# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(str: String, obj: T.untyped).returns(T::Boolean)}
def streq(str, obj)
  str === obj
end



p streq("str", 1)
p streq("str", "str")
p streq("str", nil)
