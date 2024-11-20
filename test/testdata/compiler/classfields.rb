# frozen_string_literal: true
# typed: true
# compiled: true

class B

  def write(a)
    @@f = a
  end

  # class << self
  #   def ss
  #     @@f
  #   end
  # end
  def read
    @@f
  end
  def self.read
    @@f
  end
end

# TODO, we'll need more tests when we implement subclasses and fix self in static scope
B.new.write(1)

puts B.read
puts B.new.read
