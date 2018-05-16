# typed: true
class Dir < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: String)

  sig(
      arg0: T.any(String, Pathname),
  )
  .returns(Integer)
  type_parameters(:U).sig(
      arg0: T.any(String, Pathname),
      blk: T.proc(arg0: String).returns(T.type_parameter(:U)),
  )
  .returns(T.type_parameter(:U))
  def self.chdir(arg0=_, &blk); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def self.chroot(arg0); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def self.delete(arg0); end

  sig(
      arg0: String,
      arg1: Encoding,
  )
  .returns(T::Array[String])
  def self.entries(arg0, arg1=_); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.exist?(file); end

  sig(
      dir: String,
      arg0: Encoding,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(NilClass)
  sig(
      dir: String,
      arg0: Encoding,
  )
  .returns(Enumerator[String])
  def self.foreach(dir, arg0=_, &blk); end

  sig.returns(String)
  def self.getwd(); end

  sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
  )
  .returns(T::Array[String])
  sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(NilClass)
  def self.glob(pattern, flags=_, &blk); end

  sig(
      arg0: String,
  )
  .returns(String)
  def self.home(arg0=_); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(Integer)
  def self.mkdir(arg0, arg1=_); end

  sig(
      arg0: String,
      arg1: Encoding,
  )
  .returns(Dir)
  type_parameters(:U).sig(
      arg0: String,
      arg1: Encoding,
      blk: T.proc(arg0: Dir).returns(T.type_parameter(:U)),
  )
  .returns(T.type_parameter(:U))
  def self.open(arg0, arg1=_, &blk); end

  sig.returns(String)
  def self.pwd(); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def self.rmdir(arg0); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def self.unlink(arg0); end

  sig.returns(NilClass)
  def close(); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(Dir)
  sig.returns(Enumerator[String])
  def each(&blk); end

  sig.returns(Integer)
  def fileno(); end

  sig(
      arg0: String,
      arg1: Encoding,
  )
  .returns(Object)
  def initialize(arg0, arg1=_); end

  sig.returns(String)
  def inspect(); end

  sig.returns(T.nilable(String))
  def path(); end

  sig.returns(Integer)
  def pos(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def pos=(arg0); end

  sig.returns(T.nilable(String))
  def read(); end

  sig.returns(Dir)
  def rewind(); end

  sig(
      arg0: Integer,
  )
  .returns(Dir)
  def seek(arg0); end

  sig.returns(Integer)
  def tell(); end

  sig.returns(T.nilable(String))
  def to_path(); end

  sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
  )
  .returns(T::Array[String])
  sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(NilClass)
  def self.[](pattern, flags=_, &blk); end
end
