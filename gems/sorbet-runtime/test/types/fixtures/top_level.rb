# typed: true
require_relative '../../../lib/sorbet-runtime'

extend T::Sig # rubocop:disable Style/MixinUsage

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
rescue TypeError => e
  result = /Expected.*got type.*$/.match(e.message)&.to_s
  puts result
end

begin
  bar
  puts "bar() did not raise a TypeError!"
rescue TypeError => e
  result = /Expected.*got type.*$/.match(e.message)&.to_s
  puts result
end
