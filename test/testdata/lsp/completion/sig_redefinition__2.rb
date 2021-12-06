# typed: true

class A
  def foo; end

  sig # error: no block
  #  ^ apply-completion: [B] item: 0
  def bar; end
end
