# typed: false

class A
  # This method actually parses normally, but because there was a syntax error
  # *somewhere* in the file, we eagerly report an error here attempting to
  # recover from it.
  def test1
    if x.f
    puts 'inside if'
  end # error: Hint: closing "end" token was not indented as far as "if" token
    # We this ends up at the class top-level, not inside test1
    puts 'after if but inside test1'
  end

  # This ends up outside of `A`, but maybe it's better than showing nothing in
  # the whole file.
  def test2
    if x.f
  end # error: Hint: closing "end" token was not indented as far as "if" token
end # error: unexpected token "end"
