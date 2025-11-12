# typed: true
# enable-experimental-requires-ancestor: true

module DoesNotSupportTAttachedClass1
  class A
  end

  module ClassMethods
    extend T::Helpers

    requires_ancestor { T.class_of(A) }

    def build
      new # error: Expression does not have a fully-defined type (Did you reference another class's type members?)
    end
  end
end

module DoesNotSupportTAttachedClass2
  class ::Module
    include T::Sig

    sig { params(other: T.anything).returns(T.nilable(T.attached_class)) }
    def downcast(other)
      case other
      when self then other
      else nil
      end
    end
  end

  module Foo
    extend T::Helpers

    requires_ancestor { Module }

    sig { void }
    def example1
      x = downcast(0) # error: Expression does not have a fully-defined type (Did you reference another class's type members?)
      T.reveal_type(x) # error: Revealed type: `T.untyped`
    end

    sig { void }
    def example2
      cond = self.===(0)
      if cond
        T.reveal_type(self) # error: Revealed type: `DoesNotSupportTAttachedClass2::Foo`
      else
        T.reveal_type(self) # error: Revealed type: `DoesNotSupportTAttachedClass2::Foo`
      end
    end
  end
end
