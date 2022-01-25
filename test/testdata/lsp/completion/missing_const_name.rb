# typed: true

class Outer
  class Middle
    class Inner
    end
  end
end

def test_constant_completion_with_no_name
  Outer::
  #      ^ completion: Middle
  #    ^^ error: expected constant name following "::"
end

def test_constant_completion_adjacent_missing_names
  # No parse error here because the constant on the next line is allowed by Ruby
  Outer::
  #      ^ completion: Middle

  # No completion results here because Outer::Outer does not resolve
  Outer::Middle:: # error: Unable to resolve constant `Outer`
  #              ^ completion: (nothing)
  #            ^^ error: expected constant name following "::"
end

def test_constant_completion_before_method
  # No parse error here because the constant on the next line is allowed by Ruby
  Outer::
  #      ^ completion: Middle

  puts
end

def test_constant_completion_before_keyword
  # No parse error here because the constant on the next line is allowed by Ruby
  Outer::
  #      ^ completion: Middle
  #    ^^ error: expected constant name following "::"

  begin; end
end

def test_constant_completion_before_variable(x)
  # No parse error here because the constant on the next line is allowed by Ruby
  Outer::
  #      ^ completion: Middle

  x # error: Method `x` does not exist on `T.class_of(Outer)`
end

# TODO(jez) cbase constants with no name aren't implemented yet
def test_constant_completion_empty_scope(x)
  ::
  # ^ completion: (nothing)
end # error: unexpected token "end"
