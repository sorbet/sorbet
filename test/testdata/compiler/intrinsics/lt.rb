# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

class SomethingElse
  def <(arg0)
    "got arg: #{arg0}"
  end
end

sig {params(x: T.any(Integer, SomethingElse), y: T.untyped).returns(T.untyped)}
def do_lt(x, y)
  x < y
end

# INITIAL-LABEL: {{^}}define{{.*}}@"func_Object#5do_lt"
# INITIAL-NOT: call i64 @sorbet_vm_lt
# INITIAL: call i64{{.*}}@sorbet_i_send
# INITIAL-NOT: call i64 @sorbet_vm_lt
# INITIAL: call i64 @sorbet_rb_int_lt
# INITIAL-NOT: call i64 @sorbet_vm_lt
# INITIAL{LITERAL}: }

def do_lt_untyped(x, y)
  x < y
end

# INITIAL-LABEL: {{^}}define{{.*}}@"func_Object#13do_lt_untyped"
# INITIAL-NOT: call i64 @sorbet_rb_int_lt
# INITIAL: call i64 @sorbet_vm_lt
# INITIAL-NOT: call i64 @sorbet_rb_int_lt
# INITIAL{LITERAL}: }

p do_lt(4, 5)
p do_lt(8, 9.0)
p do_lt(SomethingElse.new, :foo)

p do_lt_untyped(4, 5)
p do_lt_untyped(7, 3.0)
p do_lt_untyped(SomethingElse.new, [:foo])
