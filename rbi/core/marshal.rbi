# typed: true
module Marshal
  MAJOR_VERSION = T.let(T.unsafe(nil), Integer)
  MINOR_VERSION = T.let(T.unsafe(nil), Integer)

  sig(
      arg0: Object,
      arg1: IO,
      arg2: Integer,
  )
  .returns(Object)
  sig(
      arg0: Object,
      arg1: Integer,
  )
  .returns(Object)
  def self.dump(arg0, arg1=_, arg2=_); end

  sig(
      arg0: String,
      arg1: Proc,
  )
  .returns(Object)
  def self.load(arg0, arg1=_); end
end
