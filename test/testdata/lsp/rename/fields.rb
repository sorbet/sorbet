# typed: true

class Person
  attr_reader :age
  attr_reader :name

  def initialize
    @age = T.let(1, Integer)
#    ^ apply-rename: [A] newName: @years_of_age placeholderText: @age
    @name = T.let("John", String)
#    ^ apply-rename: [B] newName: first_name placeholderText: @name
  end

  def information
    "#{@name} is #{@age} years old."
  end
end

class Child < Person
  def information
    "Child #{@name} is #{@age} years old."
#             ^ apply-rename: [C] newName: first_name placeholderText: @name
  end
end
