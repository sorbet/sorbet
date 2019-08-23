# typed: false
class LetErrorsFalse
  def self.not_initialize_self
    @a = T.let(0, Integer) # error: Singleton instance variables must be declared inside the class body
  end

  def not_initialize
    @a = T.let(0, Integer) # error: Instance variables must be declared inside `initialize`
  end
end
