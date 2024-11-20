# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(str: String, i: T.untyped).returns(T.nilable(String))}
def string_brackets(str, i)
  str[i]
end


sig {params(str: String, i: T.untyped).returns(T.nilable(String))}
def string_slice(str, i)
  str.slice(i)
end


p string_brackets("123", 1)
p string_slice("123", 1)
p string_brackets("123", 0...2)
p string_slice("123", 0...2)
