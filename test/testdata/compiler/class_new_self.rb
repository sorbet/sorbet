# frozen_string_literal: true
# typed: true
# compiled: true

Class.new do
  puts ("#{self}".gsub(/0x[0-9a-f]+/, ''))
end
