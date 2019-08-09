# typed: true

class A
  extend T::Sig
  sig {returns(Integer)}
  def foo
    1 + 3
  end

  sig {returns(Integer)}
  def self.bar
    1 + 2
  end
end

class Integer
  class Foo
  end
end

class A
  extend T::Sig
  sig {void}
  def foo
  end
end

# these could all be returns(TrueClass)
module M
  extend T::Sig
  module M
    extend T::Sig
    module M
      extend T::Sig
      sig {returns(T::Boolean)}
      def self.a
        M == self
      end
      sig {returns(T::Boolean)}
      def self.b
        ::M::M::M == self
      end
    end
    sig {returns(T::Boolean)}
    def self.a
      M != self
    end
    sig {returns(T::Boolean)}
    def self.b
      M == ::M::M::M
    end
    sig {returns(T::Boolean)}
    def self.c
      ::M::M == self
    end
  end
  sig {returns(T::Boolean)}
  def self.a
    M != self
  end
  sig {returns(T::Boolean)}
  def self.b
    M == ::M::M
  end
  sig {returns(T::Boolean)}
  def self.c
    ::M == self
  end
end

class C
  extend T::Sig
  @a = 1
  @@b = 2
  sig {returns(Integer)}
  def self.f
    @a + @@b
  end
  class C
    extend T::Sig
    sig {returns(Integer)}
    def self.f
      @a + @@b
    end
    sig {returns(T.class_of(C))}
    def c
      C
    end
  end
end

class C
  extend T::Sig
  sig {void}
  def C
    T.reveal_type(C) # error: Revealed type: `T.class_of(C::C)`
    T.reveal_type(C.new.c) # error: Revealed type: `T.class_of(C::C)`
  end
  sig {returns(Integer::Foo)}
  def self.C
    T.reveal_type(C) # error: Revealed type: `T.class_of(C::C)`
    T.reveal_type(self.C) # error: Revealed type: `Integer::Foo`
    Integer::Foo.new
  end
end
