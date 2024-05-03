# frozen_string_literal: true
# typed: true
# compiled: true

module M

  def self.test_block()
    {a: 1, b: 2, c: 3}.transform_values do |a|
      p " #{a}"
      a + 1
    end
  end

  def self.test_enum()
    {a: 1, b: 2, c: 3}.transform_values
  end

end

p(M.test_block)
p(M.test_enum.each {|x| p "enum #{x}"})

