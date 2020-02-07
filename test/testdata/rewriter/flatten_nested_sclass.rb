# typed: true

class A
  def self.foo;
    class << self
      def bar; end
    end
  end
end

A.foo
A.bar

class B
  def foo;
    class << self
      def bar; end
    end
  end
end

B.new.foo
B.new.bar # error: Method `bar` does not exist on `B`


class C
  def self.foo;
    class << self
      class << self
        def bar; end
      end
    end
  end
end
