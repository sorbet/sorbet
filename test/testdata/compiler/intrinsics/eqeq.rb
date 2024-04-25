# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

class SomethingElse
  def ==(arg0)
    "got arg: #{arg0}"
  end
end

sig {params(x: T.any(Integer, SomethingElse), y: T.untyped).returns(T.untyped)}
def do_eqeq(x, y)
  x == y
end

# INITIAL-LABEL: @"func_Object#7do_eqeq"
# INITIAL-NOT: call i64 @sorbet_vm_eqeq
# INITIAL: call i64{{.*}}@sorbet_i_send
# INITIAL-NOT: call i64 @sorbet_vm_eqeq
# INITIAL: call i64 @sorbet_rb_int_equal
# INITIAL-NOT: call i64 @sorbet_vm_eqeq
# INITIAL{LITERAL}: }

def do_eqeq_untyped(x, y)
  x == y
end

# INITIAL-LABEL: @"func_Object#15do_eqeq_untyped"
# INITIAL-NOT: call i64 @sorbet_rb_int_equal
# INITIAL: call i64 @sorbet_vm_eqeq
# INITIAL-NOT: call i64 @sorbet_rb_int_equal
# INITIAL{LITERAL}: }

p do_eqeq(4, 5)
p do_eqeq(8, 9.0)
p do_eqeq(SomethingElse.new, :foo)

p do_eqeq_untyped(4, 5)
p do_eqeq_untyped(7, 3.0)
p do_eqeq_untyped(SomethingElse.new, [:foo])
