# typed: strict

class Ripper
  class << self
    sig(src: String, filename: String, lineno: Integer).returns(T::Array[T.untyped])
    def sexp(src, filename = "-", lineno = 1); end

    sig(src: String, filename: String, lineno: Integer).returns(T::Array[T.untyped])
    def sexp_raw(src, filename = "-", lineno = 1); end

    sig(src: String, pattern: String, n: Integer).returns(String)
    def slice(src, pattern, n = 0); end

    sig(src: String, filename: String, lineno: Integer).returns(T::Array[String])
    def tokenize(src, filename = "-", lineno = 1); end
  end
end
