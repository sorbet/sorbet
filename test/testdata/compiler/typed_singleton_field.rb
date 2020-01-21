# typed: true
# compiled: true

class A
  @s = T.let(413, Integer)

  def self.read
    @s
  end
end

puts A.read
