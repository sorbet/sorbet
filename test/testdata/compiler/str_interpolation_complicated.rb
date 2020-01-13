# typed: true
# compiled: true

def foo_1
  s = Class.new do
    T.unsafe(self).define_method(:to_s) do
      puts "foo_1 to_s"
      "1"
    end
  end
  s.new
end

def foo_2
  s = Class.new do
    T.unsafe(self).define_method(:to_s) do
      puts "foo_2 to_s"
      1
    end
  end
  s.new
end

puts "#{foo_1}"
puts "#{foo_2}".gsub(/0x[0-9a-f]+/, '')
