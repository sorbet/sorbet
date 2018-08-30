# typed: true

class Parent
  def self.foo; end
end

Class.new
Class.new(Parent)

Class.new {|cls| cls.superclass}
Class.new(Parent) {|cls| cls.superclass}

# TODO This should ideally type-check, but doesn't
# Class.new(Parent).foo
# c = Class.new(Parent) {|cls| cls.foo}
# c.foo
