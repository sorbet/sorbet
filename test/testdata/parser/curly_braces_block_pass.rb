# typed: true

class A
  def f
    f {&:foo}
    # ^^^^^^^ error: block pass should not be enclosed in curly braces
  end

  def g
    g {&(1+1)}
    # ^^^^^^^^ error: block pass should not be enclosed in curly braces
  end

  def h
    h {&(T.unsafe(true) ? :sneeze
                        : :cough)}
    # error: block pass should not be enclosed in curly braces
  end
end
