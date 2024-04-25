# frozen_string_literal: true
# compiled: true
# typed: true

# Ensure that init_change always returns the same object regardless of whether
# things have been initialized or not.  The intent of this test is that it
# ensures entirely deleting <Magic>.<must-be-truthy> does not change semantics.
class A
  def self.init_change
    @@changes ||= {}
  end

  def self.set_change(change)
    @@changes[change] = :done
  end

  def self.get_change(change)
    @@changes[change]
  end
end

initial = A.init_change
p initial
A.set_change('constant')
A.set_change('other')
p A.get_change('constant')
p A.get_change('other')
other = A.init_change
p other
p initial.object_id == other.object_id
