# typed: true
require_relative '../../../lib/sorbet-runtime'

class Module
  include T::Sig
end

module M
  sig { returns(Integer) }
  def baz
    puts 'called M#baz'

    T.unsafe('bad')
  end
end

class A
  include M

  sig { returns(Integer) }
  def foo
    puts 'called A#foo'

    T.unsafe('nope')
  end

  sig { returns(Symbol) }
  def self.bar
    puts 'called A.bar'

    T.unsafe(0.0)
  end
end

a = A.new

begin
  a.foo
  puts "foo() did not raise a TypeError!"
rescue TypeError => e
  result = /Expected.*got type.*$/.match(e.message)&.to_s
  puts result
end

begin
  A.bar
  puts "bar() did not raise a TypeError!"
rescue TypeError => e
  result = /Expected.*got type.*$/.match(e.message)&.to_s
  puts result
end

begin
  a.baz
  puts "baz() did not raise a TypeError!"
rescue TypeError => e
  result = /Expected.*got type.*$/.match(e.message)&.to_s
  puts result
end
