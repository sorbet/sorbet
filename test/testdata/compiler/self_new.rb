# typed: true
# compiled: true

class A
  def self.make
    self.new
  end

  def to_s
    "A: <empty>"
  end
end

class B
  def initialize(x)
    @x = x
  end

  def self.make(x)
    self.new(x)
  end

  def to_s
    "B: #{@x}"
  end
end

puts A.make
puts B.make(10)
