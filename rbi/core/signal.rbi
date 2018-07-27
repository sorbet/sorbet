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
      arg0: T.any(Integer, String, Symbol),
  )
  .returns(T.any(String, Proc))
  sig(
      arg0: T.any(Integer, String, Symbol),
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.any(String, Proc))
  def self.trap(arg0, &blk); end
end
