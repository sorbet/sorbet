# frozen_string_literal: true
# typed: true
# compiled: true

x = 10

# Force "x" to escape. We know that we won't match MRI behavior if it doesn't.
let_x_escape = lambda {puts x}

class A
  y = 20

  # Force "y" to escape. We know that we won't match MRI behavior if it doesn't.
  let_y_escape = lambda {puts y}

  puts local_variables.include?(:x)
  puts local_variables.include?(:y)
  puts local_variables.include?(:z)
end

module B
  z = 30

  # Force "z" to escape. We know that we won't match MRI behavior if it doesn't.
  let_z_escape = lambda {puts z}

  puts local_variables.include?(:x)
  puts local_variables.include?(:y)
  puts local_variables.include?(:z)
end

puts local_variables.include?(:x)
puts local_variables.include?(:y)
puts local_variables.include?(:z)
