# typed: true
class Dir < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: String)

  Sorbet.sig(
      arg0: T.any(String, Pathname),
  )
  .returns(Integer)
  type_parameters(:U).sig(
      arg0: T.any(String, Pathname),
      blk: T.proc(arg0: String).returns(T.type_parameter(:U)),
  )
  .returns(T.type_parameter(:U))
  def self.chdir(arg0=T.unsafe(nil), &blk); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(Integer)
  def self.chroot(arg0); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(Integer)
  def self.delete(arg0); end

  Sorbet.sig(
      arg0: String,
      arg1: Encoding,
  )
  .returns(T::Array[String])
  def self.entries(arg0, arg1=T.unsafe(nil)); end

  Sorbet.sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.exist?(file); end

  Sorbet.sig(
      dir: String,
      arg0: Encoding,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(NilClass)
  Sorbet.sig(
      dir: String,
      arg0: Encoding,
  )
  .returns(Enumerator[String])
  def self.foreach(dir, arg0=T.unsafe(nil), &blk); end

  Sorbet.sig.returns(String)
  def self.getwd(); end

  Sorbet.sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
  )
  .returns(T::Array[String])
  Sorbet.sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(NilClass)
  def self.glob(pattern, flags=T.unsafe(nil), &blk); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def self.home(arg0=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(Integer)
  def self.mkdir(arg0, arg1=T.unsafe(nil)); end

  Sorbet.sig(
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
  def self.open(arg0, arg1=T.unsafe(nil), &blk); end

  Sorbet.sig.returns(String)
  def self.pwd(); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(Integer)
  def self.rmdir(arg0); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(Integer)
  def self.unlink(arg0); end

  Sorbet.sig.returns(NilClass)
  def close(); end

  Sorbet.sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig.returns(Enumerator[String])
  def each(&blk); end

  Sorbet.sig.returns(Integer)
  def fileno(); end

  Sorbet.sig(
      arg0: String,
      arg1: Encoding,
  )
  .void
  def initialize(arg0, arg1=T.unsafe(nil)); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(T.nilable(String))
  def path(); end

  Sorbet.sig.returns(Integer)
  def pos(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def pos=(arg0); end

  Sorbet.sig.returns(T.nilable(String))
  def read(); end

  Sorbet.sig.returns(T.self_type)
  def rewind(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(T.self_type)
  def seek(arg0); end

  Sorbet.sig.returns(Integer)
  def tell(); end

  Sorbet.sig.returns(T.nilable(String))
  def to_path(); end

  Sorbet.sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
  )
  .returns(T::Array[String])
  Sorbet.sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(NilClass)
  def self.[](pattern, flags=T.unsafe(nil), &blk); end
end
