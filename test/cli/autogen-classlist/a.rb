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
end

module A
  class Dup; end # Only appears once in output
end
