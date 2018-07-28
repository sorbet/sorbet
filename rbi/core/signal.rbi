# typed: true
module Signal
  sig.returns(T::Hash[String, Integer])
  def self.list(); end

  sig(
      arg0: Integer,
  )
  .returns(T.nilable(String))
  def self.signame(arg0); end

  sig(
      signal: T.any(Integer, String, Symbol),
      command: BasicObject,
  )
  .returns(T.any(String, Proc))
  sig(
      signal: T.any(Integer, String, Symbol),
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.any(String, Proc))
  def self.trap(signal, command=_, &blk); end
end
