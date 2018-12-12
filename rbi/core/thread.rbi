# typed: strict

class Thread < Object
  sig {returns(Thread)}
  def self.current; end

  sig {returns(Thread)}
  def self.main; end

  sig {params(key: T.any(String, Symbol)).returns(T.untyped)}
  def [](key); end

  sig {params(key: T.any(String, Symbol), value: T.untyped).returns(T.untyped)}
  def []=(key, value); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def alive?; end
end
