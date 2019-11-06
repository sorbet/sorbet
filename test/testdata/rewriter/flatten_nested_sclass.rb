class A
  def self.foo;
    class << self
      def bar; end
    end
  end
end

A.foo
A.bar
