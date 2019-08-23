# typed: false
class LetErrorsFalse
  def self.not_initialize_self
    @a = T.let(0, Integer) # error: The singleton instance variable `@a` must be declared inside the class body
  end

  def not_initialize
    @a = T.let(0, Integer) # error: The instance variable `@a` must be declared inside `initialize`
  end
end
