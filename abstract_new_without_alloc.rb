module Helpers
  def self.extended(other)
    # puts("Helpers#included")
  end

  def abstract!
    class << self
      alias_method :__orig_new, :new
    end

    mod = self
    self.define_singleton_method(:new) do |*args, &blk|
      puts("⚠️ called slow wrapper #{mod}.new")

      result = super(*args, &blk)
      if result.instance_of?(mod)
        raise "💥 instantiated abstract class #{mod}"
      end
      result
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
