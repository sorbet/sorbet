# @typed

class Test

  # Simplest test case: We can remember that an `Object` variable is
  # known to be truthy
  sig(x: Object).returns(T.untyped)
  def test_simple(x)
    if x
      if x
        1
      else
        0 # error: This code is unreachable
      end
    end
  end

  # Minimized test case from real code
  sig(
    x: T.nilable(String),
    y: Object,
  ).returns(T.untyped)
  def test_flow(x, y)
    y || !x || x.empty?
  end

  # Approximately what the above desugars into
  sig(
    x: T.nilable(String),
    y: Object,
  ).returns(T.untyped)
  def test_desugared(x, y)
    t1 = if y
           y
         else
           !x
         end

    # Prevent the above CFG blocks from chaining directly into the
    # lower blocks; force them to go through an intermediate block
    puts("hi")

    t2 = if t1
           t1
         else
           x.empty?
         end
    t2
  end

  # Test that we can merge truthiness from two blocks into the
  # successor block.
  def test_merge(x, y)
    if x
      if y then 0 else return false end
      puts("l")
    else
      if y then 0 else return false end
      puts("r")
    end

    puts("hi")

    if y
      0
    else
      1 # error: This code is unreachable
    end
  end
end
