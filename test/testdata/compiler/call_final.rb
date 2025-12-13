# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

class A; 
  T::Sig::WithoutRuntime.sig(:final) {params(n: Integer).returns(Integer)}
  def self.foo(n)
    n
  end
end;
class B
def caller(a)
  A.foo(a)
end
end

# These groups of assertions check that the call to the direct wrapper is
# present in the un-optimized ir, and that it gets inlined in the optimized
# version.

# INITIAL-LABEL: define internal i64 @"func_B#6caller"
# INITIAL: call i64 @direct_func_A.3foo(%struct.FunctionInlineCache*
# INITIAL{LITERAL}: }

# OPT-LABEL: define internal i64 @"func_B#6caller"
# OPT-NOT: call i64 @direct_func_A.3foo(%struct.FunctionInlineCache*
# OPT{LITERAL}: }
