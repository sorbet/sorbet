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

  sig(
      file: String,
      dir: String,
  )
  .returns(String)
  def self.absolute_path(file, dir=_); end

  sig(
      file: BasicObject,
  )
  .returns(Time)
  def self.atime(file); end

  sig(
      file: String,
      suffix: String,
  )
  .returns(String)
  def self.basename(file, suffix=_); end

  sig(
      arg0: String,
  )
  .returns(String)
  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  sig(
      arg0: String,
      arg1: Integer,
      arg2: Integer,
  )
  .returns(String)
  def self.binread(arg0, arg1=_, arg2=_); end

  sig(
      file: BasicObject,
  )
  .returns(Time)
  def self.birthtime(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.blockdev?(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.chardev?(file); end

  sig(
      mode: Integer,
      files: String,
  )
  .returns(Integer)
  def self.chmod(mode, *files); end

  sig(
      owner: Integer,
      group: Integer,
      files: String,
  )
  .returns(Integer)
  def self.chown(owner, group, *files); end

  sig(
      file: BasicObject,
  )
  .returns(Time)
  def self.ctime(file); end

  sig(
      files: String,
  )
  .returns(Integer)
  def self.delete(*files); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.directory?(file); end

  sig(
      file: String,
  )
  .returns(String)
  def self.dirname(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.executable?(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.executable_real?(file); end

  sig(
      file: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.exist?(file); end

  sig(
      file: BasicObject,
      dir: BasicObject,
  )
  .returns(String)
  def self.expand_path(file, dir=_); end

  sig(
      path: String,
  )
  .returns(String)
  def self.extname(path); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.file?(file); end

  sig(
      pattern: String,
      path: String,
      flags: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.fnmatch(pattern, path, flags=_); end

  sig(
      file: String,
  )
  .returns(String)
  def self.ftype(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.grpowned?(file); end

  sig(
      file_1: T.any(String, IO),
      file_2: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.identical?(file_1, file_2); end

  sig(
      arg0: BasicObject,
  )
  .returns(String)
  def self.join(*arg0); end

  sig(
      mode: Integer,
      files: String,
  )
  .returns(Integer)
  def self.lchmod(mode, *files); end

  sig(
      owner: Integer,
      group: Integer,
      files: String,
  )
  .returns(Integer)
  def self.lchown(owner, group, *files); end

  sig(
      old: String,
      new: String,
  )
  .returns(Integer)
  def self.link(old, new); end

  sig(
      file: String,
  )
  .returns(File::Stat)
  def self.lstat(file); end

  sig(
      file: BasicObject,
  )
  .returns(Time)
  def self.mtime(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.owned?(file); end

  sig(
      path: String,
  )
  .returns(String)
  def self.path(path); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.pipe?(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.readable?(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.readable_real?(file); end

  sig(
      link: String,
  )
  .returns(String)
  def self.readlink(link); end

  sig(
      pathname: String,
      dir: String,
  )
  .returns(String)
  def self.realdirpath(pathname, dir=_); end

  sig(
      pathname: String,
      dir: String,
  )
  .returns(String)
  def self.realpath(pathname, dir=_); end

  sig(
      old: String,
      new: String,
  )
  .returns(Integer)
  def self.rename(old, new); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.setgid?(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.setuid?(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(Integer)
  def self.size(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.nilable(Integer))
  def self.size?(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.socket?(file); end

  sig(
      file: String,
  )
  .returns([String, String])
  def self.split(file); end

  sig(
      file: BasicObject,
  )
  .returns(File::Stat)
  def self.stat(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.sticky?(file); end

  sig(
      old: String,
      new: String,
  )
  .returns(Integer)
  def self.symlink(old, new); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.symlink?(file); end

  sig(
      file: String,
      arg0: Integer,
  )
  .returns(Integer)
  def self.truncate(file, arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.umask(arg0=_); end

  sig(
      atime: Time,
      mtime: Time,
      files: String,
  )
  .returns(Integer)
  def self.utime(atime, mtime, *files); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.nilable(Integer))
  def self.world_readable?(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.nilable(Integer))
  def self.world_writable?(file); end

  sig(
      file: String,
  )
  .returns(T.nilable(Integer))
  def self.writable?(file); end

  sig(
      file: String,
  )
  .returns(T.nilable(Integer))
  def self.writable_real?(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.nilable(Integer))
  def self.zero?(file); end

  sig.returns(Time)
  def atime(); end

  sig.returns(Time)
  def birthtime(); end

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

  sig.returns(Time)
  def ctime(); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(Integer, TrueClass, FalseClass))
  def flock(arg0); end

  sig(
      file: String,
      mode: String,
      perm: String,
      opt: Integer,
  )
  .void
  def initialize(file, mode=_, perm=_, opt=_); end

  sig.returns(File::Stat)
  def lstat(); end

  sig.returns(Time)
  def mtime(); end

  sig.returns(String)
  def path(); end

  sig.returns(Integer)
  def size(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def truncate(arg0); end

  sig(
      pattern: String,
      path: String,
      flags: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.fnmatch?(pattern, path, flags=_); end

  sig(
      files: String,
  )
  .returns(Integer)
  def self.unlink(*files); end

  sig.returns(String)
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

  sig(
      other: File::Stat,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig.returns(Time)
  def atime(); end

  sig.returns(Time)
  def birthtime(); end

  sig.returns(T.nilable(Integer))
  def blksize(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def blockdev?(); end

  sig.returns(T.nilable(Integer))
  def blocks(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def chardev?(); end

  sig.returns(Time)
  def ctime(); end

  sig.returns(Integer)
  def dev(); end

  sig.returns(Integer)
  def dev_major(); end

  sig.returns(Integer)
  def dev_minor(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def directory?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def executable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def executable_real?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def file?(); end

  sig.returns(String)
  def ftype(); end

  sig.returns(Integer)
  def gid(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def grpowned?(); end

  sig(
      file: String,
  )
  .returns(Object)
  def initialize(file); end

  sig.returns(Integer)
  def ino(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Integer)
  def mode(); end

  sig.returns(Time)
  def mtime(); end

  sig.returns(Integer)
  def nlink(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def owned?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def pipe?(); end

  sig.returns(T.nilable(Integer))
  def rdev(); end

  sig.returns(Integer)
  def rdev_major(); end

  sig.returns(Integer)
  def rdev_minor(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def readable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def readable_real?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def setgid?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def setuid?(); end

  sig.returns(Integer)
  def size(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def socket?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def sticky?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def symlink?(); end

  sig.returns(Integer)
  def uid(); end

  sig.returns(T.nilable(Integer))
  def world_readable?(); end

  sig.returns(T.nilable(Integer))
  def world_writable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def writable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def writable_real?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end
