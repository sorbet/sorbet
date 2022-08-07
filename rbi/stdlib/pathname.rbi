# typed: __STDLIB_INTERNAL

# [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) represents the
# name of a file or directory on the filesystem, but not the file itself.
#
# The pathname depends on the Operating System: Unix, Windows, etc. This library
# works with pathnames of local OS, however non-Unix pathnames are supported
# experimentally.
#
# A [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) can be
# relative or absolute. It's not until you try to reference the file that it
# even matters whether the file exists or not.
#
# [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) is immutable.
# It has no method for destructive update.
#
# The goal of this class is to manipulate file path information in a neater way
# than standard Ruby provides. The examples below demonstrate the difference.
#
# **All** functionality from
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html),
# [`FileTest`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html), and some from
# [`Dir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html) and
# [`FileUtils`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html) is included,
# in an unsurprising way. It is essentially a facade for all of these, and more.
#
# ## Examples
#
# ### Example 1: Using [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html)
#
# ```ruby
# require 'pathname'
# pn = Pathname.new("/usr/bin/ruby")
# size = pn.size              # 27662
# isdir = pn.directory?       # false
# dir  = pn.dirname           # Pathname:/usr/bin
# base = pn.basename          # Pathname:ruby
# dir, base = pn.split        # [Pathname:/usr/bin, Pathname:ruby]
# data = pn.read
# pn.open { |f| _ }
# pn.each_line { |line| _ }
# ```
#
# ### Example 2: Using standard Ruby
#
# ```ruby
# pn = "/usr/bin/ruby"
# size = File.size(pn)        # 27662
# isdir = File.directory?(pn) # false
# dir  = File.dirname(pn)     # "/usr/bin"
# base = File.basename(pn)    # "ruby"
# dir, base = File.split(pn)  # ["/usr/bin", "ruby"]
# data = File.read(pn)
# File.open(pn) { |f| _ }
# File.foreach(pn) { |line| _ }
# ```
#
# ### Example 3: Special features
#
# ```ruby
# p1 = Pathname.new("/usr/lib")   # Pathname:/usr/lib
# p2 = p1 + "ruby/1.8"            # Pathname:/usr/lib/ruby/1.8
# p3 = p1.parent                  # Pathname:/usr
# p4 = p2.relative_path_from(p3)  # Pathname:lib/ruby/1.8
# pwd = Pathname.pwd              # Pathname:/home/gavin
# pwd.absolute?                   # true
# p5 = Pathname.new "."           # Pathname:.
# p5 = p5 + "music/../articles"   # Pathname:music/../articles
# p5.cleanpath                    # Pathname:articles
# p5.realpath                     # Pathname:/home/gavin/articles
# p5.children                     # [Pathname:/home/gavin/articles/linux, ...]
# ```
#
# ## Breakdown of functionality
#
# ### Core methods
#
# These methods are effectively manipulating a
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), because that's
# all a path is. None of these access the file system except for
# [`mountpoint?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-mountpoint-3F),
# [`children`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-children),
# [`each_child`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-each_child),
# [`realdirpath`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-realdirpath)
# and
# [`realpath`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-realpath).
#
# *   +
# *   [`join`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-join)
# *   [`parent`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-parent)
# *   [`root?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-root-3F)
# *   [`absolute?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-absolute-3F)
# *   [`relative?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-relative-3F)
# *   [`relative_path_from`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-relative_path_from)
# *   [`each_filename`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-each_filename)
# *   [`cleanpath`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-cleanpath)
# *   [`realpath`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-realpath)
# *   [`realdirpath`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-realdirpath)
# *   [`children`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-children)
# *   [`each_child`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-each_child)
# *   [`mountpoint?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-mountpoint-3F)
#
#
# ### [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) status predicate methods
#
# These methods are a facade for FileTest:
# *   [`blockdev?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-blockdev-3F)
# *   [`chardev?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-chardev-3F)
# *   [`directory?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-directory-3F)
# *   [`executable?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-executable-3F)
# *   [`executable_real?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-executable_real-3F)
# *   [`exist?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-exist-3F)
# *   [`file?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-file-3F)
# *   [`grpowned?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-grpowned-3F)
# *   [`owned?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-owned-3F)
# *   [`pipe?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-pipe-3F)
# *   [`readable?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-readable-3F)
# *   [`world_readable?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-world_readable-3F)
# *   [`readable_real?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-readable_real-3F)
# *   [`setgid?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-setgid-3F)
# *   [`setuid?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-setuid-3F)
# *   [`size`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-size)
# *   [`size?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-size-3F)
# *   [`socket?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-socket-3F)
# *   [`sticky?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-sticky-3F)
# *   [`symlink?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-symlink-3F)
# *   [`writable?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-writable-3F)
# *   [`world_writable?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-world_writable-3F)
# *   [`writable_real?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-writable_real-3F)
# *   [`zero?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-zero-3F)
#
#
# ### [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) property and manipulation methods
#
# These methods are a facade for File:
# *   [`atime`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-atime)
# *   [`birthtime`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-birthtime)
# *   [`ctime`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-ctime)
# *   [`mtime`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-mtime)
# *   [`chmod(mode)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-chmod)
# *   [`lchmod(mode)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-lchmod)
# *   [`chown`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-chown)(owner,
#     group)
# *   [`lchown`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-lchown)(owner,
#     group)
# *   [`fnmatch`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-fnmatch)(pattern,
#     \*args)
# *   [`fnmatch?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-fnmatch-3F)(pattern,
#     \*args)
# *   [`ftype`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-ftype)
# *   [`make_link(old)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-make_link)
# *   [`open`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-open)(\*args,
#     &block)
# *   [`readlink`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-readlink)
# *   [`rename(to)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-rename)
# *   [`stat`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-stat)
# *   [`lstat`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-lstat)
# *   [`make_symlink(old)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-make_symlink)
# *   [`truncate(length)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-truncate)
# *   [`utime`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-utime)(atime,
#     mtime)
# *   [`basename(*args)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-basename)
# *   [`dirname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-dirname)
# *   [`extname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-extname)
# *   [`expand_path(*args)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-expand_path)
# *   [`split`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-split)
#
#
# ### Directory methods
#
# These methods are a facade for Dir:
# *   [`Pathname.glob(*args)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-c-glob)
# *   [`Pathname.getwd`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-c-getwd)
#     /
#     [`Pathname.pwd`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-c-pwd)
# *   [`rmdir`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-rmdir)
# *   [`entries`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-entries)
# *   [`each_entry`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-each_entry)(&block)
# *   [`mkdir(*args)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-mkdir)
# *   [`opendir(*args)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-opendir)
#
#
# ### [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
#
# These methods are a facade for IO:
# *   [`each_line`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-each_line)(\*args,
#     &block)
# *   [`read(*args)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-read)
# *   [`binread(*args)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-binread)
# *   [`readlines(*args)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-readlines)
# *   [`sysopen(*args)`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-sysopen)
#
#
# ### Utilities
#
# These methods are a mixture of
# [`Find`](https://docs.ruby-lang.org/en/2.7.0/Find.html),
# [`FileUtils`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html), and others:
# *   [`find`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-find)(&block)
# *   [`mkpath`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-mkpath)
# *   [`rmtree`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-rmtree)
# *   [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-unlink)
#     /
#     [`delete`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-delete)
#
#
# ## [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) documentation
#
# As the above section shows, most of the methods in
# [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) are facades.
# The documentation for these methods generally just says, for instance, "See
# [`FileTest.writable?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-writable-3F)",
# as you should be familiar with the original method anyway, and its
# documentation (e.g. through `ri`) will contain more information. In some
# cases, a brief description will follow.
class Pathname < Object
  SAME_PATHS = T.let(T.unsafe(nil), Proc)
  SEPARATOR_LIST = T.let(T.unsafe(nil), String)
  SEPARATOR_PAT = T.let(T.unsafe(nil), Regexp)
  TO_PATH = T.let(T.unsafe(nil), Symbol)

  # Returns the current working directory as a
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html).
  #
  # ```ruby
  # Pathname.getwd
  #     #=> #<Pathname:/home/zzak/projects/ruby>
  # ```
  #
  # See
  # [`Dir.getwd`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-getwd).
  sig {returns(Pathname)}
  def self.getwd(); end

  # Returns or yields
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) objects.
  #
  # ```ruby
  # Pathname.glob("lib/i*.rb")
  #     #=> [#<Pathname:lib/ipaddr.rb>, #<Pathname:lib/irb.rb>]
  # ```
  #
  # See
  # [`Dir.glob`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-glob).
  sig do
    params(
        p1: T.any(String, Pathname),
        p2: Integer,
    )
    .returns(T::Array[Pathname])
  end
  sig do
    params(
        p1: T.any(String, Pathname),
        p2: Integer,
        blk: T.proc.params(arg0: Pathname).void,
    )
    .returns(NilClass)
  end
  def self.glob(p1, p2=0, &blk); end

  def initialize(p); end

  # Appends a pathname fragment to `self` to produce a new
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) object.
  #
  # ```ruby
  # p1 = Pathname.new("/usr")      # Pathname:/usr
  # p2 = p1 + "bin/ruby"           # Pathname:/usr/bin/ruby
  # p3 = p1 + "/etc/passwd"        # Pathname:/etc/passwd
  #
  # # / is aliased to +.
  # p4 = p1 / "bin/ruby"           # Pathname:/usr/bin/ruby
  # p5 = p1 / "/etc/passwd"        # Pathname:/etc/passwd
  # ```
  #
  # This method doesn't access the file system; it is pure string manipulation.
  #
  # Also aliased as:
  # [`/`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-2F)
  sig do
    params(
        other: T.any(String, Pathname),
    )
    .returns(Pathname)
  end
  def +(other); end

  # Provides a case-sensitive comparison operator for pathnames.
  #
  # ```ruby
  # Pathname.new('/usr') <=> Pathname.new('/usr/bin')
  #     #=> -1
  # Pathname.new('/usr/bin') <=> Pathname.new('/usr/bin')
  #     #=> 0
  # Pathname.new('/usr/bin') <=> Pathname.new('/USR/BIN')
  #     #=> 1
  # ```
  #
  # It will return `-1`, `0` or `1` depending on the value of the left argument
  # relative to the right argument. Or it will return `nil` if the arguments are
  # not comparable.
  sig do
    params(
        p1: BasicObject,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(p1); end

  # Compare this pathname with `other`. The comparison is string-based. Be aware
  # that two different paths (`foo.txt` and `./foo.txt`) can refer to the same
  # file.
  sig do
    params(
        p1: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(p1); end

  # Compare this pathname with `other`. The comparison is string-based. Be aware
  # that two different paths (`foo.txt` and `./foo.txt`) can refer to the same
  # file.
  sig do
    params(
        p1: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(p1); end

  # Predicate method for testing whether a path is absolute.
  #
  # It returns `true` if the pathname begins with a slash.
  #
  # ```ruby
  # p = Pathname.new('/im/sure')
  # p.absolute?
  #     #=> true
  #
  # p = Pathname.new('not/so/sure')
  # p.absolute?
  #     #=> false
  # ```
  sig {returns(T::Boolean)}
  def absolute?(); end

  # Iterates over and yields a new
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) object for
  # each element in the given path in ascending order.
  #
  # ```ruby
  # Pathname.new('/path/to/some/file.rb').ascend {|v| p v}
  #    #<Pathname:/path/to/some/file.rb>
  #    #<Pathname:/path/to/some>
  #    #<Pathname:/path/to>
  #    #<Pathname:/path>
  #    #<Pathname:/>
  #
  # Pathname.new('path/to/some/file.rb').ascend {|v| p v}
  #    #<Pathname:path/to/some/file.rb>
  #    #<Pathname:path/to/some>
  #    #<Pathname:path/to>
  #    #<Pathname:path>
  # ```
  #
  # Returns an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) if no
  # block was given.
  #
  # ```
  # enum = Pathname.new("/usr/bin/ruby").ascend
  #   # ... do stuff ...
  # enum.each { |e| ... }
  #   # yields Pathnames /usr/bin/ruby, /usr/bin, /usr, and /.
  # ```
  #
  # It doesn't access the filesystem.
  sig do
    params(
        blk: T.proc.params(arg0: Pathname).returns(BasicObject),
    )
    .void
  end
  sig {returns(T::Enumerator[Pathname])}
  def ascend(&blk); end

  # Returns the last access time for the file.
  #
  # See
  # [`File.atime`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-atime).
  sig {returns(Time)}
  def atime(); end

  # Returns the last component of the path.
  #
  # See
  # [`File.basename`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-basename).
  sig do
    params(
        p1: String,
    )
    .returns(Pathname)
  end
  def basename(p1=T.unsafe(nil)); end

  # Returns all the bytes from the file, or the first `N` if specified.
  #
  # See
  # [`File.binread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-binread).
  sig do
    params(
        length: Integer,
        offset: Integer,
    )
    .returns(String)
  end
  def binread(length=T.unsafe(nil), offset=T.unsafe(nil)); end

  # Writes `contents` to the file, opening it in binary mode.
  #
  # See
  # [`File.binwrite`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-binwrite).
  sig do
    params(
        arg0: String,
        offset: Integer,
        external_encoding: String,
        internal_encoding: String,
        encoding: String,
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
    )
    .returns(Integer)
  end
  def binwrite(arg0, offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  # Returns the birth time for the file. If the platform doesn't have birthtime,
  # raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html).
  #
  # See
  # [`File.birthtime`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-birthtime).
  sig {returns(Time)}
  def birthtime(); end

  # See
  # [`FileTest.blockdev?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-blockdev-3F).
  sig {returns(T::Boolean)}
  def blockdev?(); end

  # See
  # [`FileTest.chardev?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-chardev-3F).
  sig {returns(T::Boolean)}
  def chardev?(); end

  # Returns the children of the directory (files and subdirectories, not
  # recursive) as an array of
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) objects.
  #
  # By default, the returned pathnames will have enough information to access
  # the files. If you set `with_directory` to `false`, then the returned
  # pathnames will contain the filename only.
  #
  # For example:
  #
  # ```
  # pn = Pathname("/usr/lib/ruby/1.8")
  # pn.children
  #     # -> [ Pathname:/usr/lib/ruby/1.8/English.rb,
  #            Pathname:/usr/lib/ruby/1.8/Env.rb,
  #            Pathname:/usr/lib/ruby/1.8/abbrev.rb, ... ]
  # pn.children(false)
  #     # -> [ Pathname:English.rb, Pathname:Env.rb, Pathname:abbrev.rb, ... ]
  # ```
  #
  # Note that the results never contain the entries `.` and `..` in the
  # directory because they are not children.
  sig do
    params(
        with_directory: T::Boolean,
    )
    .returns(T::Array[Pathname])
  end
  def children(with_directory=T.unsafe(nil)); end

  # Changes file permissions.
  #
  # See
  # [`File.chmod`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-chmod).
  sig do
    params(
        mode: Integer,
    )
    .returns(Integer)
  end
  def chmod(mode); end

  # Change owner and group of the file.
  #
  # See
  # [`File.chown`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-chown).
  sig do
    params(
        owner: T.nilable(Integer),
        group: T.nilable(Integer),
    )
    .returns(Integer)
  end
  def chown(owner, group); end

  # Returns clean pathname of `self` with consecutive slashes and useless dots
  # removed. The filesystem is not accessed.
  #
  # If `consider_symlink` is `true`, then a more conservative algorithm is used
  # to avoid breaking symbolic linkages. This may retain more `..` entries than
  # absolutely necessary, but without accessing the filesystem, this can't be
  # avoided.
  #
  # See
  # [`Pathname#realpath`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-realpath).
  sig do
    params(
        consider_symlink: T::Boolean,
    )
    .returns(Pathname)
  end
  def cleanpath(consider_symlink=T.unsafe(nil)); end

  # Returns the last change time, using directory information, not the file
  # itself.
  #
  # See
  # [`File.ctime`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-ctime).
  sig {returns(Time)}
  def ctime(); end

  # Removes a file or directory, using
  # [`File.unlink`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-unlink)
  # if `self` is a file, or
  # [`Dir.unlink`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-unlink)
  # as necessary.
  sig {void}
  def delete(); end

  # Iterates over and yields a new
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) object for
  # each element in the given path in descending order.
  #
  # ```ruby
  # Pathname.new('/path/to/some/file.rb').descend {|v| p v}
  #    #<Pathname:/>
  #    #<Pathname:/path>
  #    #<Pathname:/path/to>
  #    #<Pathname:/path/to/some>
  #    #<Pathname:/path/to/some/file.rb>
  #
  # Pathname.new('path/to/some/file.rb').descend {|v| p v}
  #    #<Pathname:path>
  #    #<Pathname:path/to>
  #    #<Pathname:path/to/some>
  #    #<Pathname:path/to/some/file.rb>
  # ```
  #
  # Returns an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) if no
  # block was given.
  #
  # ```
  # enum = Pathname.new("/usr/bin/ruby").descend
  #   # ... do stuff ...
  # enum.each { |e| ... }
  #   # yields Pathnames /, /usr, /usr/bin, and /usr/bin/ruby.
  # ```
  #
  # It doesn't access the filesystem.
  sig do
    params(
        blk: T.proc.params(arg0: Pathname).returns(BasicObject),
    )
    .void
  end
  sig {returns(T::Enumerator[Pathname])}
  def descend(&blk); end

  # See
  # [`FileTest.directory?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-directory-3F).
  sig {returns(T::Boolean)}
  def directory?(); end

  # Returns all but the last component of the path.
  #
  # See
  # [`File.dirname`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-dirname).
  sig {returns(Pathname)}
  def dirname(); end

  # Iterates over the children of the directory (files and subdirectories, not
  # recursive).
  #
  # It yields [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html)
  # object for each child.
  #
  # By default, the yielded pathnames will have enough information to access the
  # files.
  #
  # If you set `with_directory` to `false`, then the returned pathnames will
  # contain the filename only.
  #
  # ```ruby
  # Pathname("/usr/local").each_child {|f| p f }
  # #=> #<Pathname:/usr/local/share>
  # #   #<Pathname:/usr/local/bin>
  # #   #<Pathname:/usr/local/games>
  # #   #<Pathname:/usr/local/lib>
  # #   #<Pathname:/usr/local/include>
  # #   #<Pathname:/usr/local/sbin>
  # #   #<Pathname:/usr/local/src>
  # #   #<Pathname:/usr/local/man>
  #
  # Pathname("/usr/local").each_child(false) {|f| p f }
  # #=> #<Pathname:share>
  # #   #<Pathname:bin>
  # #   #<Pathname:games>
  # #   #<Pathname:lib>
  # #   #<Pathname:include>
  # #   #<Pathname:sbin>
  # #   #<Pathname:src>
  # #   #<Pathname:man>
  # ```
  #
  # Note that the results never contain the entries `.` and `..` in the
  # directory because they are not children.
  #
  # See
  # [`Pathname#children`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-children)
  sig do
    params(
        with_directory: T::Boolean,
        blk: T.proc.params(arg0: Pathname).returns(BasicObject),
    )
    .void
  end
  def each_child(with_directory = true, &blk); end

  # Iterates over the entries (files and subdirectories) in the directory,
  # yielding a [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html)
  # object for each entry.
  sig do
    params(
        blk: T.proc.params(arg0: Pathname).returns(BasicObject),
    )
    .void
  end
  def each_entry(&blk); end

  # Iterates over each component of the path.
  #
  # ```
  # Pathname.new("/usr/bin/ruby").each_filename {|filename| ... }
  #   # yields "usr", "bin", and "ruby".
  # ```
  #
  # Returns an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) if no
  # block was given.
  #
  # ```
  # enum = Pathname.new("/usr/bin/ruby").each_filename
  #   # ... do stuff ...
  # enum.each { |e| ... }
  #   # yields "usr", "bin", and "ruby".
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .void
  end
  sig {returns(T::Enumerator[String])}
  def each_filename(&blk); end

  # Iterates over each line in the file and yields a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) object for each.
  sig do
    params(
        sep: String,
        limit: Integer,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .void
  end
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T::Enumerator[String])
  end
  def each_line(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  # Tests the file is empty.
  #
  # See Dir#empty? and
  # [`FileTest.empty?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-empty-3F).
  sig { returns(T::Boolean) }
  def empty?; end

  # Return the entries (files and subdirectories) in the directory, each as a
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) object.
  #
  # The results contains just the names in the directory, without any trailing
  # slashes or recursive look-up.
  #
  # ```ruby
  # pp Pathname.new('/usr/local').entries
  # #=> [#<Pathname:share>,
  # #    #<Pathname:lib>,
  # #    #<Pathname:..>,
  # #    #<Pathname:include>,
  # #    #<Pathname:etc>,
  # #    #<Pathname:bin>,
  # #    #<Pathname:man>,
  # #    #<Pathname:games>,
  # #    #<Pathname:.>,
  # #    #<Pathname:sbin>,
  # #    #<Pathname:src>]
  # ```
  #
  # The result may contain the current directory `#<Pathname:.>` and the parent
  # directory `#<Pathname:..>`.
  #
  # If you don't want `.` and `..` and want directories, consider
  # [`Pathname#children`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-children).
  sig {returns(T::Array[Pathname])}
  def entries(); end

  # Compare this pathname with `other`. The comparison is string-based. Be aware
  # that two different paths (`foo.txt` and `./foo.txt`) can refer to the same
  # file.
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  # See
  # [`FileTest.executable?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-executable-3F).
  sig {returns(T::Boolean)}
  def executable?(); end

  # See
  # [`FileTest.executable_real?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-executable_real-3F).
  sig {returns(T::Boolean)}
  def executable_real?(); end

  # See
  # [`FileTest.exist?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-exist-3F).
  sig {returns(T::Boolean)}
  def exist?(); end

  # Returns the absolute path for the file.
  #
  # See
  # [`File.expand_path`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-expand_path).
  sig do
    params(
        p1: T.any(String, Pathname),
    )
    .returns(Pathname)
  end
  def expand_path(p1=T.unsafe(nil)); end

  # Returns the file's extension.
  #
  # See
  # [`File.extname`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-extname).
  sig {returns(String)}
  def extname(); end

  # See
  # [`FileTest.file?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-file-3F).
  sig {returns(T::Boolean)}
  def file?(); end

  # Iterates over the directory tree in a depth first manner, yielding a
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) for each
  # file under "this" directory.
  #
  # Returns an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) if no
  # block is given.
  #
  # Since it is implemented by the standard library module
  # [`Find`](https://docs.ruby-lang.org/en/2.7.0/Find.html),
  # [`Find.prune`](https://docs.ruby-lang.org/en/2.7.0/Find.html#method-c-prune)
  # can be used to control the traversal.
  #
  # If `self` is `.`, yielded pathnames begin with a filename in the current
  # directory, not `./`.
  #
  # See
  # [`Find.find`](https://docs.ruby-lang.org/en/2.7.0/Find.html#method-c-find)
  sig do
    params(
        ignore_error: T::Boolean,
    )
    .returns(T::Enumerator[Pathname])
  end
  sig do
    params(
        ignore_error: T::Boolean,
        blk: T.proc.params(arg0: Pathname).void,
    )
    .returns(NilClass)
  end
  def find(ignore_error: true, &blk); end

  # Return `true` if the receiver matches the given pattern.
  #
  # See
  # [`File.fnmatch`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-fnmatch).
  sig do
    params(
        pattern: String,
        flags: Integer,
    )
    .returns(T::Boolean)
  end
  def fnmatch(pattern, flags=T.unsafe(nil)); end

  # Return `true` if the receiver matches the given pattern.
  #
  # See
  # [`File.fnmatch`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-fnmatch).
  sig do
    params(
        pattern: String,
        flags: Integer,
    )
    .returns(T::Boolean)
  end
  def fnmatch?(pattern, flags=T.unsafe(nil)); end

  # Freezes this
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html).
  #
  # See
  # [`Object.freeze`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-freeze).
  sig {returns(T.self_type)}
  def freeze(); end

  # Returns "type" of file ("file", "directory", etc).
  #
  # See
  # [`File.ftype`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-ftype).
  sig {returns(String)}
  def ftype(); end

  sig do
    params(
        p1: T.any(String, Pathname),
        p2: Integer,
    )
    .returns(T::Array[Pathname])
  end
  sig do
    params(
        p1: T.any(String, Pathname),
        p2: Integer,
        blk: T.proc.params(arg0: Pathname).void
    )
    .returns(NilClass)
  end
  def glob(p1, p2=0, &blk); end

  # See
  # [`FileTest.grpowned?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-grpowned-3F).
  sig {returns(T::Boolean)}
  def grpowned?(); end

  # Joins the given pathnames onto `self` to create a new
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) object.
  #
  # ```ruby
  # path0 = Pathname.new("/usr")                # Pathname:/usr
  # path0 = path0.join("bin/ruby")              # Pathname:/usr/bin/ruby
  #     # is the same as
  # path1 = Pathname.new("/usr") + "bin/ruby"   # Pathname:/usr/bin/ruby
  # path0 == path1
  #     #=> true
  # ```
  sig do
    params(
        args: T.any(String, Pathname),
    )
    .returns(Pathname)
  end
  def join(*args); end

  # Same as
  # [`Pathname.chmod`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-chmod),
  # but does not follow symbolic links.
  #
  # See
  # [`File.lchmod`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-lchmod).
  sig do
    params(
        mode: Integer,
    )
    .returns(Integer)
  end
  def lchmod(mode); end

  # Same as
  # [`Pathname.chown`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-chown),
  # but does not follow symbolic links.
  #
  # See
  # [`File.lchown`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-lchown).
  sig do
    params(
        owner: T.nilable(Integer),
        group: T.nilable(Integer),
    )
    .returns(Integer)
  end
  def lchown(owner, group); end

  # See
  # [`File.lstat`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-lstat).
  sig {returns(File::Stat)}
  def lstat(); end

  # Creates a hard link at *pathname*.
  #
  # See
  # [`File.link`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-link).
  sig do
    params(
        old: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def make_link(old); end

  # Creates a symbolic link.
  #
  # See
  # [`File.symlink`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-symlink).
  sig do
    params(
        old: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def make_symlink(old); end

  # Create the referenced directory.
  #
  # See
  # [`Dir.mkdir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-mkdir).
  sig do
    params(
        p1: Integer,
    )
    .returns(Integer)
  end
  def mkdir(p1); end

  # Creates a full path, including any intermediate directories that don't yet
  # exist.
  #
  # See
  # [`FileUtils.mkpath`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-mkpath)
  # and
  # [`FileUtils.mkdir_p`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-mkdir_p)
  sig {void}
  def mkpath(); end

  # Returns `true` if `self` points to a mountpoint.
  sig {returns(T::Boolean)}
  def mountpoint?(); end

  # Returns the last modified time of the file.
  #
  # See
  # [`File.mtime`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-mtime).
  sig {returns(Time)}
  def mtime(); end

  # Opens the file for reading or writing.
  #
  # See
  # [`File.open`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-open).
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

  # Opens the referenced directory.
  #
  # See
  # [`Dir.open`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-open).
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

  # See
  # [`FileTest.owned?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-owned-3F).
  sig {returns(T::Boolean)}
  def owned?(); end

  # Returns the parent directory.
  #
  # This is same as `self + '..'`.
  sig {returns(Pathname)}
  def parent(); end

  # See
  # [`FileTest.pipe?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-pipe-3F).
  sig {returns(T::Boolean)}
  def pipe?(); end

  # Returns all data from the file, or the first `N` bytes if specified.
  #
  # See
  # [`File.read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-read).
  sig do
    params(
        length: Integer,
        offset: Integer,
        open_args: Integer,
    )
    .returns(String)
  end
  def read(length=T.unsafe(nil), offset=T.unsafe(nil), open_args=T.unsafe(nil)); end

  # See
  # [`FileTest.readable?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-readable-3F).
  sig {returns(T::Boolean)}
  def readable?(); end

  # See
  # [`FileTest.readable_real?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-readable_real-3F).
  sig {returns(T::Boolean)}
  def readable_real?(); end

  # Returns all the lines from the file.
  #
  # See
  # [`File.readlines`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-readlines).
  sig do
    params(
        sep: String,
        limit: Integer,
        open_args: Integer,
    )
    .returns(T::Array[String])
  end
  def readlines(sep=T.unsafe(nil), limit=T.unsafe(nil), open_args=T.unsafe(nil)); end

  # Read symbolic link.
  #
  # See
  # [`File.readlink`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-readlink).
  sig {returns(T.self_type)}
  def readlink(); end

  # Returns the real (absolute) pathname of `self` in the actual filesystem.
  #
  # Does not contain symlinks or useless dots, `..` and `.`.
  #
  # The last component of the real pathname can be nonexistent.
  sig do
    params(
        p1: String,
    )
    .returns(Pathname)
  end
  def realdirpath(p1=T.unsafe(nil)); end

  # Returns the real (absolute) pathname for `self` in the actual filesystem.
  #
  # Does not contain symlinks or useless dots, `..` and `.`.
  #
  # All components of the pathname must exist when this method is called.
  sig do
    params(
        p1: String,
    )
    .returns(Pathname)
  end
  def realpath(p1=T.unsafe(nil)); end

  # The opposite of
  # [`Pathname#absolute?`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-absolute-3F)
  #
  # It returns `false` if the pathname begins with a slash.
  #
  # ```ruby
  # p = Pathname.new('/im/sure')
  # p.relative?
  #     #=> false
  #
  # p = Pathname.new('not/so/sure')
  # p.relative?
  #     #=> true
  # ```
  sig {returns(T::Boolean)}
  def relative?(); end

  # Returns a relative path from the given `base_directory` to the receiver.
  #
  # If `self` is absolute, then `base_directory` must be absolute too.
  #
  # If `self` is relative, then `base_directory` must be relative too.
  #
  # This method doesn't access the filesystem. It assumes no symlinks.
  #
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html) is
  # raised when it cannot find a relative path.
  sig do
    params(
        base_directory: T.any(String, Pathname),
    )
    .returns(Pathname)
  end
  def relative_path_from(base_directory); end

  # Rename the file.
  #
  # See
  # [`File.rename`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-rename).
  sig do
    params(
        p1: String,
    )
    .returns(Integer)
  end
  def rename(p1); end

  # Remove the referenced directory.
  #
  # See
  # [`Dir.rmdir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-rmdir).
  sig {returns(Integer)}
  def rmdir(); end

  # Recursively deletes a directory, including all directories beneath it.
  #
  # See
  # [`FileUtils.rm_r`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-rm_r)
  sig {returns(Integer)}
  def rmtree(); end

  # Predicate method for root directories. Returns `true` if the pathname
  # consists of consecutive slashes.
  #
  # It doesn't access the filesystem. So it may return `false` for some
  # pathnames which points to roots such as `/usr/..`.
  sig {returns(T::Boolean)}
  def root?(); end

  # See
  # [`FileTest.setgid?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-setgid-3F).
  sig {returns(T::Boolean)}
  def setgid?(); end

  # See
  # [`FileTest.setuid?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-setuid-3F).
  sig {returns(T::Boolean)}
  def setuid?(); end

  # See
  # [`FileTest.size`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-size).
  sig {returns(Integer)}
  def size(); end

  # See
  # [`FileTest.size?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-size-3F).
  sig {returns(T::Boolean)}
  def size?(); end

  # See
  # [`FileTest.socket?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-socket-3F).
  sig {returns(T::Boolean)}
  def socket?(); end

  # Returns the
  # [`dirname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-dirname)
  # and the
  # [`basename`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-basename)
  # in an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  #
  # See
  # [`File.split`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-split).
  sig {returns([Pathname, Pathname])}
  def split(); end

  # Returns a [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html)
  # object.
  #
  # See
  # [`File.stat`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-stat).
  sig {returns(File::Stat)}
  def stat(); end

  # See
  # [`FileTest.sticky?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-sticky-3F).
  sig {returns(T::Boolean)}
  def sticky?(); end

  # Return a pathname which is substituted by
  # [`String#sub`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-sub).
  #
  # ```ruby
  # path1 = Pathname.new('/usr/bin/perl')
  # path1.sub('perl', 'ruby')
  #     #=> #<Pathname:/usr/bin/ruby>
  # ```
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: T.any(String, T::Hash[T.untyped, T.untyped]),
    )
    .returns(Pathname)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(Pathname)
  end
  def sub(arg0, arg1=T.unsafe(nil), &blk); end

  # Return a pathname with `repl` added as a suffix to the basename.
  #
  # If self has no extension part, `repl` is appended.
  #
  # ```ruby
  # Pathname.new('/usr/bin/shutdown').sub_ext('.rb')
  #     #=> #<Pathname:/usr/bin/shutdown.rb>
  # ```
  sig do
    params(
        p1: String,
    )
    .returns(Pathname)
  end
  def sub_ext(p1); end

  # See
  # [`FileTest.symlink?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-symlink-3F).
  sig do
    params(
        old: String,
    )
    .returns(Integer)
  end
  sig {returns(T::Boolean)}
  def symlink?(old=T.unsafe(nil)); end

  # See
  # [`IO.sysopen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-sysopen).
  sig do
    params(
        mode: Integer,
        perm: Integer,
    )
    .returns(Integer)
  end
  def sysopen(mode=T.unsafe(nil), perm=T.unsafe(nil)); end

  # Returns pathname. This method is deprecated and will be removed in Ruby 3.2.
  sig {returns(T.self_type)}
  def taint(); end

  # Return the path as a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  # [`to_path`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-to_path)
  # is implemented so
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) objects are
  # usable with
  # [`File.open`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-open),
  # etc.
  sig {returns(String)}
  def to_path(); end

  # Truncates the file to `length` bytes.
  #
  # See
  # [`File.truncate`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-truncate).
  sig do
    params(
        length: Integer,
    )
    .returns(Integer)
  end
  def truncate(length); end

  # Removes a file or directory, using
  # [`File.unlink`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-unlink)
  # if `self` is a file, or
  # [`Dir.unlink`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-unlink)
  # as necessary.
  sig {returns(Integer)}
  def unlink(); end

  # Returns pathname. This method is deprecated and will be removed in Ruby 3.2.
  sig {returns(T.self_type)}
  def untaint(); end

  # Update the access and modification times of the file.
  #
  # See
  # [`File.utime`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-utime).
  sig do
    params(
        atime: Time,
        mtime: Time,
    )
    .returns(Integer)
  end
  def utime(atime, mtime); end

  # See
  # [`FileTest.world_readable?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-world_readable-3F).
  sig {returns(T::Boolean)}
  def world_readable?(); end

  # See
  # [`FileTest.world_writable?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-world_writable-3F).
  sig {returns(T::Boolean)}
  def world_writable?(); end

  # See
  # [`FileTest.writable?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-writable-3F).
  sig {returns(T::Boolean)}
  def writable?(); end

  # See
  # [`FileTest.writable_real?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-writable_real-3F).
  sig {returns(T::Boolean)}
  def writable_real?(); end

  # Writes `contents` to the file.
  #
  # See
  # [`File.write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-write).
  sig do
    params(
        arg0: String,
        offset: Integer,
        open_args: Integer,
    )
    .returns(Integer)
  end
  def write(arg0, offset=T.unsafe(nil), open_args=T.unsafe(nil)); end

  # See
  # [`FileTest.zero?`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html#method-i-zero-3F).
  sig {returns(T::Boolean)}
  def zero?(); end

  # Returns the current working directory as a
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html).
  #
  # ```ruby
  # Pathname.getwd
  #     #=> #<Pathname:/home/zzak/projects/ruby>
  # ```
  #
  # See
  # [`Dir.getwd`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-getwd).
  sig {returns(Pathname)}
  def self.pwd(); end

  # Alias for:
  # [`+`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-2B)
  sig do
    params(
        other: T.any(String, Pathname),
    )
    .returns(Pathname)
  end
  def /(other); end

  # Return the path as a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  # [`to_path`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-i-to_path)
  # is implemented so
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) objects are
  # usable with
  # [`File.open`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-open),
  # etc.
  sig {returns(String)}
  def to_s(); end
end

# RubyGems adds the
# [`gem`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-gem) method
# to allow activation of specific gem versions and overrides the
# [`require`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-require)
# method on [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) to make
# gems appear as if they live on the `$LOAD_PATH`. See the documentation of
# these methods for further detail.
# The [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) module is
# included by class [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html),
# so its methods are available in every Ruby object.
#
# The [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) instance
# methods are documented in class
# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) while the module
# methods are documented here. These methods are called without a receiver and
# thus can be called in functional form:
#
# ```ruby
# sprintf "%.1f", 1.234 #=> "1.2"
# ```
#
# fronzen-string-literal: true
module Kernel
  # Creates a new
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) object from
  # the given string, `path`, and returns pathname object.
  #
  # In order to use this constructor, you must first require the
  # [`Pathname`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html) standard
  # library extension.
  #
  # ```ruby
  # require 'pathname'
  # Pathname("/home/zzak")
  # #=> #<Pathname:/home/zzak>
  # ```
  #
  # See also
  # [`Pathname::new`](https://docs.ruby-lang.org/en/2.7.0/Pathname.html#method-c-new)
  # for more information.
  def Pathname(_); end
end
