# frozen_string_literal: true
# typed: true
# compiled: true

# NOTE explicitly not testing the interaction with interpreted code here
# because we only emit direct calls if the call would be compiled -> compiled.

require_relative './final_method_assume_result_type__2'

def test_singleton_method
  Compiled.test_singleton_method.length
end

def test_instance_method
  compiled_inst = T.let(Compiled.new, Compiled)
  compiled_inst.test_instance_method.length
end

puts test_singleton_method
puts test_instance_method
