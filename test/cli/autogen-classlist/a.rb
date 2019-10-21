# typed: true

class Z; end

module A
  module B
    class C1
      module ModInClass; end # Should not be in output
    end;
  end
  class Dup; end;

  class Parent; end
  class Child < Parent; end

  sig {params(x: NonExistentClass).returns(Integer)}
  def foo(x); 1; end

  # None of the following should appear in the output:
  TypeAlias = T.type_alias {Integer}
  ClassAlias = Integer
  Anon = Class.new
end

module A
  class Dup; end # Only appears once in output
end

# Parent is not resolved but class is still included in output
class C::D < NonExistentParent; end
