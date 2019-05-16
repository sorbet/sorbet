# typed: core
module Signal
  sig {returns(T::Hash[String, Integer])}
  def self.list(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T.nilable(String))
  end
  def self.signame(arg0); end

  sig do
    params(
        signal: T.any(Integer, String, Symbol),
        command: BasicObject,
    )
    .returns(T.any(String, Proc))
  end
  sig do
    params(
        signal: T.any(Integer, String, Symbol),
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.any(String, Proc))
  end
  def self.trap(signal, command=T.unsafe(nil), &blk); end
end
