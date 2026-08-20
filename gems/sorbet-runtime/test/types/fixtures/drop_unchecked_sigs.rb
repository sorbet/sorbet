# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# This fixture runs in a subprocess so that `T::Configuration.drop_unchecked_sigs!`
# and `T::Utils.run_all_sig_blocks` are isolated from the main test process.

def check(description, loc: caller_locations(1, 1).first)
  unless yield
    puts "FAIL: #{loc.path}:#{loc.lineno}: #{description}"
  end
end

unchecked = Class.new do
  extend T::Sig

  sig { params(x: Symbol).returns(Symbol).checked(:never) }
  def foo(x)
    x
  end
  alias_method :bar, :foo
end

checked = Class.new do
  extend T::Sig

  sig { params(x: Symbol).returns(Symbol) }
  def foo(x)
    x
  end
  alias_method :bar, :foo
end

abstract_klass = Class.new do
  extend T::Sig
  extend T::Helpers
  abstract!

  sig { abstract.params(x: Symbol).returns(Symbol).checked(:never) }
  def foo(x); end
end

T::Utils.run_all_sig_blocks

check("an unchecked sig is kept before the call") do
  !T::Utils.signature_for_method(unchecked.instance_method(:foo)).nil?
end

T::Configuration.drop_unchecked_sigs!

# The unchecked signature is freed, but the method and its alias still work.
check("the unchecked sig is freed") { T::Utils.signature_for_method(unchecked.instance_method(:foo)).nil? }
check("the unchecked method still runs") { unchecked.new.foo(:foo) == :foo }
check("the alias still runs") { unchecked.new.bar(:bar) == :bar }
check("the alias records no sig of its own") do
  T::Utils.signature_for_method(unchecked.instance_method(:bar)).nil?
end

# `Method#parameters` still reports the parameters, because the method is unwrapped.
check("the parameters survive") { unchecked.instance_method(:foo).parameters == [%i[req x]] }

# A checked signature is untouched: it stays introspectable and it still validates,
# through the original method and through the alias.
check("a checked sig is kept") { !T::Utils.signature_for_method(checked.instance_method(:foo)).nil? }
check("the checked method still runs") { checked.new.foo(:foo) == :foo }
check("the checked alias still runs") { checked.new.bar(:bar) == :bar }
check("the checked alias still validates") do
  checked.new.bar(1)
  false
rescue TypeError
  true
end

# An abstract signature is never dropped: `T::AbstractUtils` reads it to tell
# whether a method is abstract, long after load time.
check("an abstract sig is kept") { !T::Utils.signature_for_method(abstract_klass.instance_method(:foo)).nil? }
check("the method still reads as abstract") do
  T::AbstractUtils.abstract_method?(abstract_klass.instance_method(:foo))
end
check("an unimplemented abstract method still raises") do
  Class.new(abstract_klass).new.foo(:foo)
  false
rescue NotImplementedError
  true
end

puts "PASS"
