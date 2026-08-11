# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# This fixture runs in a subprocess so that `T::Configuration.drop_unchecked_sigs!`
# and `T::Configuration.enable_checking_for_sigs_marked_checked_tests` are isolated
# from the main test process. It must not call `T::Utils.run_all_sig_blocks`, which
# wraps the `.checked(:tests)` sigs of sorbet-runtime itself.

def check(description, loc: caller_locations(1, 1).first)
  unless yield
    puts "FAIL: #{loc.path}:#{loc.lineno}: #{description}"
  end
end

# Dropping must not read `RuntimeLevels.check_tests?` on its own, otherwise it
# would close the window for turning on `:tests`-level checking.
T::Configuration.drop_unchecked_sigs!

check("test-level checking can still be turned on") do
  T::Configuration.enable_checking_for_sigs_marked_checked_tests
  true
rescue StandardError => e
  puts "  #{e.message}"
  false
end

# In a test environment a `.checked(:tests)` sig is a checked sig, so it is kept
# and it still validates.
klass = Class.new do
  extend T::Sig

  sig { params(x: Symbol).returns(Symbol).checked(:tests) }
  def foo(x)
    x
  end
  alias_method :bar, :foo
end

check("the method runs") { klass.new.foo(:foo) == :foo }
check("the sig is kept") { !T::Utils.signature_for_method(klass.instance_method(:foo)).nil? }
check("the alias still validates") do
  klass.new.bar(1)
  false
rescue TypeError
  true
end

puts "PASS"
