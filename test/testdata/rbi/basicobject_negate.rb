# typed: true

if !nil # if true
  puts "true branch"
else
  puts "else branch" # error: This code is unreachable
end

a = T.let(nil, BasicObject)

if !a
  puts "true branch"
else
  puts "else branch"
end

if !false # if true
  puts "true branch"
else
  puts "else branch" # error: This code is unreachable
end

if !true # if false
  puts "true branch" # error: This code is unreachable
else
  puts "else branch"
end

if !0 # if false
  puts "true branch" # error: This code is unreachable
else
  puts "else branch"
end

if ![] # if false
  puts "true branch" # error: This code is unreachable
else
  puts "else branch"
end

if !"" # if false
  puts "true branch" # error: This code is unreachable
else
  puts "else branch"
end

class A; end

if !A.new # if false
  puts "true branch"
else
  puts "else branch"
end

class MyArray < Array
  extend T::Sig
  sig { returns(Integer) }
  def !()
# ^^^^^^^ error: Method `MyArray#!` overrides an overridable method `Array#!` but is not declared with `override.`
# ^^^^^^^ error: Return type `Integer` does not match return type of overridable method `Array#!`
    1
  end
end

class MyInteger < Integer
  extend T::Sig
  sig { override.returns(Integer) }
  def !()
# ^^^^^^^ error: Return type `Integer` does not match return type of overridable method `Integer#!`
    1
  end
end

class MyString < String
  extend T::Sig
  sig { override(allow_incompatible: true).returns(Integer) }
  def !()
    1
  end
end
