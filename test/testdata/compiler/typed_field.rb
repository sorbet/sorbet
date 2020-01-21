# typed: true
# compiled: true

class A
  def initialize
    @s = T.let(115, Integer)
  end

  def read
    @s
  end
end

puts A.new.read
