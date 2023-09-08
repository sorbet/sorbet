# typed: strict
class Module; include T::Sig; end

class Parent
  sig {void}
  def frob
    puts 'Parent#frob'
  end
end

class Child < Parent
end

Child.class_exec do
  sig {void}
  def frob
    super
  end
end

Child.new.frob
