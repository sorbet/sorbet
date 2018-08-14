# typed: true
class Method < Object
  Sorbet.sig.returns(Proc)
  def to_proc; end
end
