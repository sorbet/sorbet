# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

# INITIAL-LABEL: define internal i64 @"func_Object#6func_a"
# INITIAL-NOT: call { i64, i8 } @sorbet_vm_return_from_block_wrapper
# INITIAL{LITERAL}: }
def func_a
  if T.unsafe(35 > 36)
    return 209
  end

  begin
    if T.unsafe(false)
      raise "oh no"
    end
    return 3
  rescue
    return 3.times { 444 }
  ensure
    return 822
  end
end

# INITIAL-LABEL: define internal i64 @"func_Object#6func_b"
# INITIAL: call { i64, i8 } @sorbet_vm_return_from_block_wrapper
# INITIAL{LITERAL}: }
def func_b
  1.times { return 33 }
end

def justyield
  yield
end

# INITIAL-LABEL: define internal i64 @"func_Object#6func_c"
# INITIAL: call { i64, i8 } @sorbet_vm_return_from_block_wrapper
# INITIAL{LITERAL}: }
def func_c
  if T.unsafe(3+3 > 5)
    puts (justyield { return 33 })
  end
end

# INITIAL-LABEL: define internal i64 @"func_Object#6func_d"
# INITIAL-NOT: call { i64, i8 } @sorbet_vm_return_from_block_wrapper
# INITIAL{LITERAL}: }
def func_d
  if T.unsafe(3+3 > 5)
    puts (justyield { 33 })
  end
end
