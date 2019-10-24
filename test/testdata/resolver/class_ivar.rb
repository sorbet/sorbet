# typed: true
module A
  def self.something
    @x = T.let(1, Integer) # error: The singleton instance variable `@x` must be declared inside the class body
  end
end

class B
  def foo
    def self.bar
      @x = T.let(0, Integer) # error: The instance variable `@x` must be declared inside `initialize` or declared nilable
    end
  end
end

module C
  def self.foo
    @@x = T.let(1, Integer) # error: The class variable `@@x` must be declared at class scope
  end
end
