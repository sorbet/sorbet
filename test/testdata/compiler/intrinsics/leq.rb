# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

class SomethingElse
  def <=(arg0)
    "got arg: #{arg0}"
  end
end

sig {params(x: T.any(Integer, SomethingElse), y: T.untyped).returns(T.untyped)}
def do_leq(x, y)
  x <= y
end

# INITIAL-LABEL: {{^}}define{{.*}}@"func_Object#6do_leq"
# INITIAL-NOT: call i64 @sorbet_vm_leq
# INITIAL: call i64{{.*}}@sorbet_i_send
# INITIAL-NOT: call i64 @sorbet_vm_leq
# INITIAL: call i64 @sorbet_rb_int_le
# INITIAL-NOT: call i64 @sorbet_vm_leq
# INITIAL{LITERAL}: }

def do_leq_untyped(x, y)
  x <= y
end

# INITIAL-LABEL: {{^}}define{{.*}}@"func_Object#14do_leq_untyped"
# INITIAL-NOT: call i64 @sorbet_rb_int_le
# INITIAL: call i64 @sorbet_vm_leq
# INITIAL-NOT: call i64 @sorbet_rb_int_le
# INITIAL{LITERAL}: }

p do_leq(4, 5)
p do_leq(8, 9.0)
p do_leq(SomethingElse.new, :foo)

p do_leq_untyped(4, 5)
p do_leq_untyped(7, 3.0)
p do_leq_untyped(SomethingElse.new, [:foo])
