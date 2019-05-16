# typed: core
class Dir < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: String)

  sig do
    params(
        arg0: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  sig do
    type_parameters(:U).params(
        arg0: T.any(String, Pathname),
        blk: T.proc.params(arg0: String).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def self.chdir(arg0=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def self.chroot(arg0); end

  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def self.delete(arg0); end

  sig do
    params(
        arg0: String,
        arg1: Encoding,
    )
    .returns(T::Array[String])
  end
  def self.entries(arg0, arg1=T.unsafe(nil)); end

  sig do
    params(
        file: String,
    )
    .returns(T::Boolean)
  end
  def self.exist?(file); end

  sig do
    params(
        dir: String,
        arg0: Encoding,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(NilClass)
  end
  sig do
    params(
        dir: String,
        arg0: Encoding,
    )
    .returns(Enumerator[String])
  end
  def self.foreach(dir, arg0=T.unsafe(nil), &blk); end

  sig {returns(String)}
  def self.getwd(); end

  sig do
    params(
        pattern: T.any(String, T::Array[String]),
        flags: Integer,
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        pattern: T.any(String, T::Array[String]),
        flags: Integer,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(NilClass)
  end
  def self.glob(pattern, flags=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def self.home(arg0=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(Integer)
  end
  def self.mkdir(arg0, arg1=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
        arg1: Encoding,
    )
    .returns(Dir)
  end
  sig do
    type_parameters(:U).params(
        arg0: String,
        arg1: Encoding,
        blk: T.proc.params(arg0: Dir).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def self.open(arg0, arg1=T.unsafe(nil), &blk); end

  sig {returns(String)}
  def self.pwd(); end

  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def self.rmdir(arg0); end

  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def self.unlink(arg0); end

  sig {returns(NilClass)}
  def close(); end

  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(Enumerator[String])}
  def each(&blk); end

  sig {returns(Integer)}
  def fileno(); end

  sig do
    params(
        arg0: String,
        arg1: Encoding,
    )
    .void
  end
  def initialize(arg0, arg1=T.unsafe(nil)); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(T.nilable(String))}
  def path(); end

  sig {returns(Integer)}
  def pos(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def pos=(arg0); end

  sig {returns(T.nilable(String))}
  def read(); end

  sig {returns(T.self_type)}
  def rewind(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T.self_type)
  end
  def seek(arg0); end

  sig {returns(Integer)}
  def tell(); end

  sig {returns(T.nilable(String))}
  def to_path(); end

  sig do
    params(
        pattern: T.any(String, T::Array[String]),
        flags: Integer,
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        pattern: T.any(String, T::Array[String]),
        flags: Integer,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(NilClass)
  end
  def self.[](pattern, flags=T.unsafe(nil), &blk); end
end
