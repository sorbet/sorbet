class A
  def take_arguments(a, b=1, *c, d:, e:2, **f, &g)
    proc do |a, b=1, *c, d:, e:2, **f, &g; h|
    end
  end
end
