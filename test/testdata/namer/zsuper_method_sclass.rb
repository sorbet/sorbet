# typed: true

class Parent
  def self.foo; end
end

class Child < Parent
  class << self
    private :new

    private :foo
  end
end

Child.new # error: Non-private call to private method `new`
Child.foo # error: Non-private call to private method `foo`
