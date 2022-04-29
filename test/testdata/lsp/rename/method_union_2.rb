# typed: true
# frozen_string_literal: true

class Dog
  def sound; "Bark"; end
end
class Cat
  def sound; "Meow"; end
end

animal = T.let(Cat.new, T.any(Dog, Cat))
animal.sound
#      ^ apply-rename: [A] newName: sound_new placeholderText: sound

