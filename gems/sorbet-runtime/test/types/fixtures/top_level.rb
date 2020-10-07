# typed: true
require_relative '../../../lib/sorbet-runtime'

extend T::Sig

sig {returns(Integer)}
def foo
  puts 'called main#foo'

  T.unsafe('nope')
end

sig {returns(Symbol)}
def self.bar
  puts 'called main.bar'

  T.unsafe(0.0)
end

begin
  foo
  puts "foo() did not raise a TypeError!"
rescue TypeError => exn
  puts (/Expected.*got type.*$/).match(exn.message)&.to_s
end

begin
  bar
  puts "bar() did not raise a TypeError!"
rescue TypeError => exn
  puts (/Expected.*got type.*$/).match(exn.message)&.to_s
end
