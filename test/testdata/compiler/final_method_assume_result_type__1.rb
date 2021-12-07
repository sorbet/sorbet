# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: LOWERED

# NOTE: explicitly not testing the interaction with interpreted code here
# because we only emit direct calls if the call would be compiled -> compiled.

#

require_relative './final_method_assume_result_type__2'

# LOWERED-LABEL: @"func_Object#21test_singleton_method"
# LOWERED: @direct_func_Compiled.21test_singleton_method
# LOWERED-NOT: @sorbet_i_isa_Array
# LOWERED{LITERAL}: }
def test_singleton_method
  Compiled.test_singleton_method.length
end

# LOWERED-LABEL: @"func_Object#20test_instance_method"
# LOWERED: @"direct_func_Compiled#20test_instance_method"
# LOWERED-NOT: @sorbet_i_isa_Array
# LOWERED{LITERAL}: }
def test_instance_method
  compiled_inst = T.let(Compiled.new, Compiled)
  compiled_inst.test_instance_method.length
end

puts test_singleton_method
puts test_instance_method
