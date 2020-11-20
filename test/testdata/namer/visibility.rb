# typed: true

class A
  def f1; end
  public def f2; end
  private def f3; end
  protected def f4; end
  private_class_method def self.f5; end
end

class B
  def f1; end
  def f2; end
  def f3; end
  def f4; end
  def self.f5; end
  public :f2
  private :f3
  protected :f4
  private_class_method :f5
end


class C
  private :foo
  def foo; end  # this does not end up being private
end

class Foo1
  def foo;  # this does not end up being private
  end
  private_class_method :foo # error: No method called `foo` exists to be made `private` in `T.class_of(Foo1)`
end

class Foo2
  def self.foo;  # this does not end up being private
  end
  private :foo # error: No method called `foo` exists to be made `private` in `Foo2`
end

class Foo3
  def self.foo;
  end
  class <<self
    private :foo
  end
end

class Foo4
  class <<self
    def foo;
    end
    private :foo
  end
end

C.new.foo # error: Non-private call to private method `foo`
