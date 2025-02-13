# typed: true

class A
  def f
    f {&:foo}
    # ^^^^^^^ parser-error: block pass should not be enclosed in curly braces
  end

  def g
    g {&(1+1)}
    # ^^^^^^^^ parser-error: block pass should not be enclosed in curly braces
  end

  def h
    h {&(T.unsafe(true) ? :sneeze
                        : :cough)}
    # parser-error: block pass should not be enclosed in curly braces
  end
end
