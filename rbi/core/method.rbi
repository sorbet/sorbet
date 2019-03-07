# typed: true
class Method < Object
  sig {returns(Proc)}
  def to_proc; end

  sig {params(args: T.untyped).returns(T.untyped)}
  def call(*args);end
end
