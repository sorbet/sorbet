# typed: true
# enable-suggest-unsafe: true
extend T::Sig

sig {returns(String)}
def wrap_in_unsafe
  nil # error: Expected `String` but found `NilClass` for method result type
end

class DSLMethod
  extend T::Sig

  # The method that `attr_writer` defines uses the whole `attr_writer :foo`
  # as the loc of its return value. There's no expression to wrap.
  sig {params(foo: Integer).returns(String)}
  attr_writer :foo # error: Expected `String` but found `Integer` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def branches(c, u)
  if c
    0
  else
    u
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def missing_else(c, u)
  if c
    u
  end
end

sig {params(x: T.untyped).returns(String)}
def safe_navigation(x)
  x&.foo
end

sig {params(u: T.untyped).returns(T::Boolean)}
def and_and(u)
  u.foo? && u.bar?
end

sig {params(x: Integer, u: T.untyped).returns(String)}
def elsif_missing_else(x, u)
  if x == 1
    u
  elsif x == 2
    u
  end
end

sig {params(x: T.any(Integer, String, Symbol), u: T.untyped).returns(String)}
def case_missing_else(x, u)
  case x
  when Integer then u
  when String then u
  end
end

sig {params(u: T.untyped).returns(T::Boolean)}
def and_and_block(u)
  u.all? { |k| k.foo } && u.bar
end

sig {returns(Integer)}
def or_assign_ivar
  @or_assign_ivar ||= 'str'
end

sig {params(h: T::Hash[T.untyped, Integer], k: T.untyped).returns(String)}
def or_assign_index(h, k)
  h[k] ||= 0
end # error: Expected `String` but found `Integer` for method result type

module ConstantBranch
  X = T.let('', String)
end

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def constant_branch(c, u)
  if c
    ConstantBranch::X
  else
    u
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def modifier_if_missing_else(c, u)
  u if c
end

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def block_branch(c, u)
  if c
    [1].map { |x| x.to_s }
  else
    u
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def block_receiver_branch(c, u)
  if c
    [1].to_h { |g| [g, g] }.reject { |k, _| k.nil? }
  else
    u
  end
end
