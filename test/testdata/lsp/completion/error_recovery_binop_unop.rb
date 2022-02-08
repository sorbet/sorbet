# typed: true

class A
  def f(a, b, c)
    if a + 
    #      ^ completion:
    # We would like the above line to be "completion: a, b, c, ...", but this doesn't work yet. If you make it work,
    # please feel free to adjust this test.
    end
  # ^^^ error: unexpected token "end"
  end
end