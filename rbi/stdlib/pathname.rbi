# typed: core
class Pathname < Object
  SAME_PATHS = T.let(T.unsafe(nil), Proc)
  SEPARATOR_LIST = T.let(T.unsafe(nil), String)
  SEPARATOR_PAT = T.let(T.unsafe(nil), Regexp)
  TO_PATH = T.let(T.unsafe(nil), Symbol)

  sig {returns(Pathname)}
  def self.getwd(); end

  sig do
    params(
        p1: String,
        p2: String,
    )
    .returns(T::Array[Pathname])
  end
  def self.glob(p1, p2=T.unsafe(nil)); end

  def initialize(p); end

  sig do
    params(
        other: T.any(String, Pathname),
    )
    .returns(Pathname)
  end
  def +(other); end

  sig do
    params(
        p1: BasicObject,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(p1); end

  sig do
    params(
        p1: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(p1); end

  sig do
    params(
        p1: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(p1); end

  sig {returns(T::Boolean)}
  def absolute?(); end

  sig do
    params(
        blk: T.proc.params(arg0: Pathname).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  def ascend(&blk); end

  sig {returns(Time)}
  def atime(); end

  sig do
    params(
        p1: String,
    )
    .returns(Pathname)
  end
  def basename(p1=T.unsafe(nil)); end

  sig do
    params(
        length: Integer,
        offset: Integer,
    )
    .returns(String)
  end
  def binread(length=T.unsafe(nil), offset=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
        offset: Integer,
    )
    .returns(Integer)
  end
  def binwrite(arg0, offset=T.unsafe(nil)); end

  sig {returns(Time)}
  def birthtime(); end

  sig {returns(T::Boolean)}
  def blockdev?(); end

  sig {returns(T::Boolean)}
  def chardev?(); end

  sig do
    params(
        with_directory: T::Boolean,
    )
    .returns(T::Array[Pathname])
  end
  def children(with_directory); end

  sig do
    params(
        mode: Integer,
    )
    .returns(Integer)
  end
  def chmod(mode); end

  sig do
    params(
        owner: Integer,
        group: Integer,
    )
    .returns(Integer)
  end
  def chown(owner, group); end

  sig do
    params(
        consider_symlink: T::Boolean,
    )
    .returns(T.untyped)
  end
  def cleanpath(consider_symlink=T.unsafe(nil)); end

  sig {returns(Time)}
  def ctime(); end

  sig {returns(T.untyped)}
  def delete(); end

  sig do
    params(
        blk: T.proc.params(arg0: Pathname).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  def descend(&blk); end

  sig {returns(T::Boolean)}
  def directory?(); end

  sig {returns(Pathname)}
  def dirname(); end

  sig do
    params(
        with_directory: T::Boolean,
        blk: T.proc.params(arg0: Pathname).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  def each_child(with_directory, &blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Pathname).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  def each_entry(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig {returns(Enumerator[String])}
  def each_filename(&blk); end

  sig do
    params(
        sep: String,
        limit: Integer,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(Enumerator[String])
  end
  def each_line(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  sig {returns(T::Array[Pathname])}
  def entries(); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  sig {returns(T::Boolean)}
  def executable?(); end

  sig {returns(T::Boolean)}
  def executable_real?(); end

  sig {returns(T::Boolean)}
  def exist?(); end

  sig do
    params(
        p1: T.any(String, Pathname),
    )
    .returns(Pathname)
  end
  def expand_path(p1=T.unsafe(nil)); end

  sig {returns(String)}
  def extname(); end

  sig {returns(T::Boolean)}
  def file?(); end

  sig do
    params(
        ignore_error: T::Boolean,
        blk: T.proc.params(arg0: Pathname).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig do
    params(
        ignore_error: T::Boolean,
    )
    .returns(Enumerator[Pathname])
  end
  def find(ignore_error, &blk); end

  sig do
    params(
        pattern: String,
        flags: Integer,
    )
    .returns(T::Boolean)
  end
  def fnmatch(pattern, flags=T.unsafe(nil)); end

  sig {returns(T.self_type)}
  def freeze(); end

  sig {returns(String)}
  def ftype(); end

  sig {returns(T::Boolean)}
  def grpowned?(); end

  sig do
    params(
        args: T.any(String, Pathname),
    )
    .returns(Pathname)
  end
  def join(*args); end

  sig do
    params(
        mode: Integer,
    )
    .returns(Integer)
  end
  def lchmod(mode); end

  sig do
    params(
        owner: Integer,
        group: Integer,
    )
    .returns(Integer)
  end
  def lchown(owner, group); end

  sig {returns(File::Stat)}
  def lstat(); end

  sig do
    params(
        old: String,
    )
    .returns(Integer)
  end
  def make_link(old); end

  sig do
    params(
        p1: String,
    )
    .returns(Integer)
  end
  def mkdir(p1); end

  sig {returns(T.untyped)}
  def mkpath(); end

  sig {returns(T::Boolean)}
  def mountpoint?(); end

  sig {returns(Time)}
  def mtime(); end

  sig do
    params(
        mode: String,
        perm: String,
        opt: Integer,
    )
    .returns(File)
  end
  sig do
    type_parameters(:T).params(
        mode: String,
        perm: String,
        opt: Integer,
        blk: T.proc.params(arg0: File).returns(T.type_parameter(:T)),
    )
    .returns(T.type_parameter(:T))
  end
  def open(mode=T.unsafe(nil), perm=T.unsafe(nil), opt=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: Encoding,
    )
    .returns(Dir)
  end
  sig do
    type_parameters(:U).params(
        arg0: Encoding,
        blk: T.proc.params(arg0: Dir).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def opendir(arg0=T.unsafe(nil), &blk); end

  sig {returns(T::Boolean)}
  def owned?(); end

  sig {returns(Pathname)}
  def parent(); end

  sig {returns(T::Boolean)}
  def pipe?(); end

  sig do
    params(
        length: Integer,
        offset: Integer,
        open_args: Integer,
    )
    .returns(String)
  end
  def read(length=T.unsafe(nil), offset=T.unsafe(nil), open_args=T.unsafe(nil)); end

  sig {returns(T::Boolean)}
  def readable?(); end

  sig {returns(T::Boolean)}
  def readable_real?(); end

  sig do
    params(
        sep: String,
        limit: Integer,
        open_args: Integer,
    )
    .returns(T::Array[String])
  end
  def readlines(sep=T.unsafe(nil), limit=T.unsafe(nil), open_args=T.unsafe(nil)); end

  sig {returns(String)}
  def readlink(); end

  sig do
    params(
        p1: String,
    )
    .returns(String)
  end
  def realdirpath(p1=T.unsafe(nil)); end

  sig do
    params(
        p1: String,
    )
    .returns(String)
  end
  def realpath(p1=T.unsafe(nil)); end

  sig {returns(T::Boolean)}
  def relative?(); end

  sig do
    params(
        base_directory: T.any(String, Pathname),
    )
    .returns(Pathname)
  end
  def relative_path_from(base_directory); end

  sig do
    params(
        p1: String,
    )
    .returns(Integer)
  end
  def rename(p1); end

  sig {returns(Integer)}
  def rmdir(); end

  sig {returns(Integer)}
  def rmtree(); end

  sig {returns(T::Boolean)}
  def root?(); end

  sig {returns(T::Boolean)}
  def setgid?(); end

  sig {returns(T::Boolean)}
  def setuid?(); end

  sig {returns(Integer)}
  def size(); end

  sig {returns(T::Boolean)}
  def size?(); end

  sig {returns(T::Boolean)}
  def socket?(); end

  sig {returns([Pathname, Pathname])}
  def split(); end

  sig {returns(File::Stat)}
  def stat(); end

  sig {returns(T::Boolean)}
  def sticky?(); end

  sig do
    params(
        args: String,
    )
    .returns(Pathname)
  end
  def sub(*args); end

  sig do
    params(
        p1: String,
    )
    .returns(Pathname)
  end
  def sub_ext(p1); end

  sig do
    params(
        old: String,
    )
    .returns(Integer)
  end
  sig {returns(T::Boolean)}
  def symlink?(old=T.unsafe(nil)); end

  sig do
    params(
        mode: Integer,
        perm: Integer,
    )
    .returns(Integer)
  end
  def sysopen(mode=T.unsafe(nil), perm=T.unsafe(nil)); end

  sig {returns(T.self_type)}
  def taint(); end

  sig {returns(String)}
  def to_path(); end

  sig do
    params(
        length: Integer,
    )
    .returns(Integer)
  end
  def truncate(length); end

  sig {returns(Integer)}
  def unlink(); end

  sig {returns(T.self_type)}
  def untaint(); end

  sig do
    params(
        atime: Time,
        mtime: Time,
    )
    .returns(Integer)
  end
  def utime(atime, mtime); end

  sig {returns(T::Boolean)}
  def world_readable?(); end

  sig {returns(T::Boolean)}
  def world_writable?(); end

  sig {returns(T::Boolean)}
  def writable?(); end

  sig {returns(T::Boolean)}
  def writable_real?(); end

  sig do
    params(
        arg0: String,
        offset: Integer,
        open_args: Integer,
    )
    .returns(Integer)
  end
  def write(arg0, offset=T.unsafe(nil), open_args=T.unsafe(nil)); end

  sig {returns(T::Boolean)}
  def zero?(); end

  sig {returns(Pathname)}
  def self.pwd(); end

  sig do
    params(
        other: T.any(String, Pathname),
    )
    .returns(Pathname)
  end
  def /(other); end

  sig {returns(String)}
  def to_s(); end
end
