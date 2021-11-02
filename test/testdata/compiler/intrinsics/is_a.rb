# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

class Normal
  def f(x, y)
    x.is_a?(y)
  end

  def g(x, y)
    x.kind_of?(y)
  end
end

# INITIAL-LABEL: define internal i64 @"func_Normal#1f"
# INITIAL: call i64 @sorbet_vm_isa_p
# INITIAL{LITERAL}: }

# INITIAL-LABEL: define internal i64 @"func_Normal#1g"
# INITIAL: call i64 @sorbet_vm_isa_p
# INITIAL{LITERAL}: }

class Override
  def is_a?(x)
    "always"
  end

  def kind_of?(x)
    "definitely"
  end
end

n = Normal.new

p n.f("a", String)
p n.f(1, Integer)
p n.f([], Hash)

p n.g("a", String)
p n.g(1, Integer)
p n.g([], Hash)

o = Override.new

p n.f(o, String)
p n.f(o, Integer)
p n.g(o, Array)
p n.g(o, Hash)
