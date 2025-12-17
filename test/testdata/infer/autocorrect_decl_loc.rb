# typed: true
extend T::Sig

sig {params(klass: Class).void}
def bad_arg(klass)
  loop do
    klass = klass.superclass
    #       ^^^^^^^^^^^^^^^^ error: Changing the type of a variable is not permitted in loops and blocks
  end
end

sig {params(klass: Class).void}
def multiline_bad_arg(
  klass
)
  loop do
    klass = klass.superclass
    #       ^^^^^^^^^^^^^^^^ error: Changing the type of a variable is not permitted in loops and blocks
  end
end

sig {params(klass: Class).void}
def bad_arg_defined_twice(klass)
  loop do
    klass = klass.superclass
    #       ^^^^^^^^^^^^^^^^ error: Changing the type of a variable is not permitted in loops and blocks
  end
end
def bad_arg_defined_twice(klass)
  loop do
    klass = klass.superclass
    #       ^^^^^^^^^^^^^^^^ error: Changing the type of a variable is not permitted in loops and blocks
  end
end
