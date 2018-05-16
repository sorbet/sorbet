# typed: true
class StringScanner < Object
  sig(
      arg0: String,
      arg1: T.any(TrueClass, FalseClass),
  )
  .returns(StringScanner)
  def self.new(arg0, arg1=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def eos?(); end

  sig.returns(String)
  def getch(); end

  sig(
      arg0: Regexp,
  )
  .returns(String)
  def scan(arg0); end
end

class StringScanner::Error < StandardError
end
