# frozen_string_literal: true
# typed: true
# compiled: true
def my_name(name:, prefix: "Mr")
  prefix + " " + name
end

puts my_name(name: "Paul", prefix: "Master")
puts my_name(name: "Paul")
