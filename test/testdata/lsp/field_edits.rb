# typed: true
class C
  @@foo = T.let(3, Integer)

  def bar
    @@quz
  end
end
