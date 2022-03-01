# typed: true
class Foo
  extend T::Sig

  sig {bind(Integer).returns(Integer)}
  def int
    self + 2
  end

  sig {bind(Integer).bind(Symbol).returns(Integer)} # error: Malformed `bind`: Multiple calls to `.bind`
  def double_bind; 1; end

  sig {bind(Bar).returns(Integer)}
  def bar; 1; end

  sig {bind(T.attached_class).returns(Integer)}
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Using `bind` is not permitted here
  def self.bar; 1; end

  sig {bind(T.any(Integer, String)).void} # error: Malformed `bind`: Can only bind to simple class names
  def too_complex; end

  sig {params(blk: T.proc.params(x: T.proc.bind(T.attached_class).void).void).void}
                                  # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Using `bind` is not permitted here
  def self.lambda_argument(&blk); end

  sig {returns(T.proc.returns(T.proc.bind(T.attached_class).void))}
                            # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Using `bind` is not permitted here
  def self.lambda_returns
    T.unsafe(nil)
  end
end

class Bar
end
