# typed: true
module Marshal
  MAJOR_VERSION = T.let(T.unsafe(nil), Integer)
  MINOR_VERSION = T.let(T.unsafe(nil), Integer)

  Sorbet.sig(
      arg0: Object,
      arg1: IO,
      arg2: Integer,
  )
  .returns(Object)
  Sorbet.sig(
      arg0: Object,
      arg1: Integer,
  )
  .returns(Object)
  def self.dump(arg0, arg1=_, arg2=_); end

  Sorbet.sig(
      arg0: String,
      arg1: Proc,
  )
  .returns(Object)
  def self.load(arg0, arg1=_); end
end
