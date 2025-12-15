# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: LOWERED

# NOTE: explicitly not testing the interaction with interpreted code here
# because we only emit direct calls if the call would be compiled -> compiled.

# NOTE: In LLVM 15, the optimizer is more conservative and doesn't remove
# the @sorbet_i_isa_Array type test even though it's only used in llvm.assume.
# The type test is still generated but used as a hint to the optimizer.

require_relative './final_method_assume_result_type__2'

# LOWERED-LABEL: @"func_Object#21test_singleton_method"
# LOWERED: @direct_func_Compiled.21test_singleton_method
# LOWERED: @sorbet_i_isa_Array
# LOWERED: @llvm.assume
# LOWERED{LITERAL}: }
def test_singleton_method
  Compiled.test_singleton_method.length
end

# LOWERED-LABEL: @"func_Object#20test_instance_method"
# LOWERED: @"direct_func_Compiled#20test_instance_method"
# LOWERED: @sorbet_i_isa_Array
# LOWERED: @llvm.assume
# LOWERED{LITERAL}: }
def test_instance_method
  compiled_inst = T.let(Compiled.new, Compiled)
  compiled_inst.test_instance_method.length
end

puts test_singleton_method
puts test_instance_method
