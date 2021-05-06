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

C4 = Class.new do
  define_method(:m_five) do; end
  alias_method(:m_six, :m_five)
end

C5 = Class.new do
  define_singleton_method(:m_seven) do; end
  alias_method(:m_eight, :m_seven)
end
