# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig

  sig {params(a: T.untyped).returns(A)}
  def self.returns_an_a(a)
    a
  end
end

class NotA
  def is_a?(x)
    case x
    when x == A.class
      puts "called A"
      true
    else
      puts "calling super"
      super
    end
  end
end

begin
  A.returns_an_a(NotA.new)
rescue
  puts "got error, as expected"
else
  puts "what?!"
end
