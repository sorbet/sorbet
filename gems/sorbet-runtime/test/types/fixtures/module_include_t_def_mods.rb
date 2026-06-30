# typed: true
require_relative '../../../lib/sorbet-runtime'

# Test that `Module.include T::DefMods` installs hooks globally, so that
# the method_added hooks fire on all classes/modules without needing T::Sig.
#
# We intentionally avoid `extend T::Sig` here. Instead we call the internal
# declare_sig API directly to prove that hooks come from T::DefMods alone.
#
# In a future change where something like `override def foo; end` has meaning
# even absent any preceding sig, we can restructure this test to not need this
# fake `sig` method definition, and just test whatever property we hope to
# happen in those cases.
class Module
  include T::DefMods

  private def sig(arg = nil, &blk)
    T::Private::Methods.declare_sig(self, caller_locations(1, 1)&.first, arg, &blk)
  end
end

module M
  extend T::Helpers
  interface!

  sig { returns(Integer) }
  abstract def baz; end
end

class A
  extend T::Helpers
  abstract!
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

class B < A
  sig { returns(Integer) }
  override def baz
    puts 'called B#baz'
    42
  end
end

b = B.new

begin
  b.foo
  puts "foo() did not raise a TypeError!"
rescue TypeError => e
  result = /Expected.*got type.*$/.match(e.message)&.to_s
  puts result
end

begin
  B.bar
  puts "bar() did not raise a TypeError!"
rescue TypeError => e
  result = /Expected.*got type.*$/.match(e.message)&.to_s
  puts result
end

puts b.baz
