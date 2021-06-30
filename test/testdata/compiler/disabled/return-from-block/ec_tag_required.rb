# frozen_string_literal: true
# typed: true
# compiled: true
# DISABLED FOR NOW: run_filecheck: INITIAL

# INITIAL-LABEL: define i64 @"func_Object#func_a"
# INITIAL-NOT: alloca %struct.rb_vm_tag
# INITIAL{LITERAL}: }
def func_a
  return

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

# INITIAL-LABEL: define i64 @"func_Object#func_b"
# INITIAL: alloca %struct.rb_vm_tag
# INITIAL{LITERAL}: }
def func_b
  1.times { return 33 }
end

def justyield
  yield
end

# INITIAL-LABEL: define i64 @"func_Object#func_c"
# INITIAL: alloca %struct.rb_vm_tag
# INITIAL{LITERAL}: }
def func_c
  if T.unsafe(3+3 > 5)
    puts (justyield { return 33 })
  end
end

# INITIAL-LABEL: define i64 @"func_Object#func_d"
# INITIAL-NOT: alloca %struct.rb_vm_tag
# INITIAL{LITERAL}: }
def func_d
  if T.unsafe(3+3 > 5)
    puts (justyield { 33 })
  end
end
