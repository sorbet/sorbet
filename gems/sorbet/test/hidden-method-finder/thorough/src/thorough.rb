# typed: false

C1 = Class.new do
  define_method(:m_three) do; end
end

C2 = Class.new do
  define_singleton_method(:m_four) do; end
end

class C3; end
C3.const_set('X', 5)
