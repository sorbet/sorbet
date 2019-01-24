# typed: true
class File < IO
  ALT_SEPARATOR = T.let(T.unsafe(nil), NilClass)
  APPEND = T.let(T.unsafe(nil), Integer)
  BINARY = T.let(T.unsafe(nil), Integer)
  CREAT = T.let(T.unsafe(nil), Integer)
  DIRECT = T.let(T.unsafe(nil), Integer)
  DSYNC = T.let(T.unsafe(nil), Integer)
  EXCL = T.let(T.unsafe(nil), Integer)
  FNM_CASEFOLD = T.let(T.unsafe(nil), Integer)
  FNM_DOTMATCH = T.let(T.unsafe(nil), Integer)
  FNM_EXTGLOB = T.let(T.unsafe(nil), Integer)
  FNM_NOESCAPE = T.let(T.unsafe(nil), Integer)
  FNM_PATHNAME = T.let(T.unsafe(nil), Integer)
  FNM_SHORTNAME = T.let(T.unsafe(nil), Integer)
  FNM_SYSCASE = T.let(T.unsafe(nil), Integer)
  LOCK_EX = T.let(T.unsafe(nil), Integer)
  LOCK_NB = T.let(T.unsafe(nil), Integer)
  LOCK_SH = T.let(T.unsafe(nil), Integer)
  LOCK_UN = T.let(T.unsafe(nil), Integer)
  NOATIME = T.let(T.unsafe(nil), Integer)
  NOCTTY = T.let(T.unsafe(nil), Integer)
  NOFOLLOW = T.let(T.unsafe(nil), Integer)
  NONBLOCK = T.let(T.unsafe(nil), Integer)
  NULL = T.let(T.unsafe(nil), String)
  PATH_SEPARATOR = T.let(T.unsafe(nil), String)
  RDONLY = T.let(T.unsafe(nil), Integer)
  RDWR = T.let(T.unsafe(nil), Integer)
  RSYNC = T.let(T.unsafe(nil), Integer)
  SEEK_CUR = T.let(T.unsafe(nil), Integer)
  SEEK_DATA = T.let(T.unsafe(nil), Integer)
  SEEK_END = T.let(T.unsafe(nil), Integer)
  SEEK_HOLE = T.let(T.unsafe(nil), Integer)
  SEEK_SET = T.let(T.unsafe(nil), Integer)
  SEPARATOR = T.let(T.unsafe(nil), String)
  SHARE_DELETE = T.let(T.unsafe(nil), Integer)
  SYNC = T.let(T.unsafe(nil), Integer)
  TMPFILE = T.let(T.unsafe(nil), Integer)
  TRUNC = T.let(T.unsafe(nil), Integer)
  WRONLY = T.let(T.unsafe(nil), Integer)

  extend T::Generic
  Elem = type_member(:out, fixed: String)

  sig do
    params(
        file: String,
        dir: String,
    )
    .returns(String)
  end
  def self.absolute_path(file, dir=T.unsafe(nil)); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.atime(file); end

  sig do
    params(
        file: String,
        suffix: String,
    )
    .returns(String)
  end
  def self.basename(file, suffix=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: String,
        arg1: Integer,
        arg2: Integer,
    )
    .returns(String)
  end
  def self.binread(arg0, arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.birthtime(file); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.blockdev?(file); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.chardev?(file); end

  sig do
    params(
        mode: Integer,
        files: String,
    )
    .returns(Integer)
  end
  def self.chmod(mode, *files); end

  sig do
    params(
        owner: Integer,
        group: Integer,
        files: String,
    )
    .returns(Integer)
  end
  def self.chown(owner, group, *files); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.ctime(file); end

  sig do
    params(
        files: String,
    )
    .returns(Integer)
  end
  def self.delete(*files); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.directory?(file); end

  sig do
    params(
        file: String,
    )
    .returns(String)
  end
  def self.dirname(file); end

  sig do
    params(
        file: String,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.executable?(file); end

  sig do
    params(
        file: String,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.executable_real?(file); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.exist?(file); end

  sig do
    params(
        file: BasicObject,
        dir: BasicObject,
    )
    .returns(String)
  end
  def self.expand_path(file, dir=T.unsafe(nil)); end

  sig do
    params(
        path: String,
    )
    .returns(String)
  end
  def self.extname(path); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.file?(file); end

  sig do
    params(
        pattern: String,
        path: String,
        flags: Integer,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.fnmatch(pattern, path, flags=T.unsafe(nil)); end

  sig do
    params(
        file: String,
    )
    .returns(String)
  end
  def self.ftype(file); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.grpowned?(file); end

  sig do
    params(
        file_1: T.any(String, IO),
        file_2: T.any(String, IO),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.identical?(file_1, file_2); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(String)
  end
  def self.join(*arg0); end

  sig do
    params(
        mode: Integer,
        files: String,
    )
    .returns(Integer)
  end
  def self.lchmod(mode, *files); end

  sig do
    params(
        owner: Integer,
        group: Integer,
        files: String,
    )
    .returns(Integer)
  end
  def self.lchown(owner, group, *files); end

  sig do
    params(
        old: String,
        new: String,
    )
    .returns(Integer)
  end
  def self.link(old, new); end

  sig do
    params(
        file: String,
    )
    .returns(File::Stat)
  end
  def self.lstat(file); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.mtime(file); end

  sig do
    params(
        file: String,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.owned?(file); end

  sig do
    params(
        path: String,
    )
    .returns(String)
  end
  def self.path(path); end

  sig do
    params(
        file: String,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.pipe?(file); end

  sig do
    params(
        file: String,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.readable?(file); end

  sig do
    params(
        file: String,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.readable_real?(file); end

  sig do
    params(
        link: String,
    )
    .returns(String)
  end
  def self.readlink(link); end

  sig do
    params(
        pathname: String,
        dir: String,
    )
    .returns(String)
  end
  def self.realdirpath(pathname, dir=T.unsafe(nil)); end

  sig do
    params(
        pathname: String,
        dir: String,
    )
    .returns(String)
  end
  def self.realpath(pathname, dir=T.unsafe(nil)); end

  sig do
    params(
        old: String,
        new: String,
    )
    .returns(Integer)
  end
  def self.rename(old, new); end

  sig do
    params(
        file: String,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.setgid?(file); end

  sig do
    params(
        file: String,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.setuid?(file); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(Integer)
  end
  def self.size(file); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(T.nilable(Integer))
  end
  def self.size?(file); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.socket?(file); end

  sig do
    params(
        file: String,
    )
    .returns([String, String])
  end
  def self.split(file); end

  sig do
    params(
        file: BasicObject,
    )
    .returns(File::Stat)
  end
  def self.stat(file); end

  sig do
    params(
        file: String,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.sticky?(file); end

  sig do
    params(
        old: String,
        new: String,
    )
    .returns(Integer)
  end
  def self.symlink(old, new); end

  sig do
    params(
        file: String,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.symlink?(file); end

  sig do
    params(
        file: String,
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.truncate(file, arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.umask(arg0=T.unsafe(nil)); end

  sig do
    params(
        atime: Time,
        mtime: Time,
        files: String,
    )
    .returns(Integer)
  end
  def self.utime(atime, mtime, *files); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(T.nilable(Integer))
  end
  def self.world_readable?(file); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(T.nilable(Integer))
  end
  def self.world_writable?(file); end

  sig do
    params(
        file: String,
    )
    .returns(T.nilable(Integer))
  end
  def self.writable?(file); end

  sig do
    params(
        file: String,
    )
    .returns(T.nilable(Integer))
  end
  def self.writable_real?(file); end

  sig do
    params(
        file: T.any(String, IO),
    )
    .returns(T.nilable(Integer))
  end
  def self.zero?(file); end

  sig {returns(Time)}
  def atime(); end

  sig {returns(Time)}
  def birthtime(); end

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

  sig {returns(Time)}
  def ctime(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T.any(Integer, TrueClass, FalseClass))
  end
  def flock(arg0); end

  sig do
    params(
        file: String,
        mode: String,
        perm: String,
        opt: Integer,
    )
    .void
  end
  def initialize(file, mode=T.unsafe(nil), perm=T.unsafe(nil), opt=T.unsafe(nil)); end

  sig {returns(File::Stat)}
  def lstat(); end

  sig {returns(Time)}
  def mtime(); end

  sig {returns(String)}
  def path(); end

  sig {returns(Integer)}
  def size(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def truncate(arg0); end

  sig do
    params(
        pattern: String,
        path: String,
        flags: Integer,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def self.fnmatch?(pattern, path, flags=T.unsafe(nil)); end

  sig do
    params(
        files: String,
    )
    .returns(Integer)
  end
  def self.unlink(*files); end

  sig {returns(String)}
  def to_path(); end
end

module File::Constants
  APPEND = T.let(T.unsafe(nil), Integer)
  BINARY = T.let(T.unsafe(nil), Integer)
  CREAT = T.let(T.unsafe(nil), Integer)
  DIRECT = T.let(T.unsafe(nil), Integer)
  DSYNC = T.let(T.unsafe(nil), Integer)
  EXCL = T.let(T.unsafe(nil), Integer)
  FNM_CASEFOLD = T.let(T.unsafe(nil), Integer)
  FNM_DOTMATCH = T.let(T.unsafe(nil), Integer)
  FNM_EXTGLOB = T.let(T.unsafe(nil), Integer)
  FNM_NOESCAPE = T.let(T.unsafe(nil), Integer)
  FNM_PATHNAME = T.let(T.unsafe(nil), Integer)
  FNM_SHORTNAME = T.let(T.unsafe(nil), Integer)
  FNM_SYSCASE = T.let(T.unsafe(nil), Integer)
  LOCK_EX = T.let(T.unsafe(nil), Integer)
  LOCK_NB = T.let(T.unsafe(nil), Integer)
  LOCK_SH = T.let(T.unsafe(nil), Integer)
  LOCK_UN = T.let(T.unsafe(nil), Integer)
  NOATIME = T.let(T.unsafe(nil), Integer)
  NOCTTY = T.let(T.unsafe(nil), Integer)
  NOFOLLOW = T.let(T.unsafe(nil), Integer)
  NONBLOCK = T.let(T.unsafe(nil), Integer)
  NULL = T.let(T.unsafe(nil), String)
  RDONLY = T.let(T.unsafe(nil), Integer)
  RDWR = T.let(T.unsafe(nil), Integer)
  RSYNC = T.let(T.unsafe(nil), Integer)
  SHARE_DELETE = T.let(T.unsafe(nil), Integer)
  SYNC = T.let(T.unsafe(nil), Integer)
  TMPFILE = T.let(T.unsafe(nil), Integer)
  TRUNC = T.let(T.unsafe(nil), Integer)
  WRONLY = T.let(T.unsafe(nil), Integer)
end

class File::Stat < Object
  include Comparable

  sig do
    params(
        other: File::Stat,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  sig {returns(Time)}
  def atime(); end

  sig {returns(Time)}
  def birthtime(); end

  sig {returns(T.nilable(Integer))}
  def blksize(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def blockdev?(); end

  sig {returns(T.nilable(Integer))}
  def blocks(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def chardev?(); end

  sig {returns(Time)}
  def ctime(); end

  sig {returns(Integer)}
  def dev(); end

  sig {returns(Integer)}
  def dev_major(); end

  sig {returns(Integer)}
  def dev_minor(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def directory?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def executable?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def executable_real?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def file?(); end

  sig {returns(String)}
  def ftype(); end

  sig {returns(Integer)}
  def gid(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def grpowned?(); end

  sig do
    params(
        file: String,
    )
    .returns(Object)
  end
  def initialize(file); end

  sig {returns(Integer)}
  def ino(); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(Integer)}
  def mode(); end

  sig {returns(Time)}
  def mtime(); end

  sig {returns(Integer)}
  def nlink(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def owned?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def pipe?(); end

  sig {returns(T.nilable(Integer))}
  def rdev(); end

  sig {returns(Integer)}
  def rdev_major(); end

  sig {returns(Integer)}
  def rdev_minor(); end

  sig do
    params(
        length: Integer,
        outbuf: String,
    )
    .returns(String)
  end
  def read(length=T.unsafe(nil), outbuf=T.unsafe(nil)); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def readable?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def readable_real?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def setgid?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def setuid?(); end

  sig {returns(Integer)}
  def size(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def socket?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def sticky?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def symlink?(); end

  sig {returns(Integer)}
  def uid(); end

  sig {returns(T.nilable(Integer))}
  def world_readable?(); end

  sig {returns(T.nilable(Integer))}
  def world_writable?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def writable?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def writable_real?(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def zero?(); end
end
