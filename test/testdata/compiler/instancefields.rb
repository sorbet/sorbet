# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

class A
  def write(v)
    @f = v
  end
  def read
    @f
  end
end

# INITIAL-LABEL: define internal i64 @"func_A#5write"
# INITIAL: call void @sorbet_instanceVariableSet
# INITIAL{LITERAL}: }

# INITIAL-LABEL: define internal i64 @"func_A#4read"
# INITIAL: call i64 @sorbet_instanceVariableGet
# INITIAL{LITERAL}: }

# OPT-LABEL: define internal i64 @"func_A#5write"
# OPT: call void @sorbet_vm_setivar
# OPT{LITERAL}: }

# OPT-LABEL: define internal i64 @"func_A#4read"
# OPT: call i64 @sorbet_vm_getivar
# OPT{LITERAL}: }

a = A.new
b = A.new
puts a.read
a.write("value")
puts a.read
puts b.read
