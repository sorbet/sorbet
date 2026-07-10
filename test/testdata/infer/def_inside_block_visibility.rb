# typed: true

# Regression tests for https://github.com/sorbet/sorbet/issues/10452 and
# https://github.com/sorbet/sorbet/issues/10436
#
# Method defs lexically nested inside blocks used to be treated like ordinary
# top-level defs and marked implicitly private, which clobbered the visibility
# of methods like `Object#==` and `Module#name` for the entire program.
# In Ruby, the default visibility inside `class_eval`/`Class.new` blocks is
# public, so these defs must not be marked private.

Range.class_eval do
  def ==(other)
    super(other)
  end
end

(1..5) == (1..5)
nil == (1..5)

runtime_klass = Class.new do
  def self.name
    "RuntimeKlass"
  end
end

class MyAnotherClass
  def something
    self.class.name
  end
end

puts runtime_klass.name
puts MyAnotherClass.name
