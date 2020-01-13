# typed: true
# compiled: true

class A
  def to_s
    puts "A.to_s"
    "1"
  end
  def inspect
    "2"
  end
end

class B
  def to_s
    puts "B.to_s"
    1
  end
  def inspect
    2
  end
end

puts "#{A.new} x"
puts "#{B.new} x".gsub(/0x[a-f0-9]+/, '')
