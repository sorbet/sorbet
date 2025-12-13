# frozen_string_literal: true
# typed: true
# compiled: true

def multiple
  1
end

# Make sure we actually do something with the first definition.
p multiple

def multiple
  2
end

p multiple
