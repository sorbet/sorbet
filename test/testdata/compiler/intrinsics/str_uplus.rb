# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL OPT

extend T::Sig

sig {returns(String)}
def str_uplus
  s = +''
  s
end

# INITIAL-LABEL: @"func_Object#9str_uplus"
# INITIAL: call i64 @sorbet_int_str_uplus
# This is sort of a hack to see if we eliminate the exit type test, which we should.
# INITIAL: typeTestSuccess:
# INITIAL{LITERAL}: }

# OPT-LABEL: @"func_Object#9str_uplus"
# OPT: call i64 @sorbet_vm_str_uplus
# OPT-NOT: typeTestSuccess:
# OPT{LITERAL}: }

p str_uplus()
