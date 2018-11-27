# typed: true

class Shell
  include(Shell::Error)
  extend(Exception2MessageMapper)

  class SystemCommand < ::Shell::Filter
    def start()
    end

    def input=(inp)
    end

    def active?()
    end

    def wait?()
    end

    def notify(*opts)
    end

    def start_export()
    end

    def kill(sig)
    end

    def name()
    end

    def start_import()
    end

    def flush()
    end

    def each(rs = _)
    end

    def command()
    end

    def super_each(rs = _)
    end

    def terminate()
    end
  end

  class Echo < ::Shell::BuiltInCommand
    def each(rs = _)
    end
  end

  class Cat < ::Shell::BuiltInCommand
    def each(rs = _)
    end
  end

  class Glob < ::Shell::BuiltInCommand
    def each(rs = _)
    end
  end

  class Void < ::Shell::BuiltInCommand
    def each(rs = _)
    end
  end

  module Error
    extend(Exception2MessageMapper)

    class CantDefine < ::StandardError

    end

    class CantApplyMethod < ::StandardError

    end

    class DirStackEmpty < ::StandardError

    end

    class CommandNotFound < ::StandardError

    end

    def Raise(err = _, *rest)
    end

    def Fail(err = _, *rest)
    end
  end

  class AppendIO < ::Shell::BuiltInCommand
    def input=(filter)
    end
  end

  class AppendFile < ::Shell::AppendIO
    def input=(filter)
    end
  end

  class Tee < ::Shell::BuiltInCommand
    def each(rs = _)
    end
  end

  class Filter
    include(Enumerable)

    def >>(to)
    end

    def test(*args, &block)
    end

    def notify(*args, &block)
    end

    def [](*args, &block)
    end

    def empty?(*args, &block)
    end

    def append(*args, &block)
    end

    def join(*args, &block)
    end

    def split(*args, &block)
    end

    def find_system_command(*args, &block)
    end

    def finish_all_jobs(*args, &block)
    end

    def inspect()
    end

    def +(filter)
    end

    def echo(*args, &block)
    end

    def transact(*args, &block)
    end

    def cat(*args, &block)
    end

    def concat(*args, &block)
    end

    def mkdir(*args, &block)
    end

    def rmdir(*args, &block)
    end

    def tee(*args, &block)
    end

    def glob(*args, &block)
    end

    def size(*args, &block)
    end

    def each(rs = _)
    end

    def <(src)
    end

    def directory?(*args, &block)
    end

    def exist?(*args, &block)
    end

    def >(to)
    end

    def readable?(*args, &block)
    end

    def rehash(*args, &block)
    end

    def input()
    end

    def world_readable?(*args, &block)
    end

    def input=(filter)
    end

    def out(*args, &block)
    end

    def writable?(*args, &block)
    end

    def executable?(*args, &block)
    end

    def delete(*args, &block)
    end

    def to_a()
    end

    def to_s()
    end

    def size?(*args, &block)
    end

    def executable_real?(*args, &block)
    end

    def writable_real?(*args, &block)
    end

    def world_writable?(*args, &block)
    end

    def readable_real?(*args, &block)
    end

    def grpowned?(*args, &block)
    end

    def file?(*args, &block)
    end

    def symlink?(*args, &block)
    end

    def owned?(*args, &block)
    end

    def blockdev?(*args, &block)
    end

    def pipe?(*args, &block)
    end

    def setuid?(*args, &block)
    end

    def socket?(*args, &block)
    end

    def lstat(*args, &block)
    end

    def chardev?(*args, &block)
    end

    def stat(*args, &block)
    end

    def mtime(*args, &block)
    end

    def ftype(*args, &block)
    end

    def open(*args, &block)
    end

    def utime(*args, &block)
    end

    def ctime(*args, &block)
    end

    def system(*args, &block)
    end

    def atime(*args, &block)
    end

    def chmod(*args, &block)
    end

    def chown(*args, &block)
    end

    def rm(*args, &block)
    end

    def symlink(*args, &block)
    end

    def readlink(*args, &block)
    end

    def link(*args, &block)
    end

    def rename(*args, &block)
    end

    def truncate(*args, &block)
    end

    def unlink(*args, &block)
    end

    def exists?(*args, &block)
    end

    def foreach(*args, &block)
    end

    def basename(*args, &block)
    end

    def check_point(*args, &block)
    end

    def zero?(*args, &block)
    end

    def dirname(*args, &block)
    end

    def setgid?(*args, &block)
    end

    def sticky?(*args, &block)
    end

    def identical?(*args, &block)
    end

    def |(filter)
    end
  end

  class Concat < ::Shell::BuiltInCommand
    def each(rs = _)
    end
  end

  class BuiltInCommand < ::Shell::Filter
    def wait?()
    end

    def active?()
    end
  end

  class CommandProcessor
    NoDelegateMethods = T.let(T.unsafe(nil), Array)

    def test(command, file1, file2 = _)
    end

    def notify(*opts)
    end

    def [](command, file1, file2 = _)
    end

    def empty?(filename)
    end

    def append(to, filter)
    end

    def join(*items)
    end

    def split(pathname)
    end

    def find_system_command(command)
    end

    def finish_all_jobs()
    end

    def transact(&block)
    end

    def echo(*strings)
    end

    def concat(*jobs)
    end

    def cat(*filenames)
    end

    def mkdir(*path)
    end

    def rmdir(*path)
    end

    def tee(file)
    end

    def glob(pattern)
    end

    def size(filename)
    end

    def directory?(filename)
    end

    def exist?(filename)
    end

    def readable?(filename)
    end

    def rehash()
    end

    def readable_real?(filename)
    end

    def world_readable?(filename)
    end

    def writable?(filename)
    end

    def out(dev = _, &block)
    end

    def world_writable?(filename)
    end

    def delete(*filenames)
    end

    def executable_real?(filename)
    end

    def executable?(filename)
    end

    def writable_real?(filename)
    end

    def file?(filename)
    end

    def size?(filename)
    end

    def owned?(filename)
    end

    def grpowned?(filename)
    end

    def pipe?(filename)
    end

    def symlink?(filename)
    end

    def socket?(filename)
    end

    def blockdev?(filename)
    end

    def chardev?(filename)
    end

    def setuid?(filename)
    end

    def stat(filename)
    end

    def lstat(filename)
    end

    def ftype(filename)
    end

    def open(path, mode = _, perm = _, &b)
    end

    def mtime(filename)
    end

    def ctime(filenames)
    end

    def system(command, *opts)
    end

    def utime(atime, mtime, *filenames)
    end

    def chmod(mode, *filenames)
    end

    def atime(filename)
    end

    def chown(owner, group, *filename)
    end

    def rm(*filenames)
    end

    def truncate(filename, length)
    end

    def link(filename_o, filename_n)
    end

    def symlink(filename_o, filename_n)
    end

    def readlink(filename)
    end

    def unlink(path)
    end

    def check_point()
    end

    def rename(filename_from, filename_to)
    end

    def sticky?(filename)
    end

    def expand_path(path)
    end

    def exists?(filename)
    end

    def foreach(path = _, *rs)
    end

    def basename(fn, *opts)
    end

    def dirname(filename)
    end

    def zero?(filename)
    end

    def setgid?(filename)
    end

    def identical?(filename)
    end
  end

  class ProcessController
    USING_AT_EXIT_WHEN_PROCESS_EXIT = T.let(T.unsafe(nil), TrueClass)

    def start_job(command = _)
    end

    def wait_all_jobs_execution()
    end

    def shell()
    end

    def add_schedule(command)
    end

    def jobs()
    end

    def waiting_job?(job)
    end

    def active_job?(job)
    end

    def kill_job(sig, command)
    end

    def sfork(command)
    end

    def active_jobs()
    end

    def waiting_jobs()
    end

    def jobs_exist?()
    end

    def active_jobs_exist?()
    end

    def waiting_jobs_exist?()
    end

    def terminate_job(command)
    end
  end

  def record_separator=(_)
  end

  def notify(*args, &block)
  end

  def append(*args, &block)
  end

  def join(*args, &block)
  end

  def verbose()
  end

  def directory?(*args, &block)
  end

  def exist?(*args, &block)
  end

  def exists?(*args, &block)
  end

  def readable?(*args, &block)
  end

  def rehash(*args, &block)
  end

  def readable_real?(*args, &block)
  end

  def world_readable?(*args, &block)
  end

  def writable?(*args, &block)
  end

  def world_writable?(*args, &block)
  end

  def executable?(*args, &block)
  end

  def executable_real?(*args, &block)
  end

  def file?(*args, &block)
  end

  def size?(*args, &block)
  end

  def writable_real?(*args, &block)
  end

  def owned?(*args, &block)
  end

  def debug?()
  end

  def verbose?()
  end

  def debug=(val)
  end

  def pipe?(*args, &block)
  end

  def symlink?(*args, &block)
  end

  def grpowned?(*args, &block)
  end

  def setuid?(*args, &block)
  end

  def chardev?(*args, &block)
  end

  def socket?(*args, &block)
  end

  def stat(*args, &block)
  end

  def lstat(*args, &block)
  end

  def ftype(*args, &block)
  end

  def atime(*args, &block)
  end

  def blockdev?(*args, &block)
  end

  def ctime(*args, &block)
  end

  def truncate(*args, &block)
  end

  def utime(*args, &block)
  end

  def chmod(*args, &block)
  end

  def system_path()
  end

  def system_path=(path)
  end

  def record_separator()
  end

  def command_processor()
  end

  def process_controller()
  end

  def cwd()
  end

  def dir_stack()
  end

  def link(*args, &block)
  end

  def symlink(*args, &block)
  end

  def umask()
  end

  def pushdir(path = _, verbose = _)
  end

  def Fail(err = _, *rest)
  end

  def expand_path(path)
  end

  def pushd(path = _, verbose = _)
  end

  def popd()
  end

  def jobs()
  end

  def popdir()
  end

  def unlink(*args, &block)
  end

  def rename(*args, &block)
  end

  def dirname(*args, &block)
  end

  def chown(*args, &block)
  end

  def check_point(*args, &block)
  end

  def mtime(*args, &block)
  end

  def setgid?(*args, &block)
  end

  def sticky?(*args, &block)
  end

  def zero?(*args, &block)
  end

  def readlink(*args, &block)
  end

  def Raise(err = _, *rest)
  end

  def identical?(*args, &block)
  end

  def basename(*args, &block)
  end

  def debug()
  end

  def test(*args, &block)
  end

  def [](*args, &block)
  end

  def kill(sig, command)
  end

  def empty?(*args, &block)
  end

  def split(*args, &block)
  end

  def verbose=(_)
  end

  def find_system_command(*args, &block)
  end

  def finish_all_jobs(*args, &block)
  end

  def inspect()
  end

  def transact(*args, &block)
  end

  def echo(*args, &block)
  end

  def chdir(path = _, verbose = _)
  end

  def getwd()
  end

  def pwd()
  end

  def cat(*args, &block)
  end

  def cd(path = _, verbose = _)
  end

  def mkdir(*args, &block)
  end

  def concat(*args, &block)
  end

  def tee(*args, &block)
  end

  def rmdir(*args, &block)
  end

  def size(*args, &block)
  end

  def dir()
  end

  def glob(*args, &block)
  end

  def out(*args, &block)
  end

  def delete(*args, &block)
  end

  def dirs()
  end

  def rm(*args, &block)
  end

  def open(*args, &block)
  end

  def system(*args, &block)
  end

  def foreach(*args, &block)
  end

  def umask=(_)
  end
end
