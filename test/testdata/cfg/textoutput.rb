# typed: true

class A
  def f(x, y)
    x.map do |e|
      g(e, y)
    end
  end

  def g(x, y)
    T.cast(x, String) + y
  end
end

begin
  A.new.f({ :z => 3, :w => "string" }, true)
rescue => e
  p "whoops"
ensure
  p "done"
end
