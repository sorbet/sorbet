# typed: false

class BasicSocket < IO
end

C1 = Class.new do
  define_method(:m_three) do; end
end

C2 = Class.new do
  define_singleton_method(:m_four) do; end
end

class C3; end
C3.const_set('X', 5)

Object.const_set(
  :Foo,
  Module.new do |m_foo|
    m_foo.const_set(
      :Bar,
      Module.new do |m_bar|
        m_bar.const_set(:Baz, Module.new)
        m_bar.private_constant(:Baz)
        m_bar.include(m_bar.const_get(:Baz))
      end
    )
  end
)
