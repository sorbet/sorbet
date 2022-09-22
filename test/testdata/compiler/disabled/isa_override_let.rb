# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig

  sig {params(a: T.untyped).returns(T.untyped)}
  def self.takes_something(a)
    x = T.let(a, A)
    x
  end
end

class NotA
  def is_a?(x)
    if x == A
      true
    else
      super
    end
  end
end

begin
  A.takes_something(NotA.new)
rescue
  puts "got error, but we should not have!"
else
  puts "this is expected"
end
