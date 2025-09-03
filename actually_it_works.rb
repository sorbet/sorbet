module Helpers
  def abstract!
    @__is_abstract = true
    class << self
      alias_method :__orig_new, :new
    end

    extend(Hooks)
  end
end

module Hooks
  def inherited(other)
    puts ("Hooks#inherited")
    class << other
      alias_method :new, :__orig_new
    end
  end

  def new(...)
    puts("⚠️ called slow wrapper")

    result = super
    if @__is_abstract
      raise "💥 instantiated abstract class #{self}"
    end
    result
  end

  def abstract!
    super
    self.define_singleton_method(:new, Hooks.instance_method(:new))
  end
end

puts ("--- defining Parent ---")
class Parent
  extend Helpers
  abstract!
end

puts ("--- defining Child ---")
class Child < Parent
end

puts ("--- defining AbstractGrandchild ---")
class AbstractGrandchild < Child
  abstract!
end

puts ("--- defining GreatGrandchild ---")
class GreatGrandchild < AbstractGrandchild
end

puts ("--- defining AbstractChild ---")
class AbstractChild < Parent
  abstract!
end

puts ("--- ---")

def try_it
  yield
rescue => e
  puts(e.message)
else
  puts("✅ instantiated successfully")
ensure
  puts("===")
end

try_it do
  Parent.new
end
try_it do
  Child.new
end
try_it do
  AbstractGrandchild.new
end
try_it do
  GreatGrandchild.new
end
try_it do
  AbstractChild.new
end
