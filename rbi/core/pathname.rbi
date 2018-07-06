# typed: true
class Pathname < Object
  SAME_PATHS = T.let(T.unsafe(nil), Proc)
  SEPARATOR_LIST = T.let(T.unsafe(nil), String)
  SEPARATOR_PAT = T.let(T.unsafe(nil), Regexp)
  TO_PATH = T.let(T.unsafe(nil), Symbol)

  sig.returns(Pathname)
  def self.getwd(); end

  sig(
      p1: String,
      p2: String,
  )
  .returns(T::Array[Pathname])
  def self.glob(p1, p2=_); end

  sig(p: T.any(String, Pathname)).void
  def initialize(p); end

  sig(
      other: T.any(String, Pathname),
  )
  .returns(Pathname)
  def +(other); end

  sig(
      p1: BasicObject,
  )
  .returns(T.nilable(Integer))
  def <=>(p1); end

  sig(
      p1: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(p1); end

  sig(
      p1: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(p1); end

  sig.returns(T.any(TrueClass, FalseClass))
  def absolute?(); end

  sig(
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def ascend(&blk); end

  sig.returns(Time)
  def atime(); end

  sig(
      p1: String,
  )
  .returns(Pathname)
  def basename(p1=_); end

  sig(
      length: Integer,
      offset: Integer,
  )
  .returns(String)
  def binread(length=_, offset=_); end

  sig(
      arg0: String,
      offset: Integer,
  )
  .returns(Integer)
  def binwrite(arg0, offset=_); end

  sig.returns(Time)
  def birthtime(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def blockdev?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def chardev?(); end

  sig(
      with_directory: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Pathname])
  def children(with_directory); end

  sig(
      mode: Integer,
  )
  .returns(Integer)
  def chmod(mode); end

  sig(
      owner: Integer,
      group: Integer,
  )
  .returns(Integer)
  def chown(owner, group); end

  sig(
      consider_symlink: T.any(TrueClass, FalseClass),
  )
  .returns(T.untyped)
  def cleanpath(consider_symlink=_); end

  sig.returns(Time)
  def ctime(); end

  sig.returns(T.untyped)
  def delete(); end

  sig(
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def descend(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def directory?(); end

  sig.returns(Pathname)
  def dirname(); end

  sig(
      with_directory: T.any(TrueClass, FalseClass),
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def each_child(with_directory, &blk); end

  sig(
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def each_entry(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.untyped)
  sig.returns(Enumerator[String])
  def each_filename(&blk); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.untyped)
  sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def each_line(sep=_, limit=_, &blk); end

  sig.returns(T::Array[Pathname])
  def entries(); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def executable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def executable_real?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def exist?(); end

  sig(
      p1: T.any(String, Pathname),
  )
  .returns(Pathname)
  def expand_path(p1=_); end

  sig.returns(String)
  def extname(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def file?(); end

  sig(
      ignore_error: T.any(TrueClass, FalseClass),
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  sig(
      ignore_error: T.any(TrueClass, FalseClass),
  )
  .returns(Enumerator[Pathname])
  def find(ignore_error, &blk); end

  sig(
      pattern: String,
      flags: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  def fnmatch(pattern, flags=_); end

  sig.returns(T.self_type)
  def freeze(); end

  sig.returns(String)
  def ftype(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def grpowned?(); end

  sig(
      args: T.any(String, Pathname),
  )
  .returns(Pathname)
  def join(*args); end

  sig(
      mode: Integer,
  )
  .returns(Integer)
  def lchmod(mode); end

  sig(
      owner: Integer,
      group: Integer,
  )
  .returns(Integer)
  def lchown(owner, group); end

  sig.returns(File::Stat)
  def lstat(); end

  sig(
      old: String,
  )
  .returns(Integer)
  def make_link(old); end

  sig(
      p1: String,
  )
  .returns(Integer)
  def mkdir(p1); end

  sig.returns(T.untyped)
  def mkpath(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def mountpoint?(); end

  sig.returns(Time)
  def mtime(); end

  sig(
      mode: String,
      perm: String,
      opt: Integer,
  )
  .returns(File)
  type_parameters(:T).sig(
      mode: String,
      perm: String,
      opt: Integer,
      blk: T.proc(arg0: File).returns(T.type_parameter(:T)),
  )
  .returns(T.type_parameter(:T))
  def open(mode=_, perm=_, opt=_, &blk); end

  sig(
      arg0: Encoding,
  )
  .returns(Dir)
  type_parameters(:U).sig(
      arg0: Encoding,
      blk: T.proc(arg0: Dir).returns(T.type_parameter(:U)),
  )
  .returns(T.type_parameter(:U))
  def opendir(arg0=_, &blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def owned?(); end

  sig.returns(Pathname)
  def parent(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def pipe?(); end

  sig(
      length: Integer,
      offset: Integer,
      open_args: Integer,
  )
  .returns(String)
  def read(length=_, offset=_, open_args=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def readable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def readable_real?(); end

  sig(
      sep: String,
      limit: Integer,
      open_args: Integer,
  )
  .returns(T::Array[String])
  def readlines(sep=_, limit=_, open_args=_); end

  sig.returns(String)
  def readlink(); end

  sig(
      p1: String,
  )
  .returns(String)
  def realdirpath(p1=_); end

  sig(
      p1: String,
  )
  .returns(String)
  def realpath(p1=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def relative?(); end

  sig(
      base_directory: T.any(String, Pathname),
  )
  .returns(Pathname)
  def relative_path_from(base_directory); end

  sig(
      p1: String,
  )
  .returns(Integer)
  def rename(p1); end

  sig.returns(Integer)
  def rmdir(); end

  sig.returns(Integer)
  def rmtree(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def root?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def setgid?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def setuid?(); end

  sig.returns(Integer)
  def size(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def size?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def socket?(); end

  sig.returns([Pathname, Pathname])
  def split(); end

  sig.returns(File::Stat)
  def stat(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def sticky?(); end

  sig(
      args: String,
  )
  .returns(Pathname)
  def sub(*args); end

  sig(
      p1: String,
  )
  .returns(Pathname)
  def sub_ext(p1); end

  sig(
      old: String,
  )
  .returns(Integer)
  sig.returns(T.any(TrueClass, FalseClass))
  def symlink?(old=_); end

  sig(
      mode: Integer,
      perm: Integer,
  )
  .returns(Integer)
  def sysopen(mode=_, perm=_); end

  sig.returns(T.self_type)
  def taint(); end

  sig.returns(String)
  def to_path(); end

  sig(
      length: Integer,
  )
  .returns(Integer)
  def truncate(length); end

  sig.returns(Integer)
  def unlink(); end

  sig.returns(T.self_type)
  def untaint(); end

  sig(
      atime: Time,
      mtime: Time,
  )
  .returns(Integer)
  def utime(atime, mtime); end

  sig.returns(T.any(TrueClass, FalseClass))
  def world_readable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def world_writable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def writable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def writable_real?(); end

  sig(
      arg0: String,
      offset: Integer,
      open_args: Integer,
  )
  .returns(Integer)
  def write(arg0, offset=_, open_args=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end

  sig.returns(Pathname)
  def self.pwd(); end

  sig(
      other: T.any(String, Pathname),
  )
  .returns(Pathname)
  def /(other); end

  sig.returns(String)
  def to_s(); end
end
