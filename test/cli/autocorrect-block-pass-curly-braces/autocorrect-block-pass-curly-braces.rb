# typed: true

class A
  def f
    f {&:foo}
  end

  def g
    g {&(1+1)}
  end

  def h
    h {&(T.unsafe(true) ? :sneeze
                        : :cough)}
  end
end
