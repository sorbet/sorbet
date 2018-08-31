# typed: true
module Signal
  Sorbet.sig.returns(T::Hash[String, Integer])
  def self.list(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(T.nilable(String))
  def self.signame(arg0); end

  Sorbet.sig(
      signal: T.any(Integer, String, Symbol),
      command: BasicObject,
  )
  .returns(T.any(String, Proc))
  Sorbet.sig(
      signal: T.any(Integer, String, Symbol),
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.any(String, Proc))
  def self.trap(signal, command=T.unsafe(nil), &blk); end
end
