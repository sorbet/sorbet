# typed: true
# compiled: true

class A
  def self.const_missing(name)
    Module.new
  end
end

module A::B::C
end

puts A::B.class
