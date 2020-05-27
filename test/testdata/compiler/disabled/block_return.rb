# frozen_string_literal: true
# typed: true
# compiled: true

def test1
  1.times do
    return 10
  end
end

puts test1

def test2
  1.times do
    return 10
  ensure
    return 20
  end
end
