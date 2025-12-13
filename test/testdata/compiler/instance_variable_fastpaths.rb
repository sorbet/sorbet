# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

class A
  def initialize(x)
    @x = x
  end

  def fast_get
    instance_variable_get(:@x)
  end

# INITIAL-LABEL: define internal i64 @"func_A#8fast_get"
# INITIAL: call i64 @sorbet_vm_instance_variable_get
# INITIAL{LITERAL}: }

# Can't really test too many or too few args because those won't get past Sorbet.

  def non_constant_arg_get(sym)
    instance_variable_get(sym)
  end

# INITIAL-LABEL: define internal i64 @"func_A#20non_constant_arg_get"
# INITIAL-NOT: call i64 @sorbet_vm_instance_variable_get
# INITIAL{LITERAL}: }

  def fast_set(x)
    instance_variable_set(:@x, x)
  end

# INITIAL-LABEL: define internal i64 @"func_A#8fast_set"
# INITIAL: call i64 @sorbet_vm_instance_variable_set
# INITIAL{LITERAL}: }

  def non_constant_arg_set(sym, value)
    instance_variable_set(sym, value)
  end

# INITIAL-LABEL: define internal i64 @"func_A#20non_constant_arg_set"
# INITIAL-NOT: call i64 @sorbet_vm_instance_variable_set
# INITIAL{LITERAL}: }

end

a = A.new(897)
p a.fast_get
p a.non_constant_arg_get(:@x)
p a.non_constant_arg_get(:@y)

a.fast_set(76)
p a.fast_get
p a.non_constant_arg_get(:@x)
p a.non_constant_arg_get(:@y)

a.non_constant_arg_set(:@x, 492)
a.non_constant_arg_set(:@y, 371)
p a.fast_get
p a.non_constant_arg_get(:@x)
p a.non_constant_arg_get(:@y)

class FewArgs
  def instance_variable_get
    "nothing"
  end

# INITIAL-LABEL: define internal i64 @"func_FewArgs#21instance_variable_get"
# INITIAL-NOT: call i64 @sorbet_vm_instance_variable_get
# INITIAL{LITERAL}: }

  def instance_variable_set
    "nada"
  end

# INITIAL-LABEL: define internal i64 @"func_FewArgs#21instance_variable_set"
# INITIAL-NOT: call i64 @sorbet_vm_instance_variable_set
# INITIAL{LITERAL}: }

end

f = FewArgs.new
p f.instance_variable_get
p f.instance_variable_set

class ManyArgs
  def instance_variable_get(x, y)
    x + y
  end

# INITIAL-LABEL: define internal i64 @"func_ManyArgs#21instance_variable_get"
# INITIAL-NOT: call i64 @sorbet_vm_instance_variable_get
# INITIAL{LITERAL}: }

  def instance_variable_set(x, y, z)
    x + y + z
  end

# INITIAL-LABEL: define internal i64 @"func_ManyArgs#21instance_variable_set"
# INITIAL-NOT: call i64 @sorbet_vm_instance_variable_set
# INITIAL{LITERAL}: }

end

m = ManyArgs.new
p m.instance_variable_get(3, 9)
p m.instance_variable_set(1, 8, 5)
