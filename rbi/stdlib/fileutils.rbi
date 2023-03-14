# typed: __STDLIB_INTERNAL

### https://github.com/ruby/fileutils
# # fileutils.rb
#
# Copyright (c) 2000-2007 Minero Aoki
#
# This program is free software. You can distribute/modify this program under
# the same terms of ruby.
#
# ## module [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html)
#
# Namespace for several file utility methods for copying, moving, removing, etc.
#
# ### [`Module`](https://docs.ruby-lang.org/en/2.6.0/Module.html) Functions
#
# ```ruby
# require 'fileutils'
#
# FileUtils.cd(dir, options)
# FileUtils.cd(dir, options) {|dir| block }
# FileUtils.pwd()
# FileUtils.mkdir(dir, options)
# FileUtils.mkdir(list, options)
# FileUtils.mkdir_p(dir, options)
# FileUtils.mkdir_p(list, options)
# FileUtils.rmdir(dir, options)
# FileUtils.rmdir(list, options)
# FileUtils.ln(target, link, options)
# FileUtils.ln(targets, dir, options)
# FileUtils.ln_s(target, link, options)
# FileUtils.ln_s(targets, dir, options)
# FileUtils.ln_sf(target, link, options)
# FileUtils.cp(src, dest, options)
# FileUtils.cp(list, dir, options)
# FileUtils.cp_r(src, dest, options)
# FileUtils.cp_r(list, dir, options)
# FileUtils.mv(src, dest, options)
# FileUtils.mv(list, dir, options)
# FileUtils.rm(list, options)
# FileUtils.rm_r(list, options)
# FileUtils.rm_rf(list, options)
# FileUtils.install(src, dest, options)
# FileUtils.chmod(mode, list, options)
# FileUtils.chmod_R(mode, list, options)
# FileUtils.chown(user, group, list, options)
# FileUtils.chown_R(user, group, list, options)
# FileUtils.touch(list, options)
# ```
#
# The `options` parameter is a hash of options, taken from the list `:force`,
# `:noop`, `:preserve`, and `:verbose`. `:noop` means that no changes are made.
# The other three are obvious. Each method documents the options that it
# honours.
#
# All methods that have the concept of a "source" file or directory can take
# either one file or a list of files in that argument. See the method
# documentation for examples.
#
# There are some 'low level' methods, which do not accept any option:
#
# ```ruby
# FileUtils.copy_entry(src, dest, preserve = false, dereference_root = false, remove_destination = false)
# FileUtils.copy_file(src, dest, preserve = false, dereference = true)
# FileUtils.copy_stream(srcstream, deststream)
# FileUtils.remove_entry(path, force = false)
# FileUtils.remove_entry_secure(path, force = false)
# FileUtils.remove_file(path, force = false)
# FileUtils.compare_file(path_a, path_b)
# FileUtils.compare_stream(stream_a, stream_b)
# FileUtils.uptodate?(file, cmp_list)
# ```
#
# ## module [`FileUtils::Verbose`](https://docs.ruby-lang.org/en/2.6.0/FileUtils/Verbose.html)
#
# This module has all methods of
# [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html) module, but
# it outputs messages before acting. This equates to passing the `:verbose` flag
# to methods in
# [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html).
#
# ## module [`FileUtils::NoWrite`](https://docs.ruby-lang.org/en/2.6.0/FileUtils/NoWrite.html)
#
# This module has all methods of
# [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html) module, but
# never changes files/directories.  This equates to passing the `:noop` flag to
# methods in [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html).
#
# ## module [`FileUtils::DryRun`](https://docs.ruby-lang.org/en/2.6.0/FileUtils/DryRun.html)
#
# This module has all methods of
# [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html) module, but
# never changes files/directories.  This equates to passing the `:noop` and
# `:verbose` flags to methods in
# [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html).
module FileUtils
  LOW_METHODS = T.let(T.unsafe(nil), T::Array[Symbol])
  METHODS = T.let(T.unsafe(nil), T::Array[Symbol])
  OPT_TABLE = T.let(T.unsafe(nil), T::Hash[String, T::Array[Symbol]])
  VERSION = T.let(T.unsafe(nil), String)

  # Copies `src` to `dest`. If `src` is a directory, this method copies all its
  # contents recursively. If `dest` is a directory, copies `src` to `dest/src`.
  #
  # `src` can be a list of files.
  #
  # ```ruby
  # # Installing Ruby library "mylib" under the site_ruby
  # FileUtils.rm_r site_ruby + '/mylib', :force
  # FileUtils.cp_r 'lib/', site_ruby + '/mylib'
  #
  # # Examples of copying several files to target directory.
  # FileUtils.cp_r %w(mail.rb field.rb debug/), site_ruby + '/tmail'
  # FileUtils.cp_r Dir.glob('*.rb'), '/home/foo/lib/ruby', :noop => true, :verbose => true
  #
  # # If you want to copy all contents of a directory instead of the
  # # directory itself, c.f. src/x -> dest/x, src/y -> dest/y,
  # # use following code.
  # FileUtils.cp_r 'src/.', 'dest'     # cp_r('src', 'dest') makes dest/src,
  #                                    # but this doesn't.
  # ```
  sig do
    params(
      src: T.any(String, Pathname),
      dest: T.any(String, Pathname),
      preserve: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      dereference_root: T::Boolean,
      remove_destination: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  module_function def cp_r(src, dest, preserve: nil, noop: nil, verbose: nil, dereference_root: true, remove_destination: nil); end

  # remove files `list[0]` `list[1]`... If `list[n]` is a directory, removes its
  # all contents recursively. This method ignores
  # [`StandardError`](https://docs.ruby-lang.org/en/2.6.0/StandardError.html)
  # when :force option is set.
  #
  # ```ruby
  # FileUtils.rm_r Dir.glob('/tmp/*')
  # FileUtils.rm_r 'some_dir', :force => true
  # ```
  #
  # WARNING: This method causes local vulnerability if one of parent directories
  # or removing directory tree are world writable (including /tmp, whose
  # permission is 1777), and the current process has strong privilege such as
  # Unix super user (root), and the system has symbolic link. For secure
  # removing, read the documentation of
  # [`remove_entry_secure`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-remove_entry_secure)
  # carefully, and set :secure option to true. Default is :secure=>false.
  #
  # NOTE: This method calls
  # [`remove_entry_secure`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-remove_entry_secure)
  # if :secure option is set. See also
  # [`remove_entry_secure`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-remove_entry_secure).
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      secure: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  module_function def rm_r(list, force: nil, noop: nil, verbose: nil, secure: nil); end

  # Creates a directory and all its parent directories. For example,
  #
  # ```ruby
  # FileUtils.mkdir_p '/usr/local/lib/ruby'
  # ```
  #
  # causes to make following directories, if it does not exist.
  #
  # *   /usr
  # *   /usr/local
  # *   /usr/local/lib
  # *   /usr/local/lib/ruby
  #
  #
  # You can pass several directories at a time in a list.
  #
  # Also aliased as:
  # [`mkpath`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-mkpath),
  # [`makedirs`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-makedirs)
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      mode: T.nilable(T.any(String, Integer)),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  module_function def mkdir_p(list, mode: nil, noop: nil, verbose: nil); end

  # Alias for:
  # [`mkdir_p`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-mkdir_p)
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      mode: T.nilable(T.any(String, Integer)),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  module_function def makedirs(list, mode: nil, noop: nil, verbose: nil); end

  # Alias for:
  # [`mkdir_p`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-mkdir_p)
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      mode: T.nilable(T.any(String, Integer)),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  module_function def mkpath(list, mode: nil, noop: nil, verbose: nil); end

  # Updates modification time (mtime) and access time (atime) of file(s) in
  # `list`. Files are created if they don't exist.
  #
  # ```ruby
  # FileUtils.touch 'timestamp'
  # FileUtils.touch Dir.glob('*.c');  system 'make'
  # ```
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      mtime: T.nilable(Time),
      nocreate: T.nilable(T::Boolean),
    ).void
  end
  module_function def touch(list, noop: nil, verbose: nil, mtime: nil, nocreate: nil); end

  # Changes the current directory to the directory `dir`.
  #
  # If this method is called with block, resumes to the old working directory
  # after the block execution finished.
  #
  # ```ruby
  # FileUtils.cd('/', :verbose => true)   # chdir and report it
  #
  # FileUtils.cd('/') do  # chdir
  #   # ...               # do something
  # end                   # return to original directory
  # ```
  #
  #
  # Also aliased as:
  # [`chdir`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-chdir)
  sig {
    type_parameters(:U)
    .params(
      dir: T.any(String, Pathname),
      verbose: T.nilable(T::Boolean),
      block: T.proc.returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  }
  module_function def cd(dir, verbose: nil, &block); end

  # Alias for:
  # [`cd`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-cd)
  sig {
    type_parameters(:U)
    .params(
      dir: T.any(String, Pathname),
      verbose: T.nilable(T::Boolean),
      block: T.proc.returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  }
  module_function def chdir(dir, verbose: nil, &block); end

  # Changes permission bits on the named files (in `list`) to the bit pattern
  # represented by `mode`.
  #
  # `mode` is the symbolic and absolute mode can be used.
  #
  # Absolute mode is
  #
  # ```ruby
  # FileUtils.chmod 0755, 'somecommand'
  # FileUtils.chmod 0644, %w(my.rb your.rb his.rb her.rb)
  # FileUtils.chmod 0755, '/usr/bin/ruby', :verbose => true
  # ```
  #
  # Symbolic mode is
  #
  # ```ruby
  # FileUtils.chmod "u=wrx,go=rx", 'somecommand'
  # FileUtils.chmod "u=wr,go=rr", %w(my.rb your.rb his.rb her.rb)
  # FileUtils.chmod "u=wrx,go=rx", '/usr/bin/ruby', :verbose => true
  # ```
  #
  # "a"
  # :   is user, group, other mask.
  # "u"
  # :   is user's mask.
  # "g"
  # :   is group's mask.
  # "o"
  # :   is other's mask.
  # "w"
  # :   is write permission.
  # "r"
  # :   is read permission.
  # "x"
  # :   is execute permission.
  # "X"
  # :   is execute permission for directories only, must be used in conjunction
  #     with "+"
  # "s"
  # :   is uid, gid.
  # "t"
  # :   is sticky bit.
  # "+"
  # :   is added to a class given the specified mode.
  # "-"
  # :   Is removed from a given class given mode.
  # "="
  # :   Is the exact nature of the class will be given a specified mode.
  sig do
    params(
      mode: T.nilable(T.any(String, Integer)),
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.nilable(Integer))
  end
  module_function def chmod(mode, list, noop: nil, verbose: nil); end

  # Changes permission bits on the named files (in `list`) to the bit pattern
  # represented by `mode`.
  #
  # ```ruby
  # FileUtils.chmod_R 0700, "/tmp/app.#{$$}"
  # FileUtils.chmod_R "u=wrx", "/tmp/app.#{$$}"
  # ```
  sig do
    params(
      mode: T.nilable(T.any(String, Integer)),
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      force: T.nilable(T::Boolean)
    ).void
  end
  module_function def chmod_R(mode, list, noop: nil, verbose: nil, force: nil); end

  # Changes owner and group on the named files (in `list`) to the user `user`
  # and the group `group`. `user` and `group` may be an ID (Integer/String) or a
  # name (String). If `user` or `group` is nil, this method does not change the
  # attribute.
  #
  # ```ruby
  # FileUtils.chown 'root', 'staff', '/usr/local/bin/ruby'
  # FileUtils.chown nil, 'bin', Dir.glob('/usr/bin/*'), :verbose => true
  # ```
  sig do
    params(
      user: T.nilable(T.any(Integer, String)),
      group: T.nilable(T.any(Integer, String)),
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T.nilable(Integer))
  end
  module_function def chown(user, group, list, noop: nil, verbose: nil); end

  # Changes owner and group on the named files (in `list`) to the user `user`
  # and the group `group` recursively. `user` and `group` may be an ID
  # (Integer/String) or a name (String). If `user` or `group` is nil, this
  # method does not change the attribute.
  #
  # ```ruby
  # FileUtils.chown_R 'www', 'www', '/var/www/htdocs'
  # FileUtils.chown_R 'cvs', 'cvs', '/var/cvs', :verbose => true
  # ```
  sig do
    params(
      user: T.nilable(T.any(Integer, String)),
      group: T.nilable(T.any(Integer, String)),
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      force: T.nilable(T::Boolean)
    ).void
  end
  module_function def chown_R(user, group, list, noop: nil, verbose: nil, force: nil); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # method names which have the option `opt`.
  #
  # ```ruby
  # p FileUtils.collect_method(:preserve) #=> ["cp", "cp_r", "copy", "install"]
  # ```
  sig {params(opt: Symbol).returns(T::Array[String])}
  def self.collect_method(opt); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # method names which have any options.
  #
  # ```ruby
  # p FileUtils.commands  #=> ["chmod", "cp", "cp_r", "install", ...]
  # ```
  sig {returns(T::Array[String])}
  def self.commands; end

  # Returns true if the contents of a file `a` and a file `b` are identical.
  #
  # ```ruby
  # FileUtils.compare_file('somefile', 'somefile')       #=> true
  # FileUtils.compare_file('/dev/null', '/dev/urandom')  #=> false
  # ```
  #
  #
  # Also aliased as:
  # [`identical?`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-identical-3F),
  # [`cmp`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-cmp)
  sig {params(a: T.any(String, Pathname), b: T.any(String, Pathname)).returns(T::Boolean)}
  module_function def compare_file(a, b); end

  # Alias for:
  # [`compare_file`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-compare_file)
  sig {params(a: T.any(String, Pathname), b: T.any(String, Pathname)).returns(T::Boolean)}
  module_function def cmp(a, b); end

  # Alias for:
  # [`compare_file`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-compare_file)
  sig {params(a: T.any(String, Pathname), b: T.any(String, Pathname)).returns(T::Boolean)}
  module_function def identical?(a, b); end

  # Returns true if the contents of a stream `a` and `b` are identical.
  sig {params(a: IO, b: IO).returns(T::Boolean)}
  module_function def compare_stream(a, b); end

  # Copies a file content `src` to `dest`. If `dest` is a directory, copies
  # `src` to `dest/src`.
  #
  # If `src` is a list of files, then `dest` must be a directory.
  #
  # ```ruby
  # FileUtils.cp 'eval.c', 'eval.c.org'
  # FileUtils.cp %w(cgi.rb complex.rb date.rb), '/usr/lib/ruby/1.6'
  # FileUtils.cp %w(cgi.rb complex.rb date.rb), '/usr/lib/ruby/1.6', :verbose => true
  # FileUtils.cp 'symlink', 'dest'   # copy content, "dest" is not a symlink
  # ```
  #
  #
  # Also aliased as:
  # [`copy`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-copy)
  sig do
    params(
      src: T.any(File, String, Pathname, T::Array[T.any(File, String, Pathname)]),
      dest: T.any(String, Pathname),
      preserve: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def cp(src, dest, preserve: nil, noop: nil, verbose: nil); end

  # Alias for:
  # [`cp`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-cp)
  sig do
    params(
      src: T.any(File, String, Pathname, T::Array[T.any(File, String, Pathname)]),
      dest: T.any(String, Pathname),
      preserve: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def copy(src, dest, preserve: nil, noop: nil, verbose: nil); end

  # Copies a file system entry `src` to `dest`. If `src` is a directory, this
  # method copies its contents recursively. This method preserves file types,
  # c.f. symlink, directory... (FIFO, device files and etc. are not supported
  # yet)
  #
  # Both of `src` and `dest` must be a path name. `src` must exist, `dest` must
  # not exist.
  #
  # If `preserve` is true, this method preserves owner, group, and modified
  # time. Permissions are copied regardless `preserve`.
  #
  # If `dereference_root` is true, this method dereference tree root.
  #
  # If `remove_destination` is true, this method removes each destination file
  # before copy.
  sig do
    params(
      src: T.any(String, Pathname),
      dest: T.any(String, Pathname),
      preserve: T::Boolean,
      dereference_root: T::Boolean,
      remove_destination: T::Boolean
    ).void
  end
  module_function def copy_entry(src, dest, preserve = false, dereference_root = false, remove_destination = false); end

  # Copies file contents of `src` to `dest`. Both of `src` and `dest` must be a
  # path name.
  sig do
    params(
      src: T.any(String, Pathname),
      dest: T.any(String, Pathname),
      preserve: T::Boolean,
      dereference: T::Boolean
    ).void
  end
  module_function def copy_file(src, dest, preserve = false, dereference = true); end

  # Copies stream `src` to `dest`. `src` must respond to read(n) and `dest` must
  # respond to write(str).
  sig {params(src: IO, dest: IO).returns(Integer)}
  module_function def copy_stream(src, dest); end

  # Hard link `src` to `dest`. If `src` is a directory, this method links all
  # its contents recursively. If `dest` is a directory, links `src` to
  # `dest/src`.
  #
  # `src` can be a list of files.
  #
  # ```ruby
  # # Installing the library "mylib" under the site_ruby directory.
  # FileUtils.rm_r site_ruby + '/mylib', :force => true
  # FileUtils.cp_lr 'lib/', site_ruby + '/mylib'
  #
  # # Examples of linking several files to target directory.
  # FileUtils.cp_lr %w(mail.rb field.rb debug/), site_ruby + '/tmail'
  # FileUtils.cp_lr Dir.glob('*.rb'), '/home/aamine/lib/ruby', :noop => true, :verbose => true
  #
  # # If you want to link all contents of a directory instead of the
  # # directory itself, c.f. src/x -> dest/x, src/y -> dest/y,
  # # use the following code.
  # FileUtils.cp_lr 'src/.', 'dest'  # cp_lr('src', 'dest') makes dest/src, but this doesn't.
  # ```
  sig do
    params(
      src: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      dest: T.any(String, Pathname),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      dereference_root: T::Boolean,
      remove_destination: T::Boolean
    ).void
  end
  module_function def cp_lr(src, dest, noop: nil, verbose: nil, dereference_root: true, remove_destination: false); end

  # Returns true if the method `mid` have an option `opt`.
  #
  # ```ruby
  # p FileUtils.have_option?(:cp, :noop)     #=> true
  # p FileUtils.have_option?(:rm, :force)    #=> true
  # p FileUtils.have_option?(:rm, :preserve) #=> false
  # ```
  sig {params(mid: Symbol, opt: Symbol).returns(T::Boolean)}
  def self.have_option?(mid, opt); end

  # If `src` is not same as `dest`, copies it and changes the permission mode to
  # `mode`. If `dest` is a directory, destination is `dest`/`src`. This method
  # removes destination before copy.
  #
  # ```ruby
  # FileUtils.install 'ruby', '/usr/local/bin/ruby', :mode => 0755, :verbose => true
  # FileUtils.install 'lib.rb', '/usr/local/lib/ruby/site_ruby', :verbose => true
  # ```
  sig do
    params(
      src: T.any(String, Pathname),
      dest: T.any(String, Pathname),
      mode: T.nilable(T.any(String, Integer)),
      owner: T.nilable(T.any(Integer, String)),
      group: T.nilable(T.any(Integer, String)),
      preserve: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def install(src, dest, mode: nil, owner: nil, group: nil, preserve: nil, noop: nil, verbose: nil); end

  # In the first form, creates a hard link `link` which points to `target`. If
  # `link` already exists, raises Errno::EEXIST. But if the :force option is
  # set, overwrites `link`.
  #
  # ```ruby
  # FileUtils.ln 'gcc', 'cc', verbose: true
  # FileUtils.ln '/usr/bin/emacs21', '/usr/bin/emacs'
  # ```
  #
  # In the second form, creates a link `dir/target` pointing to `target`. In the
  # third form, creates several hard links in the directory `dir`, pointing to
  # each item in `targets`. If `dir` is not a directory, raises Errno::ENOTDIR.
  #
  # ```ruby
  # FileUtils.cd '/sbin'
  # FileUtils.ln %w(cp mv mkdir), '/bin'   # Now /sbin/cp and /bin/cp are linked.
  # ```
  #
  #
  # Also aliased as:
  # [`link`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-link)
  sig do
    params(
      src: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      dest: T.any(String, Pathname),
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def ln(src, dest, force: nil, noop: nil, verbose: nil); end

  # Alias for:
  # [`ln`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-ln)
  sig do
    params(
      src: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      dest: T.any(String, Pathname),
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def link(src, dest, force: nil, noop: nil, verbose: nil); end

  # Hard links a file system entry `src` to `dest`. If `src` is a directory,
  # this method links its contents recursively.
  #
  # Both of `src` and `dest` must be a path name. `src` must exist, `dest` must
  # not exist.
  #
  # If `dereference_root` is true, this method dereferences the tree root.
  #
  # If `remove_destination` is true, this method removes each destination file
  # before copy.
  sig do
    params(
      src: T.any(String, Pathname),
      dest: T.any(String, Pathname),
      dereference_root: T::Boolean,
      remove_destination: T::Boolean
    ).void
  end
  module_function def link_entry(src, dest, dereference_root = false, remove_destination = false); end

  # In the first form, creates a symbolic link `link` which points to `target`.
  # If `link` already exists, raises Errno::EEXIST. But if the :force option is
  # set, overwrites `link`.
  #
  # ```ruby
  # FileUtils.ln_s '/usr/bin/ruby', '/usr/local/bin/ruby'
  # FileUtils.ln_s 'verylongsourcefilename.c', 'c', force: true
  # ```
  #
  # In the second form, creates a link `dir/target` pointing to `target`. In the
  # third form, creates several symbolic links in the directory `dir`, pointing
  # to each item in `targets`. If `dir` is not a directory, raises
  # Errno::ENOTDIR.
  #
  # ```ruby
  # FileUtils.ln_s Dir.glob('/bin/*.rb'), '/home/foo/bin'
  # ```
  #
  #
  # Also aliased as:
  # [`symlink`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-symlink)
  sig do
    params(
      src: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      dest: T.any(String, Pathname),
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def ln_s(src, dest, force: nil, noop: nil, verbose: nil); end

  # Alias for:
  # [`ln_s`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-ln_s)
  sig do
    params(
      src: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      dest: T.any(String, Pathname),
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def symlink(src, dest, force: nil, noop: nil, verbose: nil); end

  # Same as
  #
  # ```ruby
  # FileUtils.ln_s(*args, force: true)
  # ```
  sig do
    params(
      src: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      dest: T.any(String, Pathname),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def ln_sf(src, dest, noop: nil, verbose: nil); end

  # Creates one or more directories.
  #
  # ```ruby
  # FileUtils.mkdir 'test'
  # FileUtils.mkdir %w( tmp data )
  # FileUtils.mkdir 'notexist', :noop => true  # Does not really create.
  # FileUtils.mkdir 'tmp', :mode => 0700
  # ```
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      mode: T.nilable(T.any(String, Integer)),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).returns(T::Array[String])
  end
  module_function def mkdir(list, mode: nil, noop: nil, verbose: nil); end

  # Moves file(s) `src` to `dest`. If `file` and `dest` exist on the different
  # disk partition, the file is copied then the original file is removed.
  #
  # ```ruby
  # FileUtils.mv 'badname.rb', 'goodname.rb'
  # FileUtils.mv 'stuff.rb', '/notexist/lib/ruby', :force => true  # no error
  #
  # FileUtils.mv %w(junk.txt dust.txt), '/home/foo/.trash/'
  # FileUtils.mv Dir.glob('test*.rb'), 'test', :noop => true, :verbose => true
  # ```
  #
  #
  # Also aliased as:
  # [`move`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-move)
  sig do
    params(
      src: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      dest: T.any(String, Pathname),
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      secure: T.nilable(T::Boolean)
    ).returns(T.nilable(Integer))
  end
  module_function def mv(src, dest, force: nil, noop: nil, verbose: nil, secure: nil); end

  # Alias for:
  # [`mv`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-mv)
  sig do
    params(
      src: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      dest: T.any(String, Pathname),
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      secure: T.nilable(T::Boolean)
    ).returns(T.nilable(Integer))
  end
  module_function def move(src, dest, force: nil, noop: nil, verbose: nil, secure: nil); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # option names.
  #
  # ```ruby
  # p FileUtils.options  #=> ["noop", "force", "verbose", "preserve", "mode"]
  # ```
  sig {returns(T::Array[String])}
  def self.options; end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # option names of the method `mid`.
  #
  # ```ruby
  # p FileUtils.options_of(:rm)  #=> ["noop", "verbose", "force"]
  # ```
  sig {params(mid: Symbol).returns(T::Array[String])}
  def self.options_of(mid); end

  sig {params(name: Symbol).void}
  def self.private_module_function(name); end

  # Returns the name of the current directory.
  #
  # Also aliased as:
  # [`getwd`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-getwd)
  sig {returns(String)}
  module_function def pwd; end

  # Alias for:
  # [`pwd`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-pwd)
  sig {returns(String)}
  module_function def getwd; end

  # Removes a directory `dir` and its contents recursively. This method ignores
  # [`StandardError`](https://docs.ruby-lang.org/en/2.6.0/StandardError.html) if
  # `force` is true.
  sig {params(path: T.any(String, Pathname), force: T::Boolean).returns(Integer)}
  module_function def remove_dir(path, force = false); end

  # This method removes a file system entry `path`. `path` might be a regular
  # file, a directory, or something. If `path` is a directory, remove it
  # recursively.
  #
  # See also
  # [`remove_entry_secure`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-remove_entry_secure).
  sig {params(path: T.any(String, Pathname), force: T::Boolean).returns(Integer)}
  module_function def remove_entry(path, force = false); end

  # This method removes a file system entry `path`. `path` shall be a regular
  # file, a directory, or something. If `path` is a directory, remove it
  # recursively. This method is required to avoid TOCTTOU
  # (time-of-check-to-time-of-use) local security vulnerability of
  # [`rm_r`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-rm_r).
  # [`rm_r`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-rm_r)
  # causes security hole when:
  #
  # *   Parent directory is world writable (including /tmp).
  # *   Removing directory tree includes world writable directory.
  # *   The system has symbolic link.
  #
  #
  # To avoid this security hole, this method applies special preprocess. If
  # `path` is a directory, this method chown(2) and chmod(2) all removing
  # directories. This requires the current process is the owner of the removing
  # whole directory tree, or is the super user (root).
  #
  # WARNING: You must ensure that **ALL** parent directories cannot be moved by
  # other untrusted users. For example, parent directories should not be owned
  # by untrusted users, and should not be world writable except when the sticky
  # bit set.
  #
  # WARNING: Only the owner of the removing directory tree, or Unix super user
  # (root) should invoke this method. Otherwise this method does not work.
  #
  # For details of this security vulnerability, see Perl's case:
  #
  # *   https://cve.mitre.org/cgi-bin/cvename.cgi?name=CAN-2005-0448
  # *   https://cve.mitre.org/cgi-bin/cvename.cgi?name=CAN-2004-0452
  #
  #
  # For fileutils.rb, this vulnerability is reported in [ruby-dev:26100].
  sig {params(path: T.any(String, Pathname), force: T::Boolean).void}
  module_function def remove_entry_secure(path, force = false); end

  # Removes a file `path`. This method ignores
  # [`StandardError`](https://docs.ruby-lang.org/en/2.6.0/StandardError.html) if
  # `force` is true.
  sig {params(path: T.any(String, Pathname), force: T::Boolean).returns(Integer)}
  module_function def remove_file(path, force = false); end

  # Remove file(s) specified in `list`. This method cannot remove directories.
  # All StandardErrors are ignored when the :force option is set.
  #
  # ```ruby
  # FileUtils.rm %w( junk.txt dust.txt )
  # FileUtils.rm Dir.glob('*.so')
  # FileUtils.rm 'NotExistFile', :force => true   # never raises exception
  # ```
  #
  #
  # Also aliased as:
  # [`remove`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-remove)
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def rm(list, force: nil, noop: nil, verbose: nil); end

  # Alias for:
  # [`rm`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-rm)
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      force: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def remove(list, force: nil, noop: nil, verbose: nil); end

  # Equivalent to
  #
  # ```ruby
  # FileUtils.rm(list, :force => true)
  # ```
  #
  #
  # Also aliased as:
  # [`safe_unlink`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-safe_unlink)
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def rm_f(list, noop: nil, verbose: nil); end

  # Alias for:
  # [`rm_f`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-rm_f)
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def safe_unlink(list, noop: nil, verbose: nil); end

  # Equivalent to
  #
  # ```ruby
  # FileUtils.rm_r(list, :force => true)
  # ```
  #
  # WARNING: This method causes local vulnerability. Read the documentation of
  # [`rm_r`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-rm_r)
  # first.
  #
  # Also aliased as:
  # [`rmtree`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-c-rmtree)
  sig do
    params(
      list: T.any(String, T::Array[T.any(String, Pathname)], Pathname),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      secure: T.nilable(T::Boolean)
    ).void
  end
  module_function def rm_rf(list, noop: nil, verbose: nil, secure: nil); end

  # Alias for:
  # [`rm_rf`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html#method-i-rm_rf)
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean),
      secure: T.nilable(T::Boolean)
    ).void
  end
  module_function def rmtree(list, noop: nil, verbose: nil, secure: nil); end

  # Removes one or more directories.
  #
  # ```ruby
  # FileUtils.rmdir 'somedir'
  # FileUtils.rmdir %w(somedir anydir otherdir)
  # # Does not really remove directory; outputs message.
  # FileUtils.rmdir 'somedir', :verbose => true, :noop => true
  # ```
  sig do
    params(
      list: T.any(String, Pathname, T::Array[T.any(String, Pathname)]),
      parents: T.nilable(T::Boolean),
      noop: T.nilable(T::Boolean),
      verbose: T.nilable(T::Boolean)
    ).void
  end
  module_function def rmdir(list, parents: nil, noop: nil, verbose: nil); end

  # Returns true if `new` is newer than all `old_list`. Non-existent files are
  # older than any file.
  #
  # ```ruby
  # FileUtils.uptodate?('hello.o', %w(hello.c hello.h)) or \
  #     system 'make hello.o'
  # ```
  sig {params(new: T.any(String, Pathname), old_list: T::Array[T.any(String, Pathname)]).returns(T::Boolean)}
  module_function def uptodate?(new, old_list); end
end

# This module has all methods of
# [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html) module, but
# never changes files/directories, with printing message before acting. This
# equates to passing the `:noop` and `:verbose` flag to methods in
# [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html).
module FileUtils::DryRun
  LOW_METHODS = T.let(T.unsafe(nil), T::Array[Symbol])
  METHODS = T.let(T.unsafe(nil), T::Array[Symbol])
  OPT_TABLE = T.let(T.unsafe(nil), T::Hash[String, T::Array[Symbol]])
end

class ::FileUtils::Entry_ < Object
  include FileUtils::StreamUtils_

  DIRECTORY_TERM = T.let(T.unsafe(nil), String)
  SYSCASE = T.let(T.unsafe(nil), String)
  S_IF_DOOR = T.let(T.unsafe(nil), Integer)

  sig {returns(T::Boolean)}
  def blockdev?; end

  sig {returns(T::Boolean)}
  def chardev?; end

  sig {params(mode: Integer).returns(Integer)}
  def chmod(mode); end

  sig {params(uid: Integer, gid: Integer).returns(Integer)}
  def chown(uid, gid); end

  sig {params(dest: T.any(String, Pathname)).returns(Integer)}
  def copy(dest); end

  sig {params(dest: T.any(String, Pathname)).returns(Integer)}
  def copy_file(dest); end

  sig {params(path: T.any(String, Pathname)).returns(Integer)}
  def copy_metadata(path); end

  sig {returns(T::Boolean)}
  def dereference?; end

  sig {returns(T::Boolean)}
  def directory?; end

  sig {returns(T::Boolean)}
  def door?; end

  sig {returns(T::Array[FileUtils::Entry_])}
  def entries; end

  sig {returns(T::Boolean)}
  def exist?; end

  sig {returns(T::Boolean)}
  def file?; end

  sig {params(a: T.nilable(T.any(String, Pathname)), b: T.nilable(T.any(String, Pathname)), deref: T::Boolean).void}
  def initialize(a, b = nil, deref = false); end

  sig {params(dest: T.any(String, Pathname)).returns(Integer)}
  def link(dest); end

  sig {returns(File::Stat)}
  def lstat; end

  sig {returns(T.nilable(File::Stat))}
  def lstat!; end

  sig {returns(String)}
  def path; end

  sig {returns(T::Boolean)}
  def pipe?; end

  sig do
    type_parameters(:U)
    .params(blk: T.proc.returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
  end
  def platform_support(&blk); end

  sig do
    type_parameters(:U)
    .params(blk: T.proc.returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
  end
  def postorder_traverse(&blk); end

  sig {returns(T.nilable(T.any(String, Pathname)))}
  def prefix; end

  sig do
    type_parameters(:U)
    .params(blk: T.proc.returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
  end
  def preorder_traverse(&blk); end

  sig {returns(T.nilable(T.any(String, Pathname)))}
  def rel; end

  sig {returns(Integer)}
  def remove; end

  sig {returns(Integer)}
  def remove_dir1; end

  sig {returns(Integer)}
  def remove_file; end

  sig {returns(T::Boolean)}
  def socket?; end

  sig {returns(File::Stat)}
  def stat; end

  sig {returns(T.nilable(File::Stat))}
  def stat!; end

  sig {returns(T::Boolean)}
  def symlink?; end

  sig do
    type_parameters(:U)
    .params(blk: T.proc.returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
  end
  def traverse(&blk); end

  sig do
    params(
      pre: T.proc.params(obj: FileUtils::Entry_).void,
      post: T.proc.params(obj: FileUtils::Entry_).void
    )
    .void
  end
  def wrap_traverse(pre, post); end
end

module FileUtils::LowMethods
end

# This module has all methods of
# [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html) module, but
# never changes files/directories.  This equates to passing the `:noop` flag to
# methods in [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html).
module FileUtils::NoWrite
  LOW_METHODS = T.let(T.unsafe(nil), T::Array[Symbol])
  METHODS = T.let(T.unsafe(nil), T::Array[Symbol])
  OPT_TABLE = T.let(T.unsafe(nil), T::Hash[String, T::Array[Symbol]])
end

module FileUtils::StreamUtils_
end

# This module has all methods of
# [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html) module, but
# it outputs messages before acting. This equates to passing the `:verbose` flag
# to methods in
# [`FileUtils`](https://docs.ruby-lang.org/en/2.6.0/FileUtils.html).
module FileUtils::Verbose
  LOW_METHODS = T.let(T.unsafe(nil), T::Array[Symbol])
  METHODS = T.let(T.unsafe(nil), T::Array[Symbol])
  OPT_TABLE = T.let(T.unsafe(nil), T::Hash[String, T::Array[Symbol]])
end
