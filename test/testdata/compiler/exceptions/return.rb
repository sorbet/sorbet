# frozen_string_literal: true
# typed: true
# compiled: true

def test(a, b, c, d)
  puts "test: #{a}, #{b}, #{c}, #{d}"
  begin
    puts "  body #{a}"
    return 'body' if a
    raise "foo" unless b
  rescue
    puts "  rescue #{c}"
    return 'rescue' if c
  else
    puts "  else #{c}"
    return 'else' if c
  ensure
    puts "  ensure #{d}"
    return 'ensure' if d
  end

  return "fall-through"
end

[true,false].each do |a|
  [true,false].each do |b|
    [true,false].each do |c|
      [true,false].each do |d|
        puts "result = #{test(a,b,c,d)}"
      end
    end
  end
end
