# typed: true

class C
  include A
  include B

  def bar; foo; end
end
