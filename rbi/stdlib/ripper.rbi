# typed: strict

class Ripper
  Sorbet.sig(src: String, filename: String, lineno: Integer).returns(T::Array[T.untyped])
  def self.sexp(src, filename = "-", lineno = 1); end

  Sorbet.sig(src: String, filename: String, lineno: Integer).returns(T::Array[T.untyped])
  def self.sexp_raw(src, filename = "-", lineno = 1); end

  Sorbet.sig(src: String, pattern: String, n: Integer).returns(String)
  def self.slice(src, pattern, n = 0); end

  Sorbet.sig(src: String, filename: String, lineno: Integer).returns(T::Array[String])
  def self.tokenize(src, filename = "-", lineno = 1); end
end
