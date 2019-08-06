# typed: __STDLIB_INTERNAL

# https://github.com/ruby/fileutils
module FileUtils
  LOW_METHODS = T.let(T.unsafe(nil), Array)
  METHODS = T.let(T.unsafe(nil), Array)
  OPT_TABLE = T.let(T.unsafe(nil), Hash)
  VERSION = T.let(T.unsafe(nil), String)

  sig do
    params(
      src: T.any(String, Pathname),
      dest: T.any(String, Pathname),
      preserve: T.nilable(T::Hash[Symbol, T::Boolean]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      dereference_root: T::Boolean,
      remove_destination: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  def self.cp_r(src, dest, preserve: nil, noop: nil, verbose: nil, dereference_root: true, remove_destination: nil); end

  sig do
    params(
      list: T.any(String, T::Array[String]),
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      secure: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  def self.rm_r(list, force: nil, noop: nil, verbose: nil, secure: nil); end

  sig do
    params(
      list: T.any(String, Pathname),
      mode: T.nilable(T::Hash[Symbol, T::Boolean]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  def self.mkdir_p(list, mode: nil, noop: nil, verbose: nil); end

  # makedirs is an alias of mkdir_p
  sig do
    params(
      list: T.any(String, Pathname),
      mode: T.nilable(T::Hash[Symbol, T::Boolean]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  def self.makedirs(list, mode: nil, noop: nil, verbose: nil); end

  # mkpath is an alias of mkdir_p
  sig do
    params(
      list: T.any(String, Pathname),
      mode: T.nilable(T::Hash[Symbol, T::Boolean]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  def self.mkpath(list, mode: nil, noop: nil, verbose: nil); end

  sig do
    params(
      list: T.any(String, T::Array[String]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      mtime: T.nilable(Time),
      nocreate: T.nilable(T::Boolean),
    ).void
  end
  def self.touch(list, noop: nil, verbose: nil, mtime: nil, nocreate: nil); end

  sig {params(dir: T.untyped, verbose: T.nilable(T::Boolean), block: T.untyped).returns(T.untyped)}
  def self.cd(dir, verbose: nil, &block); end

  # chdir is an alias of cd
  sig {params(dir: T.untyped, verbose: T.nilable(T::Boolean), block: T.untyped).returns(T.untyped)}
  def self.chdir(dir, verbose: nil, &block); end

  sig do
    params(
      mode: T.untyped,
      list: T.untyped,
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.chmod(mode, list, noop: nil, verbose: nil); end

  sig do
    params(
      mode: T.untyped,
      list: T.untyped,
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      force: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.chmod_R(mode, list, noop: nil, verbose: nil, force: nil); end

  sig do
    params(
      user: T.untyped,
      group: T.untyped,
      list: T.untyped,
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.chown(user, group, list, noop: nil, verbose: nil); end

  sig do
    params(
      user: T.untyped,
      group: T.untyped,
      list: T.untyped,
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      force: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.chown_R(user, group, list, noop: nil, verbose: nil, force: nil); end

  sig {params(opt: T.untyped).returns(T::Array[String])}
  def self.collect_method(opt); end

  sig {returns(T.untyped)}
  def self.commands; end

  sig {params(a: T.untyped, b: T.untyped).returns(T::Boolean)}
  def self.compare_file(a, b); end

  # cmp is an alias of compare_file
  sig {params(a: T.untyped, b: T.untyped).returns(T::Boolean)}
  def self.cmp(a, b); end

  # identical? is an alias of compare_file
  sig {params(a: T.untyped, b: T.untyped).returns(T::Boolean)}
  def self.identical?(a, b); end

  sig {params(a: T.untyped, b: T.untyped).returns(T::Boolean)}
  def self.compare_stream(a, b); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.cp(src, dest, preserve: nil, noop: nil, verbose: nil); end

  # copy is an alias of cp
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.copy(src, dest, preserve: nil, noop: nil, verbose: nil); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T::Boolean,
      dereference_root: T::Boolean,
      remove_destination: T::Boolean
    ).returns(T.untyped)
  end
  def self.copy_entry(src, dest, preserve = false, dereference_root = false, remove_destination = false); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T::Boolean,
      dereference: T::Boolean
    ).returns(T.untyped)
  end
  def self.copy_file(src, dest, preserve = false, dereference = true); end

  sig {params(src: T.untyped, dest: T.untyped).returns(T.untyped)}
  def self.copy_stream(src, dest); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      dereference_root: T::Boolean,
      remove_destination: T::Boolean
    ).returns(T.untyped)
  end
  def self.cp_lr(src, dest, noop: nil, verbose: nil, dereference_root: true, remove_destination: false); end

  sig {params(mid: T.untyped, opt: T.untyped).returns(T::Boolean)}
  def self.have_option?(mid, opt); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      mode: T.untyped,
      owner: T.untyped,
      group: T.untyped,
      preserve: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.install(src, dest, mode: nil, owner: nil, group: nil, preserve: nil, noop: nil, verbose: nil); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.ln(src, dest, force: nil, noop: nil, verbose: nil); end

  # link is an alias of ln
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.link(src, dest, force: nil, noop: nil, verbose: nil); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      dereference_root: T::Boolean,
      remove_destination: T::Boolean
    ).returns(T.untyped)
  end
  def self.link_entry(src, dest, dereference_root = false, remove_destination = false); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.ln_s(src, dest, force: nil, noop: nil, verbose: nil); end

  # symlink is an alias of ln_s
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.symlink(src, dest, force: nil, noop: nil, verbose: nil); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.ln_sf(src, dest, noop: nil, verbose: nil); end

  sig do
    params(
      list: T.untyped,
      mode: T.untyped,
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.mkdir(list, mode: nil, noop: nil, verbose: nil); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      secure: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.mv(src, dest, force: nil, noop: nil, verbose: nil, secure: nil); end

  # move is an alias of mv.
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      secure: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.move(src, dest, force: nil, noop: nil, verbose: nil, secure: nil); end

  sig {returns(T::Array[String])}
  def self.options; end

  sig {params(mid: T.untyped).returns(T::Array[String])}
  def self.options_of(mid); end

  sig {params(name: T.untyped).returns(T.untyped)}
  def self.private_module_function(name); end

  sig {returns(T.untyped)}
  def self.pwd; end

  # getwd is an alias of pwd
  sig {returns(T.untyped)}
  def self.getwd; end

  sig {params(path: T.untyped, force: T::Boolean).returns(T.untyped)}
  def self.remove_dir(path, force = false); end

  sig {params(path: T.untyped, force: T::Boolean).returns(T.untyped)}
  def self.remove_entry(path, force = false); end

  sig {params(path: T.untyped, force: T::Boolean).returns(T.untyped)}
  def self.remove_entry_secure(path, force = false); end

  sig {params(path: T.untyped, force: T::Boolean).returns(T.untyped)}
  def self.remove_file(path, force = false); end

  sig do
    params(
      list: T.untyped,
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.rm(list, force: nil, noop: nil, verbose: nil); end

  # remove is an alias of rm
  sig do
    params(
      list: T.untyped,
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.remove(list, force: nil, noop: nil, verbose: nil); end

  sig {params(list: T.untyped, noop: T.nilable(T::Boolean), verbose: T.nilable(T::Boolean)).returns(T.untyped)}
  def self.rm_f(list, noop: nil, verbose: nil); end

  # safe_unlink is an alias of rm_f
  sig {params(list: T.untyped, noop: T.nilable(T::Boolean), verbose: T.nilable(T::Boolean)).returns(T.untyped)}
  def self.safe_unlink(list, noop: nil, verbose: nil); end

  sig do
    params(
      list: T.untyped,
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      secure: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.rm_rf(list, noop: nil, verbose: nil, secure: nil); end

  # rmtree is an alias of rm_rf
  sig do
    params(
      list: T.untyped,
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      secure: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.rmtree(list, noop: nil, verbose: nil, secure: nil); end

  sig do
    params(
      list: T.untyped,
      parents: T.untyped,
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.untyped)
  end
  def self.rmdir(list, parents: nil, noop: nil, verbose: nil); end

  sig {params(new: T.untyped, old_list: T.untyped).returns(T::Boolean)}
  def self.uptodate?(new, old_list); end
end

module FileUtils::DryRun
  LOW_METHODS = T.let(T.unsafe(nil), Array)
  METHODS = T.let(T.unsafe(nil), Array)
  OPT_TABLE = T.let(T.unsafe(nil), Hash)
end

class FileUtils::Entry_ < Object
  include FileUtils::StreamUtils_

  DIRECTORY_TERM = T.let(T.unsafe(nil), String)
  SYSCASE = T.let(T.unsafe(nil), String)
  S_IF_DOOR = T.let(T.unsafe(nil), Integer)

  sig {returns(T::Boolean)}
  def blockdev?; end

  sig {returns(T::Boolean)}
  def chardev?; end

  sig {params(mode: T.untyped).returns(T.untyped)}
  def chmod(mode); end

  sig {params(uid: T.untyped, gid: T.untyped).returns(T.untyped)}
  def chown(uid, gid); end

  sig {params(dest: T.untyped).returns(T.untyped)}
  def copy(dest); end

  sig {params(dest: T.untyped).returns(T.untyped)}
  def copy_file(dest); end

  sig {params(path: T.untyped).returns(T.untyped)}
  def copy_metadata(path); end

  sig {returns(T::Boolean)}
  def dereference?; end

  sig {returns(T::Boolean)}
  def directory?; end

  sig {returns(T::Boolean)}
  def door?; end

  sig {returns(T.untyped)}
  def entries; end

  sig {returns(T::Boolean)}
  def exist?; end

  sig {returns(T::Boolean)}
  def file?; end

  sig {params(a: T.untyped, b: T.untyped, deref: T::Boolean).returns(T.untyped)}
  def initialize(a, b = nil, deref = false); end

  sig {params(dest: T.untyped).returns(T.untyped)}
  def link(dest); end

  sig {returns(T.untyped)}
  def lstat; end

  sig {returns(T.untyped)}
  def lstat!; end

  sig {returns(T.untyped)}
  def path; end

  sig {returns(T::Boolean)}
  def pipe?; end

  sig {returns(T.untyped)}
  def platform_support; end

  sig {returns(T.untyped)}
  def postorder_traverse; end

  sig {returns(T.untyped)}
  def prefix; end

  sig {returns(T.untyped)}
  def preorder_traverse; end

  sig {returns(T.untyped)}
  def rel; end

  sig {returns(T.untyped)}
  def remove; end

  sig {returns(T.untyped)}
  def remove_dir1; end

  sig {returns(T.untyped)}
  def remove_file; end

  sig {returns(T::Boolean)}
  def socket?; end

  sig {returns(T.untyped)}
  def stat; end

  sig {returns(T.untyped)}
  def stat!; end

  sig {returns(T::Boolean)}
  def symlink?; end

  sig {returns(T.untyped)}
  def traverse; end

  sig {params(pre: T.untyped, post: T.untyped).returns(T.untyped)}
  def wrap_traverse(pre, post); end
end

module FileUtils::LowMethods
end

module FileUtils::NoWrite
  LOW_METHODS = T.let(T.unsafe(nil), Array)
  METHODS = T.let(T.unsafe(nil), Array)
  OPT_TABLE = T.let(T.unsafe(nil), Hash)
end

module FileUtils::StreamUtils_
end

module FileUtils::Verbose
  LOW_METHODS = T.let(T.unsafe(nil), Array)
  METHODS = T.let(T.unsafe(nil), Array)
  OPT_TABLE = T.let(T.unsafe(nil), Hash)
end
