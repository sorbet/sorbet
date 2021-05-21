# typed: true

class Bar
  def self.print
    puts 'hello!'
  end
end

class Foo
  def self.bar
    Bar.print
  end
end
