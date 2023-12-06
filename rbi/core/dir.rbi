# typed: __STDLIB_INTERNAL

# Objects of class [`Dir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html) are
# directory streams representing directories in the underlying file system. They
# provide a variety of ways to list directories and their contents. See also
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html).
#
# The directory used in these examples contains the two regular files
# (`config.h` and `main.rb`), the parent directory (`..`), and the directory
# itself (`.`).
class Dir < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}

  # Changes the current working directory of the process to the given string.
  # When called without an argument, changes the directory to the value of the
  # environment variable `HOME`, or `LOGDIR`.
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # (probably Errno::ENOENT) if the target directory does not exist.
  #
  # If a block is given, it is passed the name of the new current directory, and
  # the block is executed with that as the current directory. The original
  # working directory is restored when the block exits. The return value of
  # `chdir` is the value of the block. `chdir` blocks can be nested, but in a
  # multi-threaded program an error will be raised if a thread attempts to open
  # a `chdir` block while another thread has one open.
  #
  # ```ruby
  # Dir.chdir("/var/spool/mail")
  # puts Dir.pwd
  # Dir.chdir("/tmp") do
  #   puts Dir.pwd
  #   Dir.chdir("/usr") do
  #     puts Dir.pwd
  #   end
  #   puts Dir.pwd
  # end
  # puts Dir.pwd
  # ```
  #
  # *produces:*
  #
  # ```
  # /var/spool/mail
  # /tmp
  # /usr
  # /tmp
  # /var/spool/mail
  # ```
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

  # Returns an array containing all of the filenames except for "." and ".." in
  # the given directory. Will raise a
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # if the named directory doesn't exist.
  #
  # The optional *encoding* keyword argument specifies the encoding of the
  # directory. If not specified, the filesystem encoding is used.
  #
  # ```ruby
  # Dir.children("testdir")   #=> ["config.h", "main.rb"]
  # ```
  def self.children(*_); end

  # Changes this process's idea of the file system root. Only a privileged
  # process may make this call. Not available on all platforms. On Unix systems,
  # see `chroot(2)` for more information.
  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def self.chroot(arg0); end

  # Deletes the named directory. Raises a subclass of
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # if the directory isn't empty.
  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def self.delete(arg0); end

  # Calls the block once for each entry except for "." and ".." in the named
  # directory, passing the filename of each entry as a parameter to the block.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # Dir.each_child("testdir") {|x| puts "Got #{x}" }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # Got config.h
  # Got main.rb
  # ```
  def self.each_child(*_, &blk); end

  # Returns `true` if the named file is an empty directory, `false` if it is not
  # a directory or non-empty.
  def self.empty?(_); end

  # Returns an array containing all of the filenames in the given directory.
  # Will raise a
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # if the named directory doesn't exist.
  #
  # The optional *encoding* keyword argument specifies the encoding of the
  # directory. If not specified, the filesystem encoding is used.
  #
  # ```ruby
  # Dir.entries("testdir")   #=> [".", "..", "config.h", "main.rb"]
  # ```
  sig do
    params(
        arg0: T.any(String, Pathname),
        arg1: Encoding,
    )
    .returns(T::Array[String])
  end
  def self.entries(arg0, arg1=T.unsafe(nil)); end

  # Returns `true` if the named file is a directory, `false` otherwise.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.exist?(file); end

  # Calls the block once for each entry in the named directory, passing the
  # filename of each entry as a parameter to the block.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # Dir.foreach("testdir") {|x| puts "Got #{x}" }
  # ```
  #
  # *produces:*
  #
  # ```
  # Got .
  # Got ..
  # Got config.h
  # Got main.rb
  # ```
  sig do
    params(
        dir: T.any(String, Pathname),
        arg0: Encoding,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(NilClass)
  end
  sig do
    params(
        dir: T.any(String, Pathname),
        arg0: Encoding,
    )
    .returns(T::Enumerator[String])
  end
  def self.foreach(dir, arg0=T.unsafe(nil), &blk); end

  # Returns the path to the current working directory of this process as a
  # string.
  #
  # ```ruby
  # Dir.chdir("/tmp")   #=> 0
  # Dir.getwd           #=> "/tmp"
  # Dir.pwd             #=> "/tmp"
  # ```
  sig {returns(String)}
  def self.getwd(); end

  # Expands `pattern`, which is a pattern string or an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of pattern
  # strings, and returns an array containing the matching filenames. If a block
  # is given, calls the block once for each matching filename, passing the
  # filename as a parameter to the block.
  #
  # The optional `base` keyword argument specifies the base directory for
  # interpreting relative pathnames instead of the current working directory. As
  # the results are not prefixed with the base directory name in this case, you
  # will need to prepend the base directory name if you want real paths.
  #
  # Note that the pattern is not a regexp, it's closer to a shell glob. See
  # [`File::fnmatch`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-fnmatch)
  # for the meaning of the `flags` parameter. Case sensitivity depends on your
  # system (File::FNM\_CASEFOLD is ignored), as does the order in which the
  # results are returned.
  #
  # `*`
  # :   Matches any file. Can be restricted by other values in the glob.
  #     Equivalent to `/ .* /mx` in regexp.
  #
  #     `*`
  # :       Matches all files
  #     `c*`
  # :       Matches all files beginning with `c`
  #     `*c`
  # :       Matches all files ending with `c`
  #     `*c*`
  # :       Match all files that have `c` in them (including at the beginning or
  #         end).
  #
  #
  #     Note, this will not match Unix-like hidden files (dotfiles). In order to
  #     include those in the match results, you must use the File::FNM\_DOTMATCH
  #     flag or something like `"{*,.*}"`.
  #
  # `**`
  # :   Matches directories recursively.
  #
  # `?`
  # :   Matches any one character. Equivalent to `/.{1}/` in regexp.
  #
  # `[set]`
  # :   Matches any one character in `set`. Behaves exactly like character sets
  #     in [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html),
  #     including set negation (`[^a-z]`).
  #
  # `{p,q}`
  # :   Matches either literal `p` or literal `q`. Equivalent to pattern
  #     alternation in regexp.
  #
  #     Matching literals may be more than one character in length. More than
  #     two literals may be specified.
  #
  # ` \\ `
  # :   Escapes the next metacharacter.
  #
  #     Note that this means you cannot use backslash on windows as part of a
  #     glob, i.e.  `Dir["c:\\foo*"]` will not work, use `Dir["c:/foo*"]`
  #     instead.
  #
  #
  # Examples:
  #
  # ```ruby
  # Dir["config.?"]                     #=> ["config.h"]
  # Dir.glob("config.?")                #=> ["config.h"]
  # Dir.glob("*.[a-z][a-z]")            #=> ["main.rb"]
  # Dir.glob("*.[^r]*")                 #=> ["config.h"]
  # Dir.glob("*.{rb,h}")                #=> ["main.rb", "config.h"]
  # Dir.glob("*")                       #=> ["config.h", "main.rb"]
  # Dir.glob("*", File::FNM_DOTMATCH)   #=> [".", "..", "config.h", "main.rb"]
  # Dir.glob(["*.rb", "*.h"])           #=> ["main.rb", "config.h"]
  #
  # rbfiles = File.join("**", "*.rb")
  # Dir.glob(rbfiles)                   #=> ["main.rb",
  #                                     #    "lib/song.rb",
  #                                     #    "lib/song/karaoke.rb"]
  #
  # Dir.glob(rbfiles, base: "lib")      #=> ["song.rb",
  #                                     #    "song/karaoke.rb"]
  #
  # libdirs = File.join("**", "lib")
  # Dir.glob(libdirs)                   #=> ["lib"]
  #
  # librbfiles = File.join("**", "lib", "**", "*.rb")
  # Dir.glob(librbfiles)                #=> ["lib/song.rb",
  #                                     #    "lib/song/karaoke.rb"]
  #
  # librbfiles = File.join("**", "lib", "*.rb")
  # Dir.glob(librbfiles)                #=> ["lib/song.rb"]
  # ```
  sig do
    params(
        pattern: T.any(String, T::Array[String]),
        flags: T.nilable(Integer),
        opts: T.nilable(T::Hash[Symbol, String]),
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        pattern: T.any(String, T::Array[String]),
        flags: T.nilable(Integer),
        opts: T.nilable(T::Hash[Symbol, String]),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(NilClass)
  end
  sig do
    params(
        pattern: T.any(String, T::Array[String]),
        opts: T.nilable(T::Hash[Symbol, String]),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(NilClass)
  end
  sig do
    params(
        pattern: T.any(String, T::Array[String]),
        opts: T.nilable(T::Hash[Symbol, String]),
    )
    .returns(T::Array[String])
  end
  def self.glob(pattern, flags=T.unsafe(nil), opts=T.unsafe(nil), &blk); end

  # Returns the home directory of the current user or the named user if given.
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def self.home(arg0=T.unsafe(nil)); end

  # Makes a new directory named by *string*, with permissions specified by the
  # optional parameter *anInteger*. The permissions may be modified by the value
  # of
  # [`File::umask`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-umask),
  # and are ignored on NT. Raises a
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # if the directory cannot be created. See also the discussion of permissions
  # in the class documentation for
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html).
  #
  # ```ruby
  # Dir.mkdir(File.join(Dir.home, ".foo"), 0700) #=> 0
  # ```
  sig do
    params(
        arg0: T.any(String, Pathname),
        arg1: Integer,
    )
    .returns(Integer)
  end
  def self.mkdir(arg0, arg1=T.unsafe(nil)); end

  # The optional *encoding* keyword argument specifies the encoding of the
  # directory. If not specified, the filesystem encoding is used.
  #
  # With no block, `open` is a synonym for
  # [`Dir::new`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-new). If
  # a block is present, it is passed *aDir* as a parameter. The directory is
  # closed at the end of the block, and
  # [`Dir::open`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-open)
  # returns the value of the block.
  sig do
    params(
        arg0: T.any(String, Pathname),
        arg1: Encoding,
    )
    .returns(Dir)
  end
  sig do
    type_parameters(:U).params(
        arg0: T.any(String, Pathname),
        arg1: Encoding,
        blk: T.proc.params(arg0: Dir).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def self.open(arg0, arg1=T.unsafe(nil), &blk); end

  # Returns the path to the current working directory of this process as a
  # string.
  #
  # ```ruby
  # Dir.chdir("/tmp")   #=> 0
  # Dir.getwd           #=> "/tmp"
  # Dir.pwd             #=> "/tmp"
  # ```
  sig {returns(String)}
  def self.pwd(); end

  # Deletes the named directory. Raises a subclass of
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # if the directory isn't empty.
  sig do
    params(
        arg0: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def self.rmdir(arg0); end

  # Returns the operating system's temporary file path.
  sig {returns(String)}
  def self.tmpdir; end

  # Deletes the named directory. Raises a subclass of
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # if the directory isn't empty.
  sig do
    params(
        arg0: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def self.unlink(arg0); end

  # Closes the directory stream. Calling this method on closed
  # [`Dir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html) object is ignored
  # since Ruby 2.3.
  #
  # ```ruby
  # d = Dir.new("testdir")
  # d.close   #=> nil
  # ```
  sig {returns(NilClass)}
  def close(); end

  # Calls the block once for each entry in this directory, passing the filename
  # of each entry as a parameter to the block.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # d = Dir.new("testdir")
  # d.each  {|x| puts "Got #{x}" }
  # ```
  #
  # *produces:*
  #
  # ```
  # Got .
  # Got ..
  # Got config.h
  # Got main.rb
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[String])}
  def each(&blk); end

  # Returns the file descriptor used in *dir*.
  #
  # ```ruby
  # d = Dir.new("..")
  # d.fileno   #=> 8
  # ```
  #
  # This method uses dirfd() function defined by POSIX 2008.
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html)
  # is raised on other platforms, such as Windows, which doesn't provide the
  # function.
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

  # Return a string describing this
  # [`Dir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html) object.
  sig {returns(String)}
  def inspect(); end

  # Returns the path parameter passed to *dir*'s constructor.
  #
  # ```ruby
  # d = Dir.new("..")
  # d.path   #=> ".."
  # ```
  sig {returns(T.nilable(String))}
  def path(); end

  # Returns the current position in *dir*. See also
  # [`Dir#seek`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-i-seek).
  #
  # ```ruby
  # d = Dir.new("testdir")
  # d.tell   #=> 0
  # d.read   #=> "."
  # d.tell   #=> 12
  # ```
  sig {returns(Integer)}
  def pos(); end

  # Synonym for
  # [`Dir#seek`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-i-seek),
  # but returns the position parameter.
  #
  # ```ruby
  # d = Dir.new("testdir")   #=> #<Dir:0x401b3c40>
  # d.read                   #=> "."
  # i = d.pos                #=> 12
  # d.read                   #=> ".."
  # d.pos = i                #=> 12
  # d.read                   #=> ".."
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def pos=(arg0); end

  # Reads the next entry from *dir* and returns it as a string. Returns `nil` at
  # the end of the stream.
  #
  # ```ruby
  # d = Dir.new("testdir")
  # d.read   #=> "."
  # d.read   #=> ".."
  # d.read   #=> "config.h"
  # ```
  sig {returns(T.nilable(String))}
  def read(); end

  # Repositions *dir* to the first entry.
  #
  # ```ruby
  # d = Dir.new("testdir")
  # d.read     #=> "."
  # d.rewind   #=> #<Dir:0x401b3fb0>
  # d.read     #=> "."
  # ```
  sig {returns(T.self_type)}
  def rewind(); end

  # Seeks to a particular location in *dir*. *integer* must be a value returned
  # by [`Dir#tell`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-i-tell).
  #
  # ```ruby
  # d = Dir.new("testdir")   #=> #<Dir:0x401b3c40>
  # d.read                   #=> "."
  # i = d.tell               #=> 12
  # d.read                   #=> ".."
  # d.seek(i)                #=> #<Dir:0x401b3c40>
  # d.read                   #=> ".."
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(T.self_type)
  end
  def seek(arg0); end

  # Returns the current position in *dir*. See also
  # [`Dir#seek`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-i-seek).
  #
  # ```ruby
  # d = Dir.new("testdir")
  # d.tell   #=> 0
  # d.read   #=> "."
  # d.tell   #=> 12
  # ```
  sig {returns(Integer)}
  def tell(); end

  # Returns the path parameter passed to *dir*'s constructor.
  #
  # ```ruby
  # d = Dir.new("..")
  # d.path   #=> ".."
  # ```
  sig {returns(T.nilable(String))}
  def to_path(); end

  # Equivalent to calling `Dir.glob([string,...], 0)`.
  sig do
    params(
        pattern: T.any(String, Pathname),
        base: T.nilable(T.any(String, Pathname)),
        sort: T::Boolean,
        blk: T.nilable(T.proc.params(arg0: String).returns(BasicObject))
    )
    .returns(T::Array[String])
  end
  def self.[](*pattern, base: nil, sort: true, &blk); end
end
