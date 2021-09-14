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

# INITIAL-LABEL: define i64 @"func_B#6caller"
# INITIAL: call i64 @sorbet_callFuncDirect(%struct.FunctionInlineCache* @{{.*@func_A.3foo}}
# INITIAL{LITERAL}: }

# OPT-LABEL: define i64 @"func_B#6caller"
# OPT-NOT: sorbet_callFuncWithCache
# OPT{LITERAL}: }
