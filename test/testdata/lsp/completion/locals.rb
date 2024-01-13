# typed: true
extend T::Sig

# This is not defined in any of the methods below.
# It should never show up in a completion result inside a method.
x_outside = nil

x_ # error: does not exist
# ^ completion: x_outside

def without_args
  x_1 = nil
  x_ # error: does not exist
  # ^ completion: x_1
end

def with_args(x_1)
  x_2 = nil
  x_ # error: does not exist
  # ^ completion: x_1, x_2
end

# We sort the local variable completion results by name.
def with_args_backwards(x_2)
  x_1 = nil
  x_ # error: does not exist
  # ^ completion: x_1, x_2
end

def with_kwargs(x_1, x_2:)
  x_0 = nil
  x_ # error: does not exist
  # ^ completion: x_0, x_1, x_2
end

# We don't look at which locals are actually in scope (and have no plans to)
def before_after
  x_1 = nil

  x_ # error: does not exist
  # ^ completion: x_1, x_2

  x_2 = 1
end

def already_valid_local
  x_1 = nil
  x_1
  #  ^ completion: x_1
end

class A
  def a_method; end
end

# If there's an explicit receiver, don't suggest similar locals.
def no_locals_after_dot()
  a_local = nil
  A.new.a_ # error: does not exist
  #       ^ completion: a_method
end

def no_duplicate_results
  # Catches regression: TreeMap should only look at assigns, not usages
  x_1 = nil
  puts x_1
  puts x_1
  x_ # error: does not exist
  # ^ completion: x_1
end

def not_all_locals
  y_2 = nil
  y_1 = nil
  x_2 = nil
  x_1 = nil
  x_ # error: does not exist
  # ^ completion: x_1, x_2
end

def non_rooted_match
  xxx_yyy_zzz = nil
  yyy # error: does not exist
  #  ^ completion: xxx_yyy_zzz
end

class Wrapper
  x_inside_class = nil
  x_ # error: does not exist
  # ^ completion: x_inside_class
end

class B
  def implicit_block_arg
    blk # error: does not exist
    #  ^ completion: (nothing)
  end
end
