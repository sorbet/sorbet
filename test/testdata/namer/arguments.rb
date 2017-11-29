# @typed
class A
  def take_arguments(a, b=1, *c, d:, e:2, **f, &g)
    [a,b,c,d,e,f,g]
    h = 1
    proc do |a, b=1, *c, d:, e:2, **f, &g; h|
      [a,b,c,d,e,f,g,h]
    end
  end
end
