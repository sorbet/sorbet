# frozen_string_literal: true
# typed: true
# compiled: true
def my_name(name:, prefix: "Mr")
  prefix + " " + name
end

def f(a, b=3, name:, prefix: "Mr")
  p a, b
  prefix + " " + name
end

puts my_name(name: "Paul", prefix: "Master")
puts my_name(name: "Paul")

puts f(1, {x: 5}, name: "Nathan")
