# typed: false

class A
  # This method actually parses normally, but because there was a syntax error
  # *somewhere* in the file, we eagerly report an error here attempting to
  # recover from it.
  def test1
    if x.f # error: Hint: this "if" token might not be properly closed
    puts 'inside if'
  end
    # We this ends up at the class top-level, not inside test1
    puts 'after if but inside test1'
  end

  # This ends up outside of `A`, but maybe it's better than showing nothing in
  # the whole file.
  def test2
    if x.f # error: Hint: this "if" token might not be properly closed
  end
end # error: unexpected token "end of file"
