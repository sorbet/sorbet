# typed: true
# enable-experimental-rbs-comments: true

#: (Integer | String) -> void
def normal_usage(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    x #: absurd
  end
end

#: (Integer | String) -> void
def forgotten_case(x)
  case x
  when Integer
    puts x
  else
    x #: absurd # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

#: (Integer | String) -> void
def got_all_cases_but_untyped(x)
  y = T.unsafe(x)
  case y
  when Integer
    puts y
  when String
    puts y
  else
    y #: absurd # error: Control flow could reach `T.absurd` because argument was `T.untyped`
  end
end

#: (Integer | String | Array[Integer]) -> void
def forgotten_two_cases(x)
  case x
  when Integer
    puts x
  else
    x #: absurd # error: Control flow could reach `T.absurd` because the type `T.any(String, T::Array[Integer])` wasn't handled
  end
end

#: (String | Array[Integer]) -> void
def missing_case_with_generic(x)
  case x
  when Array
    puts x
  else
    x #: absurd # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

#: (Array[String] | Array[Integer]) -> void
def ok_when_generic_cases_overlap(x)
  case x
  when Array
    puts x
  else
    x #: absurd
  end
end

#: (Integer | String) -> void
def dead_code_before(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    puts 1 # error: This code is unreachable
    x #: absurd
  end
end

#: (Integer | String) -> void
def dead_code_after(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    x #: absurd
    puts 1 # error: This code is unreachable
  end
end

#: (Integer | String) -> void
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
    y #: absurd
  end
end

#: (Integer | String) -> void
def normal_usage_with_isa(x)
  if x.is_a?(Integer)
    puts x
  elsif x.is_a?(String)
    puts x
  else
    x #: absurd
  end
end

#: (Integer | String) -> void
def missing_case_with_isa(x)
  if x.is_a?(Integer)
    puts x
  else
    x #: absurd # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

#: (Integer) -> void
def enforce_something_is_always_non_nil(x)
  if !x.nil?
    puts x
  else
    x #: absurd
  end
end

#: (Integer) -> void
def error_when_predicate_always_true(x)
  if !x.nil?
    puts 1 # not a dead code error, because it's always true!
    # This is a strange error message given the test case.
    x #: absurd # error: Control flow could reach `T.absurd` because the type `Integer` wasn't handled
  end
end

# --- trying to subvert normal usage ------------------------------------------

def only_absurd
  temp1 = T.let(T.unsafe(nil), T.noreturn)
  temp1 #: absurd
end

#: (bot) -> void
def allows_arg_noreturn(x)
  x #: absurd
  puts(x)
# ^^^^^^^ error: This code is unreachable
end

#: (bot, bot) -> void
def allows_args_noreturn(x, y)
  x #: absurd
  y #: absurd
  puts(x)
# ^^^^^^^ error: This code is unreachable
end

#: (Integer & String) -> void
def intersects_to_bottom(x)
  x #: absurd
  puts(x)
# ^^^^^^^ error: This code is unreachable
end

# --- reasonable usage on global, class, instance, and local variables --------
$some_global = true #: bool

#: -> void
def absurd_not_reached_on_global_var
  case $some_global
  when TrueClass
  when FalseClass
  else $some_global #: absurd # error: Control flow could reach `T.absurd` because argument was `T.untyped`
  end
end

#: -> void
def absurd_reached_on_global_var
  if !$some_global
    $some_global #: absurd # error: Control flow could reach `T.absurd` because the type `T.nilable(FalseClass)` wasn't handled
  end
end

class SomeClass
  @@some_class_var = true #: bool

  #: -> void
  def initialize
    @some_instance_var = true #: bool
  end

  #: -> void
  def absurd_not_reached_on_class_var
    case @@some_class_var
    when TrueClass
    when FalseClass
    else
      @@some_class_var #: absurd
    end
  end

  #: -> void
  def absurd_reached_on_class_var
    if !@@some_class_var
      @@some_class_var #: absurd # error: Control flow could reach `T.absurd` because the type `FalseClass` wasn't handled
    end
  end

  #: -> void
  def absurd_not_reached_on_instance_var
    case @some_instance_var
    when TrueClass
    when FalseClass
    else
      @some_instance_var #: absurd
    end
  end

  #: -> void
  def absurd_reached_on_instance_var
    if !@some_instance_var
      @some_instance_var #: absurd # error: Control flow could reach `T.absurd` because the type `FalseClass` wasn't handled
    end
  end
end

#: (bool) -> void
def absurd_reached_on_local_var(some_local_var)
  if !some_local_var
    some_local_var #: absurd # error: Control flow could reach `T.absurd` because the type `FalseClass` wasn't handled
  end
end

# --- incorrect usage ---------------------------------------------------------

#: (Integer) -> void
def not_enough_args(x)
  #: absurd # error: Unexpected RBS assertion comment found in `method`
end

#: (Integer) -> void
def too_many_args(x)
  if x.nil?
    nil #: absurd # error: `T.absurd` expects to be called on a variable
  end
end
