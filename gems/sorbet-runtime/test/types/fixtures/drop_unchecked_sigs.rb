# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# This fixture runs in a subprocess so that `T::Utils.drop_unchecked_sigs!`
# and `T::Utils.run_all_sig_blocks` are isolated from the main test process.

def check(description, loc: caller_locations(1, 1).first)
  unless yield
    puts "FAIL: #{loc.path}:#{loc.lineno}: #{description}"
  end
end

class Unchecked
  extend T::Sig

  sig { params(x: Symbol).returns(Symbol).checked(:never) }
  def foo(x)
    x
  end
  alias bar foo
end

class Checked
  extend T::Sig

  sig { params(x: Symbol).returns(Symbol) }
  def foo(x)
    x
  end
  alias bar foo
end

class AbstractParent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig { abstract.params(x: Symbol).returns(Symbol).checked(:never) }
  def foo(x); end
end

class AbstractChild < AbstractParent; end

T::Utils.run_all_sig_blocks

check("an unchecked sig is kept before the call") do
  !T::Utils.signature_for_method(Unchecked.instance_method(:foo)).nil?
end

T::Utils.drop_unchecked_sigs!

check("the unchecked sig is freed") { T::Utils.signature_for_method(Unchecked.instance_method(:foo)).nil? }
check("the unchecked method still runs") { Unchecked.new.foo(:foo) == :foo }
check("the alias still runs") { Unchecked.new.bar(:bar) == :bar }
check("the alias records a sig of its own") do
  !T::Utils.signature_for_method(Unchecked.instance_method(:bar)).nil?
end
T::Utils.drop_unchecked_sigs!
check("a later call frees the alias sig") do
  T::Utils.signature_for_method(Unchecked.instance_method(:bar)).nil?
end

check("the parameters survive") { Unchecked.instance_method(:foo).parameters == [%i[req x]] }

check("a checked sig is kept") { !T::Utils.signature_for_method(Checked.instance_method(:foo)).nil? }
check("the checked method still runs") { Checked.new.foo(:foo) == :foo }
check("the checked alias still runs") { Checked.new.bar(:bar) == :bar }
check("the checked alias still validates") do
  Checked.new.bar(1)
  false
rescue TypeError
  true
end

check("an abstract sig is kept") { !T::Utils.signature_for_method(AbstractParent.instance_method(:foo)).nil? }
check("the method still reads as abstract") do
  T::AbstractUtils.abstract_method?(AbstractParent.instance_method(:foo))
end
check("an unimplemented abstract method still raises") do
  AbstractChild.new.foo(:foo)
  false
rescue NotImplementedError
  true
end

puts "PASS"
