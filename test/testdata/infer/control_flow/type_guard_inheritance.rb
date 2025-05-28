# typed: true

class Animal
  extend T::Sig
  extend T::Helpers
  abstract!

  sig { abstract.returns(T::Boolean).narrows_to(Mammal) }
  def mammal?; end

  sig { abstract.returns(T::Boolean).narrows_to(Bird) }
  def bird?; end
end

class Mammal < Animal
  extend T::Helpers
  abstract!

  sig { override.returns(TrueClass).narrows_to(Mammal) }
  def mammal?; true; end

  sig { override.returns(FalseClass) }
  def bird?; false; end

  sig { abstract.returns(T::Boolean).narrows_to(Dog) }
  def dog?; end
end

class Bird < Animal
  sig { override.returns(FalseClass) }
  def mammal?; false; end

  sig { override.returns(TrueClass).narrows_to(Bird) }
  def bird?; true; end
end

class Dog < Mammal
  sig { override.returns(TrueClass).narrows_to(Dog) }
  def dog?; true; end
end

class Cat < Mammal
  sig { override.returns(FalseClass) }
  def dog?; false; end
end

def test_deep_inheritance
  animal = T.let(Dog.new, Animal)
  
  if animal.mammal?
    T.reveal_type(animal) # error: Revealed type: `Mammal`
    
    if animal.dog?
      T.reveal_type(animal) # error: Revealed type: `Dog`
    else
      T.reveal_type(animal) # error: Revealed type: `Mammal`
    end
  else
    T.reveal_type(animal) # error: Revealed type: `Animal`
  end
end

def test_multiple_paths
  animal = T.let(Bird.new, Animal)
  
  if animal.bird?
    T.reveal_type(animal) # error: Revealed type: `Bird`
  elsif animal.mammal?
    T.reveal_type(animal) # error: Revealed type: `Mammal`
  else
    T.reveal_type(animal) # error: Revealed type: `Animal`
  end
end