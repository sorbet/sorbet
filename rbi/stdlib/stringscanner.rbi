# typed: true
class StringScanner < Object
  sig do
    params(
        arg0: String,
        arg1: T::Boolean,
    )
    .returns(StringScanner)
  end
  def self.new(arg0, arg1=T.unsafe(nil)); end

  sig {returns(T::Boolean)}
  def eos?(); end

  sig {returns(String)}
  def getch(); end

  sig do
    params(
        arg0: Regexp,
    )
    .returns(String)
  end
  def scan(arg0); end
end
