# typed: true

class A
  extend T::Sig

  sig { void }
  def self.foo; end
end

class B
  extend T::Sig

  sig { void }
  def self.foo; end
end

class C
  extend T::Sig

  sig { void }
  def self.foo; end
end

if A.foo
 # ^^^^^ error: Can't use void types in conditional
  puts "Dead code"
end

if B.foo
 # ^^^^^ error: Can't use void types in conditional
  puts "Dead code"
else
  puts "More dead code" # error: This code is unreachable
end

unless C.foo
     # ^^^^^ error: Can't use void types in conditional
  puts "Dead code" # error: This code is unreachable
end
