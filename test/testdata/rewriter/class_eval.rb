# typed: true

# Regression tests for https://github.com/sorbet/sorbet/issues/10452 and
# https://github.com/sorbet/sorbet/issues/10436
#
# For a top-level `Foo.class_eval do ... end` with a constant literal receiver, the
# method-defining statements in the block are hoisted into a synthesized reopening of
# `class Foo`, so they are owned by `Foo` with normal public visibility instead of being
# hoisted to `Object` and marked implicitly private. All other statements stay in the
# block, keeping their closure semantics.

class Module
  include T::Sig
end

class A; end

A.class_eval do
  def foo
    0
  end

  sig {returns(Integer)}
  def bar
    0
  end

  def self.baz; end

  private def hidden; end
end

A.new.foo
T.reveal_type(A.new.bar) # error: Revealed type: `Integer`
A.baz
A.new.hidden # error: Non-private call to private method `hidden` on `A`

class B; end

B.class_exec do
  def foo; end
end

B.new.foo

# A bare visibility modifier applies to the defs hoisted after it.
class BareVisibility; end

BareVisibility.class_eval do
  private
  def also_hidden; end
end

BareVisibility.new.also_hidden # error: Non-private call to private method `also_hidden` on `BareVisibility`

# Statements that are not method definitions stay in the block, so locals captured from
# the enclosing scope keep working alongside hoisted defs.
x = T.let(true, T::Boolean)

class Mixed; end

Mixed.class_eval do
  def hoisted; end

  if x
    puts "captured local still works"
  end
end

Mixed.new.hoisted

# Block parameters are fine: they stay on the block, and the defs are still hoisted.
class C; end

C.class_eval do |klass|
  puts klass
  def quux; end
end

C.new.quux

# Reopening a class from the stdlib must not clobber visibility of existing methods
# (https://github.com/sorbet/sorbet/issues/10452: `Object#==` used to become private).
Range.class_eval do
  def ==(other)
    super
  end
end

(1..5) == (1..5)
nil == (1..5)

# Reopening also works for constants nested inside other modules when written as a
# qualified literal at the top level.
module Outer
  class Inner; end
end

Outer::Inner.class_eval do
  def qux; end
end

Outer::Inner.new.qux

# A block that defines no methods is left completely untouched, so incidental uses on
# receivers that may be modules keep working.
module SomeModule; end
SomeModule.class_eval do |mod|
  puts mod
end

# A dynamic receiver is not rewritten: the def is hoisted to Object as before, and the
# method is not visible on the receiver's class.
class Dynamic; end
klass = Dynamic
klass.class_eval do
  def not_rewritten; end
end

# A class_eval that is not a top-level statement is not rewritten, because `Foo.class_eval`
# and `class Foo` would not necessarily resolve `Foo` to the same constant there.
class NotTopLevel; end
module Wrapper
  NotTopLevel.class_eval do
    def nested_not_rewritten; end
  end
end
NotTopLevel.new.nested_not_rewritten # error: Method `nested_not_rewritten` does not exist on `NotTopLevel`

# The string form of class_eval is not rewritten.
class StringForm; end
StringForm.class_eval("def from_string; end")
StringForm.new.from_string # error: Method `from_string` does not exist on `StringForm`

# class_exec arguments are forwarded to the block; the defs are still hoisted.
class ExecWithParams; end
ExecWithParams.class_exec(1) do |arg|
  puts arg
  def now_rewritten; end
end
ExecWithParams.new.now_rewritten
