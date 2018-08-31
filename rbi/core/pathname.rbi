# typed: true
class Pathname < Object
  SAME_PATHS = T.let(T.unsafe(nil), Proc)
  SEPARATOR_LIST = T.let(T.unsafe(nil), String)
  SEPARATOR_PAT = T.let(T.unsafe(nil), Regexp)
  TO_PATH = T.let(T.unsafe(nil), Symbol)

  Sorbet.sig.returns(Pathname)
  def self.getwd(); end

  Sorbet.sig(
      p1: String,
      p2: String,
  )
  .returns(T::Array[Pathname])
  def self.glob(p1, p2=T.unsafe(nil)); end

  def initialize(p); end

  Sorbet.sig(
      other: T.any(String, Pathname),
  )
  .returns(Pathname)
  def +(other); end

  Sorbet.sig(
      p1: BasicObject,
  )
  .returns(T.nilable(Integer))
  def <=>(p1); end

  Sorbet.sig(
      p1: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(p1); end

  Sorbet.sig(
      p1: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(p1); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def absolute?(); end

  Sorbet.sig(
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def ascend(&blk); end

  Sorbet.sig.returns(Time)
  def atime(); end

  Sorbet.sig(
      p1: String,
  )
  .returns(Pathname)
  def basename(p1=T.unsafe(nil)); end

  Sorbet.sig(
      length: Integer,
      offset: Integer,
  )
  .returns(String)
  def binread(length=T.unsafe(nil), offset=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: String,
      offset: Integer,
  )
  .returns(Integer)
  def binwrite(arg0, offset=T.unsafe(nil)); end

  Sorbet.sig.returns(Time)
  def birthtime(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def blockdev?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def chardev?(); end

  Sorbet.sig(
      with_directory: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Pathname])
  def children(with_directory); end

  Sorbet.sig(
      mode: Integer,
  )
  .returns(Integer)
  def chmod(mode); end

  Sorbet.sig(
      owner: Integer,
      group: Integer,
  )
  .returns(Integer)
  def chown(owner, group); end

  Sorbet.sig(
      consider_symlink: T.any(TrueClass, FalseClass),
  )
  .returns(T.untyped)
  def cleanpath(consider_symlink=T.unsafe(nil)); end

  Sorbet.sig.returns(Time)
  def ctime(); end

  Sorbet.sig.returns(T.untyped)
  def delete(); end

  Sorbet.sig(
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def descend(&blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def directory?(); end

  Sorbet.sig.returns(Pathname)
  def dirname(); end

  Sorbet.sig(
      with_directory: T.any(TrueClass, FalseClass),
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def each_child(with_directory, &blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def each_entry(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.untyped)
  Sorbet.sig.returns(Enumerator[String])
  def each_filename(&blk); end

  Sorbet.sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.untyped)
  Sorbet.sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def each_line(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  Sorbet.sig.returns(T::Array[Pathname])
  def entries(); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def executable?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def executable_real?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def exist?(); end

  Sorbet.sig(
      p1: T.any(String, Pathname),
  )
  .returns(Pathname)
  def expand_path(p1=T.unsafe(nil)); end

  Sorbet.sig.returns(String)
  def extname(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def file?(); end

  Sorbet.sig(
      ignore_error: T.any(TrueClass, FalseClass),
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  Sorbet.sig(
      ignore_error: T.any(TrueClass, FalseClass),
  )
  .returns(Enumerator[Pathname])
  def find(ignore_error, &blk); end

  Sorbet.sig(
      pattern: String,
      flags: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  def fnmatch(pattern, flags=T.unsafe(nil)); end

  Sorbet.sig.returns(T.self_type)
  def freeze(); end

  Sorbet.sig.returns(String)
  def ftype(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def grpowned?(); end

  Sorbet.sig(
      args: T.any(String, Pathname),
  )
  .returns(Pathname)
  def join(*args); end

  Sorbet.sig(
      mode: Integer,
  )
  .returns(Integer)
  def lchmod(mode); end

  Sorbet.sig(
      owner: Integer,
      group: Integer,
  )
  .returns(Integer)
  def lchown(owner, group); end

  Sorbet.sig.returns(File::Stat)
  def lstat(); end

  Sorbet.sig(
      old: String,
  )
  .returns(Integer)
  def make_link(old); end

  Sorbet.sig(
      p1: String,
  )
  .returns(Integer)
  def mkdir(p1); end

  Sorbet.sig.returns(T.untyped)
  def mkpath(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def mountpoint?(); end

  Sorbet.sig.returns(Time)
  def mtime(); end

  Sorbet.sig(
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
  def open(mode=T.unsafe(nil), perm=T.unsafe(nil), opt=T.unsafe(nil), &blk); end

  Sorbet.sig(
      arg0: Encoding,
  )
  .returns(Dir)
  type_parameters(:U).sig(
      arg0: Encoding,
      blk: T.proc(arg0: Dir).returns(T.type_parameter(:U)),
  )
  .returns(T.type_parameter(:U))
  def opendir(arg0=T.unsafe(nil), &blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def owned?(); end

  Sorbet.sig.returns(Pathname)
  def parent(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def pipe?(); end

  Sorbet.sig(
      length: Integer,
      offset: Integer,
      open_args: Integer,
  )
  .returns(String)
  def read(length=T.unsafe(nil), offset=T.unsafe(nil), open_args=T.unsafe(nil)); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def readable?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def readable_real?(); end

  Sorbet.sig(
      sep: String,
      limit: Integer,
      open_args: Integer,
  )
  .returns(T::Array[String])
  def readlines(sep=T.unsafe(nil), limit=T.unsafe(nil), open_args=T.unsafe(nil)); end

  Sorbet.sig.returns(String)
  def readlink(); end

  Sorbet.sig(
      p1: String,
  )
  .returns(String)
  def realdirpath(p1=T.unsafe(nil)); end

  Sorbet.sig(
      p1: String,
  )
  .returns(String)
  def realpath(p1=T.unsafe(nil)); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def relative?(); end

  Sorbet.sig(
      base_directory: T.any(String, Pathname),
  )
  .returns(Pathname)
  def relative_path_from(base_directory); end

  Sorbet.sig(
      p1: String,
  )
  .returns(Integer)
  def rename(p1); end

  Sorbet.sig.returns(Integer)
  def rmdir(); end

  Sorbet.sig.returns(Integer)
  def rmtree(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def root?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def setgid?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def setuid?(); end

  Sorbet.sig.returns(Integer)
  def size(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def size?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def socket?(); end

  Sorbet.sig.returns([Pathname, Pathname])
  def split(); end

  Sorbet.sig.returns(File::Stat)
  def stat(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def sticky?(); end

  Sorbet.sig(
      args: String,
  )
  .returns(Pathname)
  def sub(*args); end

  Sorbet.sig(
      p1: String,
  )
  .returns(Pathname)
  def sub_ext(p1); end

  Sorbet.sig(
      old: String,
  )
  .returns(Integer)
  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def symlink?(old=T.unsafe(nil)); end

  Sorbet.sig(
      mode: Integer,
      perm: Integer,
  )
  .returns(Integer)
  def sysopen(mode=T.unsafe(nil), perm=T.unsafe(nil)); end

  Sorbet.sig.returns(T.self_type)
  def taint(); end

  Sorbet.sig.returns(String)
  def to_path(); end

  Sorbet.sig(
      length: Integer,
  )
  .returns(Integer)
  def truncate(length); end

  Sorbet.sig.returns(Integer)
  def unlink(); end

  Sorbet.sig.returns(T.self_type)
  def untaint(); end

  Sorbet.sig(
      atime: Time,
      mtime: Time,
  )
  .returns(Integer)
  def utime(atime, mtime); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def world_readable?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def world_writable?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def writable?(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def writable_real?(); end

  Sorbet.sig(
      arg0: String,
      offset: Integer,
      open_args: Integer,
  )
  .returns(Integer)
  def write(arg0, offset=T.unsafe(nil), open_args=T.unsafe(nil)); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end

  Sorbet.sig.returns(Pathname)
  def self.pwd(); end

  Sorbet.sig(
      other: T.any(String, Pathname),
  )
  .returns(Pathname)
  def /(other); end

  Sorbet.sig.returns(String)
  def to_s(); end
end
