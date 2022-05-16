# typed: true

class Animal
  attr_reader :age

  def initialize
    @age = T.let(1, Integer)
#    ^ apply-rename: [A] newName: @years_of_age placeholderText: @age
  end
end
