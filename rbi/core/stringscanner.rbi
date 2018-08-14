# typed: true
class StringScanner < Object
  Sorbet.sig(
      arg0: String,
      arg1: T.any(TrueClass, FalseClass),
  )
  .returns(StringScanner)
  def self.new(arg0, arg1=_); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def eos?(); end

  Sorbet.sig.returns(String)
  def getch(); end

  Sorbet.sig(
      arg0: Regexp,
  )
  .returns(String)
  def scan(arg0); end
end
