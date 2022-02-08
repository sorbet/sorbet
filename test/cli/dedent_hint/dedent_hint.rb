# typed: false

class A
  def test0
    x = nil
    if
  end

  def test1
    x = nil
    if x
  end

  def test2
    x = nil
    if x.
  end

  def test3
    x = nil
    if x.f
  end

  # -- These should still have no errors even in indentationAware mode --

  def no_syntax_error_1
    x = if y
    end
  end

  def no_syntax_error_2
    x = if y
        end
  end

  def no_syntax_error_3
      # this is a comment with weird indent
    x = if y
    end
    puts 'after'
  end
end
