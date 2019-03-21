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

  sig {returns(T.nilable(Thread))}
  def kill; end
end

class Thread::Backtrace::Location
  sig {returns(String)}
  def absolute_path(); end

  sig {returns(String)}
  def base_label(); end

  sig {returns(String)}
  def label(); end

  sig {returns(Integer)}
  def lineno(); end

  sig {returns(String)}
  def path(); end
end
