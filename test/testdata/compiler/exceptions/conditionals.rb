# frozen_string_literal: true
# typed: true
# compiled: true

# Tests that conditionals present in rescue/else/ensure don't cause problems

class A
  def self.test(a,b,c)
    begin
      raise "foo" if a
    rescue
      if b
        puts "rescue: true"
      else
        puts "rescue: false"
      end
    else
      if b
        puts "else: true"
      else
        puts "else: false"
      end
    ensure
      if c
        puts "ensure: true"
      else
        puts "ensure: false"
      end
    end
  end
end

[true,false].each do |a|
  [true,false].each do |b|
    [true,false].each do |c|
      puts "a: #{a}, b: #{b}, c: #{c}"
      A.test(a,b,c)
    end
  end
end
