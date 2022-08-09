# typed: true

extend T::Sig

sig {params(x: T.any(Integer, String)).void}
def normal_usage(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    T.absurd(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def forgotten_case(x)
  case x
  when Integer
    puts x
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {params(x: T.any(Integer, String)).void}
def got_all_cases_but_untyped(x)
  y = T.unsafe(x)
  case y
  when Integer
    puts y
  when String
    puts y
  else
    T.absurd(y) # error: Control flow could reach `T.absurd` because argument was `T.untyped`
  end
end

sig {params(x: T.any(Integer, String, T::Array[Integer])).void}
def forgotten_two_cases(x)
  case x
  when Integer
    puts x
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `T.any(String, T::Array[Integer])` wasn't handled
  end
end

sig {params(x: T.any(String, T::Array[Integer])).void}
def missing_case_with_generic(x)
  case x
  when Array
    puts x
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {params(x: T.any(T::Array[String], T::Array[Integer])).void}
def ok_when_generic_cases_overlap(x)
  case x
  when Array
    puts x
  else
    T.absurd(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def dead_code_before(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    puts 1 # error: This code is unreachable
    T.absurd(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def dead_code_after(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    T.absurd(x)
    puts 1 # error: This code is unreachable
  end
end

sig {params(x: T.any(Integer, String)).void}
def cant_alias_local_variables(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    # Hello future Sorbet contributor.
    # It's not that we MUST have an error here, but rather that we were fine
    # with it, and couldn't see a simple solution that avoided emitting one.
    y = x
    #   ^ error: This code is unreachable
    T.absurd(y)
  end
end

sig {params(x: T.any(Integer, String)).void}
def normal_usage_with_isa(x)
  if x.is_a?(Integer)
    puts x
  elsif x.is_a?(String)
    puts x
  else
    T.absurd(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def missing_case_with_isa(x)
  if x.is_a?(Integer)
    puts x
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {params(x: Integer).void}
def enforce_something_is_always_non_nil(x)
  if !x.nil?
    puts x
  else
    T.absurd(x)
  end
end

sig {params(x: Integer).void}
def error_when_predicate_always_true(x)
  if !x.nil?
    puts 1 # not a dead code error, because it's always true!
    # This is a strange error message given the test case.
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `Integer` wasn't handled
  end
end

# --- trying to subvert normal usage ------------------------------------------

def only_absurd
  temp1 = T.let(T.unsafe(nil), T.noreturn)
  T.absurd(temp1) # error: This code is unreachable
end

sig {params(x: T.noreturn).returns(T.noreturn)}
def cant_call_only_absurd(x)
  T.absurd(x) # error: This code is unreachable
end

# --- reasonable usage on global, class, instance, and local variables --------
$some_global = T.let(true, T::Boolean)

sig {void}
def absurd_not_reached_on_global_var
  case $some_global
  when TrueClass
  when FalseClass
  else T.absurd($some_global) # error: Control flow could reach `T.absurd` because argument was `T.untyped`
  end
end

sig {void}
def absurd_reached_on_global_var
  # Note T.nilable, because $some_global was not declared in this scope.
  T.absurd($some_global) if !$some_global # error: Control flow could reach `T.absurd` because the type `T.nilable(FalseClass)` wasn't handled
end

class SomeClass
  extend T::Sig

  @@some_class_var = T.let(true, T::Boolean)

  sig {void}
  def initialize
    @some_instance_var = T.let(true, T::Boolean)
  end

  sig {void}
  def absurd_not_reached_on_class_var
    case @@some_class_var
    when TrueClass
    when FalseClass
    else
      T.absurd(@@some_class_var)
    end
  end

  sig {void}
  def absurd_reached_on_class_var
    T.absurd(@@some_class_var) if !@@some_class_var # error: Control flow could reach `T.absurd` because the type `FalseClass` wasn't handled
  end

  sig {void}
  def absurd_not_reached_on_instance_var
    case @some_instance_var
    when TrueClass
    when FalseClass
    else
      T.absurd(@some_instance_var)
    end
  end

  sig {void}
  def absurd_reached_on_instance_var
    T.absurd(@some_instance_var) if !@some_instance_var # error: Control flow could reach `T.absurd` because the type `FalseClass` wasn't handled
  end
end

sig {params(some_local_var: T::Boolean).void}
def absurd_reached_on_local_var(some_local_var)
  T.absurd(some_local_var) if !some_local_var # error: Control flow could reach `T.absurd` because the type `FalseClass` wasn't handled
end

# --- incorrect usage ---------------------------------------------------------

sig {params(x: Integer).void}
def not_enough_args(x)
  T.absurd if x.nil? # error: `T.absurd` expects exactly one argument but got `0`
end

sig {params(x: Integer).void}
def too_many_args(x)
  T.absurd(nil, nil) if x.nil? # error: `T.absurd` expects exactly one argument but got `2`
end

sig {params(x: Integer).void}
def too_many_args_with_keyword_arg(x)
  T.absurd(nil, x: nil) if x.nil? # error: `T.absurd` does not accept keyword arguments
end

sig {params(x: Integer).void}
def only_keyword_arg(x)
  T.absurd(x: nil) if x.nil? # error: `T.absurd` does not accept keyword arguments
end

sig {returns(T.any(Integer, String))}
def looks_like_a_variable
  T.unsafe(nil) ? 0 : ''
end

sig {void}
def rejects_absurd_on_method_call_that_looks_like_a_variable
  case looks_like_a_variable
  when Integer
  when String
  else
    T.absurd(looks_like_a_variable) # error: `T.absurd` expects to be called on a variable, not a method call
  end
end

sig{void}
def rejects_absurd_on_integer_literal
  T.absurd(42) # error: `T.absurd` expects to be called on a variable
end
