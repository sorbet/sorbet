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
  private_class_method :foo
end

class Foo2
  def self.foo;  # this does not end up being private
  end
  private :foo
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

class Foo5
  T.unsafe(nil).private

  def foo; end  # this does not end up being private
end

class Foo6
  self.private

  def foo; end
end

class Foo7
  bar = nil
  bar.private
  #   ^^^^^^^ error: Method `private` does not exist on `NilClass`

  def foo; end
end
