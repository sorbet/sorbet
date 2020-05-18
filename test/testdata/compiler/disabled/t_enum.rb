# frozen_string_literal: true
# typed: strict
# compiled: true

class MyEnum < T::Enum
  enums do
    X = new
  end
end

puts 'required successfully'
