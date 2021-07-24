# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def self.test(x)
    begin
      puts "begin"
      if x
        puts x
        raise "foo"
      end
    rescue => e
      puts e
    else
      puts "else"
    ensure
      puts "ensure"
    end
  end
end

puts "=== no-raise ==="
A.test false

puts "=== raise ==="
A.test true
