# typed: true

class A
  def foo; end
  def test_missing_rhs(x)
    y =
    #  ^ completion: x, y, foo, test_missing_rhs, ...
  end # error: unexpected token "end"

  def test_variable_after_method(x)
    puts 'before'
    y =
    #  ^ completion: x, y, foo, test_missing_rhs, ...
  end # error: unexpected token "end"

  def test_variable_end_same_line(x)
    puts 'before'
    y =end # error: unexpected token "end"
    #  ^ completion: x, y, foo, test_missing_rhs, ...

  def test_variable_end_same_line_after(x)
    # This one is admittedly janky. There's a note about it in the parser.
    puts 'before'
    y =end # error: unexpected token "end"
    #     ^ completion: x, y, foo, test_missing_rhs, ...
end
