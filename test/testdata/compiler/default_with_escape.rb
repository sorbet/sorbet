# frozen_string_literal: true
# typed: true
# compiled: true

module Test
  def self.test(a, b = (3.times { puts "from default: #{a}" }; 10))
    puts a
    puts b
  end
end

Test.test('hello')
Test.test('hello', 'there')
