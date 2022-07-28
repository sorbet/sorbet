# typed: __STDLIB_INTERNAL

# A [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) is an abstraction of
# any file object accessible by the program and is closely associated with class
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) includes the methods
# of module [`FileTest`](https://docs.ruby-lang.org/en/2.7.0/FileTest.html) as
# class methods, allowing you to write (for example) `File.exist?("foo")`.
#
# In the description of [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)
# methods, *permission bits* are a platform-specific set of bits that indicate
# permissions of a file. On Unix-based systems, permissions are viewed as a set
# of three octets, for the owner, the group, and the rest of the world. For each
# of these entities, permissions may be set to read, write, or execute the file:
#
# The permission bits `0644` (in octal) would thus be interpreted as read/write
# for owner, and read-only for group and other. Higher-order bits may also be
# used to indicate the type of file (plain, directory, pipe, socket, and so on)
# and various other special features. If the permissions are for a directory,
# the meaning of the execute bit changes; when set the directory can be
# searched.
#
# On non-Posix operating systems, there may be only the ability to make a file
# read-only or read-write. In this case, the remaining permission bits will be
# synthesized to resemble typical values. For instance, on Windows NT the
# default permission bits are `0644`, which means read/write for owner,
# read-only for all others. The only change that can be made is to make the file
# read-only, which is reported as `0444`.
#
# Various constants for the methods in
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) can be found in
# File::Constants.
class File < IO
  # platform specific alternative separator
  ALT_SEPARATOR = T.let(nil, T.nilable(String))
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
  # path list separator
  PATH_SEPARATOR = T.let(T.unsafe(nil), String)
  RDONLY = T.let(T.unsafe(nil), Integer)
  RDWR = T.let(T.unsafe(nil), Integer)
  RSYNC = T.let(T.unsafe(nil), Integer)
  SEEK_CUR = T.let(T.unsafe(nil), Integer)
  SEEK_DATA = T.let(T.unsafe(nil), Integer)
  SEEK_END = T.let(T.unsafe(nil), Integer)
  SEEK_HOLE = T.let(T.unsafe(nil), Integer)
  SEEK_SET = T.let(T.unsafe(nil), Integer)
  # separates directory parts in path
  SEPARATOR = T.let(T.unsafe(nil), String)
  SHARE_DELETE = T.let(T.unsafe(nil), Integer)
  SYNC = T.let(T.unsafe(nil), Integer)
  TMPFILE = T.let(T.unsafe(nil), Integer)
  TRUNC = T.let(T.unsafe(nil), Integer)
  WRONLY = T.let(T.unsafe(nil), Integer)

  # separates directory parts in path
  Separator = T.let(T.unsafe(nil), String)

  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}

  # Converts a pathname to an absolute pathname. Relative paths are referenced
  # from the current working directory of the process unless *dir\_string* is
  # given, in which case it will be used as the starting point. If the given
  # pathname starts with a "`~`" it is NOT expanded, it is treated as a normal
  # directory name.
  #
  # ```ruby
  # File.absolute_path("~oracle/bin")       #=> "<relative_path>/~oracle/bin"
  # ```
  sig do
    params(
        file: T.any(String, Pathname),
        dir: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.absolute_path(file, dir=T.unsafe(nil)); end

  # Returns the last access time for the named file as a
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) object.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  #
  # ```ruby
  # File.atime("testfile")   #=> Wed Apr 09 08:51:48 CDT 2003
  # ```
  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.atime(file); end

  # Returns the last component of the filename given in *file\_name* (after
  # first stripping trailing separators), which can be formed using both
  # [`File::SEPARATOR`](https://docs.ruby-lang.org/en/2.7.0/File.html#SEPARATOR)
  # and
  # [`File::ALT_SEPARATOR`](https://docs.ruby-lang.org/en/2.7.0/File.html#ALT_SEPARATOR)
  # as the separator when
  # [`File::ALT_SEPARATOR`](https://docs.ruby-lang.org/en/2.7.0/File.html#ALT_SEPARATOR)
  # is not `nil`. If *suffix* is given and present at the end of *file\_name*,
  # it is removed. If *suffix* is ".\*", any extension will be removed.
  #
  # ```ruby
  # File.basename("/home/gumby/work/ruby.rb")          #=> "ruby.rb"
  # File.basename("/home/gumby/work/ruby.rb", ".rb")   #=> "ruby"
  # File.basename("/home/gumby/work/ruby.rb", ".*")    #=> "ruby"
  # ```
  sig do
    params(
        file: T.any(String, Pathname),
        suffix: String,
    )
    .returns(String)
  end
  def self.basename(file, suffix=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(String, Pathname),
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(String, Pathname),
        arg1: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(String, Pathname),
        arg1: Integer,
        arg2: Integer,
    )
    .returns(String)
  end
  def self.binread(arg0, arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  # Returns the birth time for the named file.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html)
  # object.
  #
  # ```ruby
  # File.birthtime("testfile")   #=> Wed Apr 09 08:53:13 CDT 2003
  # ```
  #
  # If the platform doesn't have birthtime, raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html).
  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.birthtime(file); end

  # Returns `true` if the named file is a block device.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.blockdev?(file); end

  # Returns `true` if the named file is a character device.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.chardev?(file); end

  # Changes permission bits on the named file(s) to the bit pattern represented
  # by *mode\_int*. Actual effects are operating system dependent (see the
  # beginning of this section). On Unix systems, see `chmod(2)` for details.
  # Returns the number of files processed.
  #
  # ```ruby
  # File.chmod(0644, "testfile", "out")   #=> 2
  # ```
  sig do
    params(
        mode: Integer,
        files: String,
    )
    .returns(Integer)
  end
  def self.chmod(mode, *files); end

  # Changes the owner and group of the named file(s) to the given numeric owner
  # and group id's. Only a process with superuser privileges may change the
  # owner of a file. The current owner of a file may change the file's group to
  # any group to which the owner belongs. A `nil` or -1 owner or group id is
  # ignored. Returns the number of files processed.
  #
  # ```ruby
  # File.chown(nil, 100, "testfile")
  # ```
  sig do
    params(
        owner: T.nilable(Integer),
        group: T.nilable(Integer),
        files: String,
    )
    .returns(Integer)
  end
  def self.chown(owner, group, *files); end

  # Returns the change time for the named file (the time at which directory
  # information about the file was changed, not the file itself).
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  #
  # Note that on Windows (NTFS), returns creation time (birth time).
  #
  # ```ruby
  # File.ctime("testfile")   #=> Wed Apr 09 08:53:13 CDT 2003
  # ```
  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.ctime(file); end

  # Deletes the named files, returning the number of names passed as arguments.
  # Raises an exception on any error. Since the underlying implementation relies
  # on the `unlink(2)` system call, the type of exception raised depends on its
  # error type (see https://linux.die.net/man/2/unlink) and has the form of e.g.
  # Errno::ENOENT.
  #
  # See also
  # [`Dir::rmdir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-rmdir).
  sig do
    params(
        files: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def self.delete(*files); end

  # Returns `true` if the named file is a directory, or a symlink that points at
  # a directory, and `false` otherwise.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  #
  # ```ruby
  # File.directory?(".")
  # ```
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.directory?(file); end

  # Returns all components of the filename given in *file\_name* except the last
  # one (after first stripping trailing separators). The filename can be formed
  # using both
  # [`File::SEPARATOR`](https://docs.ruby-lang.org/en/2.7.0/File.html#SEPARATOR)
  # and
  # [`File::ALT_SEPARATOR`](https://docs.ruby-lang.org/en/2.7.0/File.html#ALT_SEPARATOR)
  # as the separator when
  # [`File::ALT_SEPARATOR`](https://docs.ruby-lang.org/en/2.7.0/File.html#ALT_SEPARATOR)
  # is not `nil`.
  #
  # ```ruby
  # File.dirname("/home/gumby/work/ruby.rb")   #=> "/home/gumby/work"
  # ```
  sig do
    params(
        file: T.any(String, Pathname),
        level: Integer,
    )
    .returns(String)
  end
  def self.dirname(file, level = 1); end

  # Returns `true` if the named file exists and has a zero size.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  def self.empty?(_); end

  # Returns `true` if the named file is executable by the effective user and
  # group id of this process. See eaccess(3).
  #
  # Windows does not support execute permissions separately from read
  # permissions. On Windows, a file is only considered executable if it ends in
  # .bat, .cmd, .com, or .exe.
  #
  # Note that some OS-level security features may cause this to return true even
  # though the file is not executable by the effective user/group.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.executable?(file); end

  # Returns `true` if the named file is executable by the real user and group id
  # of this process. See access(3).
  #
  # Windows does not support execute permissions separately from read
  # permissions. On Windows, a file is only considered executable if it ends in
  # .bat, .cmd, .com, or .exe.
  #
  # Note that some OS-level security features may cause this to return true even
  # though the file is not executable by the real user/group.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.executable_real?(file); end

  # Return `true` if the named file exists.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  #
  # "file exists" means that stat() or fstat() system call is successful.
  sig do
    params(
        file: BasicObject,
    )
    .returns(T::Boolean)
  end
  def self.exist?(file); end

  # Converts a pathname to an absolute pathname. Relative paths are referenced
  # from the current working directory of the process unless `dir_string` is
  # given, in which case it will be used as the starting point. The given
  # pathname may start with a "`~`", which expands to the process owner's home
  # directory (the environment variable `HOME` must be set correctly).
  # "`~`*user*" expands to the named user's home directory.
  #
  # ```ruby
  # File.expand_path("~oracle/bin")           #=> "/home/oracle/bin"
  # ```
  #
  # A simple example of using `dir_string` is as follows.
  #
  # ```ruby
  # File.expand_path("ruby", "/usr/bin")      #=> "/usr/bin/ruby"
  # ```
  #
  # A more complex example which also resolves parent directory is as follows.
  # Suppose we are in bin/mygem and want the absolute path of lib/mygem.rb.
  #
  # ```ruby
  # File.expand_path("../../lib/mygem.rb", __FILE__)
  # #=> ".../path/to/project/lib/mygem.rb"
  # ```
  #
  # So first it resolves the parent of \_\_FILE\_\_, that is bin/, then go to
  # the parent, the root of the project and appends `lib/mygem.rb`.
  sig do
    params(
        file: BasicObject,
        dir: BasicObject,
    )
    .returns(String)
  end
  def self.expand_path(file, dir=T.unsafe(nil)); end

  # Returns the extension (the portion of file name in `path` starting from the
  # last period).
  #
  # If `path` is a dotfile, or starts with a period, then the starting dot is
  # not dealt with the start of the extension.
  #
  # An empty string will also be returned when the period is the last character
  # in `path`.
  #
  # On Windows, trailing dots are truncated.
  #
  # ```ruby
  # File.extname("test.rb")         #=> ".rb"
  # File.extname("a/b/d/test.rb")   #=> ".rb"
  # File.extname(".a/b/d/test.rb")  #=> ".rb"
  # File.extname("foo.")            #=> "" on Windows
  # File.extname("foo.")            #=> "." on non-Windows
  # File.extname("test")            #=> ""
  # File.extname(".profile")        #=> ""
  # File.extname(".profile.sh")     #=> ".sh"
  # ```
  sig do
    params(
        path: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.extname(path); end

  # Returns `true` if the named `file` exists and is a regular file.
  #
  # `file` can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  #
  # If the `file` argument is a symbolic link, it will resolve the symbolic link
  # and use the file referenced by the link.
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.file?(file); end

  # Returns true if `path` matches against `pattern`. The pattern is not a
  # regular expression; instead it follows rules similar to shell filename
  # globbing. It may contain the following metacharacters:
  #
  # `*`
  # :   Matches any file. Can be restricted by other values in the glob.
  #     Equivalent to `/ .* /x` in regexp.
  #
  #     `*`
  # :       Matches all files regular files
  #     `c*`
  # :       Matches all files beginning with `c`
  #     `*c`
  # :       Matches all files ending with `c`
  #     `*c*`
  # :       Matches all files that have `c` in them (including at the beginning
  #         or end).
  #
  #
  #     To match hidden files (that start with a `.` set the File::FNM\_DOTMATCH
  #     flag.
  #
  # `**`
  # :   Matches directories recursively or files expansively.
  #
  # `?`
  # :   Matches any one character. Equivalent to `/.{1}/` in regexp.
  #
  # `[set]`
  # :   Matches any one character in `set`. Behaves exactly like character sets
  #     in [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html),
  #     including set negation (`[^a-z]`).
  #
  # ` \ `
  # :   Escapes the next metacharacter.
  #
  # `{a,b}`
  # :   Matches pattern a and pattern b if File::FNM\_EXTGLOB flag is enabled.
  #     Behaves like a
  #     [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) union
  #     (`(?:a|b)`).
  #
  #
  # `flags` is a bitwise OR of the `FNM_XXX` constants. The same glob pattern
  # and flags are used by
  # [`Dir::glob`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-glob).
  #
  # Examples:
  #
  # ```ruby
  # File.fnmatch('cat',       'cat')        #=> true  # match entire string
  # File.fnmatch('cat',       'category')   #=> false # only match partial string
  #
  # File.fnmatch('c{at,ub}s', 'cats')                    #=> false # { } isn't supported by default
  # File.fnmatch('c{at,ub}s', 'cats', File::FNM_EXTGLOB) #=> true  # { } is supported on FNM_EXTGLOB
  #
  # File.fnmatch('c?t',     'cat')          #=> true  # '?' match only 1 character
  # File.fnmatch('c??t',    'cat')          #=> false # ditto
  # File.fnmatch('c*',      'cats')         #=> true  # '*' match 0 or more characters
  # File.fnmatch('c*t',     'c/a/b/t')      #=> true  # ditto
  # File.fnmatch('ca[a-z]', 'cat')          #=> true  # inclusive bracket expression
  # File.fnmatch('ca[^t]',  'cat')          #=> false # exclusive bracket expression ('^' or '!')
  #
  # File.fnmatch('cat', 'CAT')                     #=> false # case sensitive
  # File.fnmatch('cat', 'CAT', File::FNM_CASEFOLD) #=> true  # case insensitive
  # File.fnmatch('cat', 'CAT', File::FNM_SYSCASE)  #=> true or false # depends on the system default
  #
  # File.fnmatch('?',   '/', File::FNM_PATHNAME)  #=> false # wildcard doesn't match '/' on FNM_PATHNAME
  # File.fnmatch('*',   '/', File::FNM_PATHNAME)  #=> false # ditto
  # File.fnmatch('[/]', '/', File::FNM_PATHNAME)  #=> false # ditto
  #
  # File.fnmatch('\?',   '?')                       #=> true  # escaped wildcard becomes ordinary
  # File.fnmatch('\a',   'a')                       #=> true  # escaped ordinary remains ordinary
  # File.fnmatch('\a',   '\a', File::FNM_NOESCAPE)  #=> true  # FNM_NOESCAPE makes '\' ordinary
  # File.fnmatch('[\?]', '?')                       #=> true  # can escape inside bracket expression
  #
  # File.fnmatch('*',   '.profile')                      #=> false # wildcard doesn't match leading
  # File.fnmatch('*',   '.profile', File::FNM_DOTMATCH)  #=> true  # period by default.
  # File.fnmatch('.*',  '.profile')                      #=> true
  #
  # rbfiles = '**' '/' '*.rb' # you don't have to do like this. just write in single string.
  # File.fnmatch(rbfiles, 'main.rb')                    #=> false
  # File.fnmatch(rbfiles, './main.rb')                  #=> false
  # File.fnmatch(rbfiles, 'lib/song.rb')                #=> true
  # File.fnmatch('**.rb', 'main.rb')                    #=> true
  # File.fnmatch('**.rb', './main.rb')                  #=> false
  # File.fnmatch('**.rb', 'lib/song.rb')                #=> true
  # File.fnmatch('*',           'dave/.profile')                      #=> true
  #
  # pattern = '*' '/' '*'
  # File.fnmatch(pattern, 'dave/.profile', File::FNM_PATHNAME)  #=> false
  # File.fnmatch(pattern, 'dave/.profile', File::FNM_PATHNAME | File::FNM_DOTMATCH) #=> true
  #
  # pattern = '**' '/' 'foo'
  # File.fnmatch(pattern, 'a/b/c/foo', File::FNM_PATHNAME)     #=> true
  # File.fnmatch(pattern, '/a/b/c/foo', File::FNM_PATHNAME)    #=> true
  # File.fnmatch(pattern, 'c:/a/b/c/foo', File::FNM_PATHNAME)  #=> true
  # File.fnmatch(pattern, 'a/.b/c/foo', File::FNM_PATHNAME)    #=> false
  # File.fnmatch(pattern, 'a/.b/c/foo', File::FNM_PATHNAME | File::FNM_DOTMATCH) #=> true
  # ```
  sig do
    params(
        pattern: String,
        path: T.any(String, Pathname),
        flags: Integer,
    )
    .returns(T::Boolean)
  end
  def self.fnmatch(pattern, path, flags=T.unsafe(nil)); end

  # Identifies the type of the named file; the return string is one of "`file`",
  # "`directory`", "`characterSpecial`", "`blockSpecial`", "`fifo`", "`link`",
  # "`socket`", or "`unknown`".
  #
  # ```ruby
  # File.ftype("testfile")            #=> "file"
  # File.ftype("/dev/tty")            #=> "characterSpecial"
  # File.ftype("/tmp/.X11-unix/X0")   #=> "socket"
  # ```
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.ftype(file); end

  # Returns `true` if the named file exists and the effective group id of the
  # calling process is the owner of the file. Returns `false` on Windows.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.grpowned?(file); end

  # Returns `true` if the named files are identical.
  #
  # *file\_1* and *file\_2* can be an
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  #
  # ```ruby
  # open("a", "w") {}
  # p File.identical?("a", "a")      #=> true
  # p File.identical?("a", "./a")    #=> true
  # File.link("a", "b")
  # p File.identical?("a", "b")      #=> true
  # File.symlink("a", "c")
  # p File.identical?("a", "c")      #=> true
  # open("d", "w") {}
  # p File.identical?("a", "d")      #=> false
  # ```
  sig do
    params(
        file_1: T.any(String, Pathname, IO),
        file_2: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.identical?(file_1, file_2); end

  # Returns a new string formed by joining the strings using `"/"`.
  #
  # ```ruby
  # File.join("usr", "mail", "gumby")   #=> "usr/mail/gumby"
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(String)
  end
  def self.join(*arg0); end

  # Equivalent to
  # [`File::chmod`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-chmod),
  # but does not follow symbolic links (so it will change the permissions
  # associated with the link, not the file referenced by the link). Often not
  # available.
  sig do
    params(
        mode: Integer,
        files: String,
    )
    .returns(Integer)
  end
  def self.lchmod(mode, *files); end

  # Equivalent to
  # [`File::chown`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-chown),
  # but does not follow symbolic links (so it will change the owner associated
  # with the link, not the file referenced by the link). Often not available.
  # Returns number of files in the argument list.
  sig do
    params(
        owner: T.nilable(Integer),
        group: T.nilable(Integer),
        files: String,
    )
    .returns(Integer)
  end
  def self.lchown(owner, group, *files); end

  # Creates a new name for an existing file using a hard link. Will not
  # overwrite *new\_name* if it already exists (raising a subclass of
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)).
  # Not available on all platforms.
  #
  # ```ruby
  # File.link("testfile", ".testfile")   #=> 0
  # IO.readlines(".testfile")[0]         #=> "This is line one\n"
  # ```
  sig do
    params(
        old: T.any(String, Pathname),
        new: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def self.link(old, new); end

  # Same as
  # [`File::stat`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-stat),
  # but does not follow the last symbolic link. Instead, reports on the link
  # itself.
  #
  # ```ruby
  # File.symlink("testfile", "link2test")   #=> 0
  # File.stat("testfile").size              #=> 66
  # File.lstat("link2test").size            #=> 8
  # File.stat("link2test").size             #=> 66
  # ```
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(File::Stat)
  end
  def self.lstat(file); end

  # Sets the access and modification times of each named file to the first two
  # arguments. If a file is a symlink, this method acts upon the link itself as
  # opposed to its referent; for the inverse behavior, see
  # [`File.utime`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-utime).
  # Returns the number of file names in the argument list.
  def self.lutime(*_); end

  # Creates a FIFO special file with name *file\_name*. *mode* specifies the
  # FIFO's permissions. It is modified by the process's umask in the usual way:
  # the permissions of the created file are (mode & ~umask).
  def self.mkfifo(*_); end

  # Returns the modification time for the named file as a
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) object.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  #
  # ```ruby
  # File.mtime("testfile")   #=> Tue Apr 08 12:58:04 CDT 2003
  # ```
  sig do
    params(
        file: BasicObject,
    )
    .returns(Time)
  end
  def self.mtime(file); end

  # With no associated block,
  # [`File.open`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-open)
  # is a synonym for
  # [`File.new`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-new). If
  # the optional code block is given, it will be passed the opened `file` as an
  # argument and the [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)
  # object will automatically be closed when the block terminates. The value of
  # the block will be returned from
  # [`File.open`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-open).
  #
  # If a file is being created, its initial permissions may be set using the
  # `perm` parameter. See
  # [`File.new`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-new) for
  # further discussion.
  #
  # See [`IO.new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new) for
  # a description of the `mode` and `opt` parameters.
  sig do
    params(
      filename: T.any(String, Pathname),
      mode: T.any(Integer, String),
      perm: T.nilable(Integer),
      opt: T.untyped,
    ).returns(File)
  end
  sig do
    type_parameters(:U).params(
      filename: T.any(String, Pathname),
      mode: T.any(Integer, String),
      perm: T.nilable(Integer),
      opt: T.untyped,
      blk: T.proc.params(file: File).returns(T.type_parameter(:U))
    ).returns(T.type_parameter(:U))
  end
  def self.open(filename, mode='r', perm=nil, **opt, &blk); end

  # Returns `true` if the named file exists and the effective used id of the
  # calling process is the owner of the file.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.owned?(file); end

  # Returns the string representation of the path
  #
  # ```ruby
  # File.path("/dev/null")          #=> "/dev/null"
  # File.path(Pathname.new("/tmp")) #=> "/tmp"
  # ```
  sig do
    params(
        path: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.path(path); end

  # Returns `true` if the named file is a pipe.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.pipe?(file); end

  # Returns `true` if the named file is readable by the effective user and group
  # id of this process. See eaccess(3).
  #
  # Note that some OS-level security features may cause this to return true even
  # though the file is not readable by the effective user/group.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.readable?(file); end

  # Returns `true` if the named file is readable by the real user and group id
  # of this process. See access(3).
  #
  # Note that some OS-level security features may cause this to return true even
  # though the file is not readable by the real user/group.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.readable_real?(file); end

  # Returns the name of the file referenced by the given link. Not available on
  # all platforms.
  #
  # ```ruby
  # File.symlink("testfile", "link2test")   #=> 0
  # File.readlink("link2test")              #=> "testfile"
  # ```
  sig do
    params(
        link: String,
    )
    .returns(String)
  end
  def self.readlink(link); end

  # Returns the real (absolute) pathname of *pathname* in the actual filesystem.
  # The real pathname doesn't contain symlinks or useless dots.
  #
  # If *dir\_string* is given, it is used as a base directory for interpreting
  # relative pathname instead of the current directory.
  #
  # The last component of the real pathname can be nonexistent.
  sig do
    params(
        pathname: T.any(String, Pathname),
        dir: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.realdirpath(pathname, dir=T.unsafe(nil)); end

  # Returns the real (absolute) pathname of *pathname* in the actual filesystem
  # not containing symlinks or useless dots.
  #
  # If *dir\_string* is given, it is used as a base directory for interpreting
  # relative pathname instead of the current directory.
  #
  # All components of the pathname must exist when this method is called.
  sig do
    params(
        pathname: T.any(String, Pathname),
        dir: T.any(String, Pathname),
    )
    .returns(String)
  end
  def self.realpath(pathname, dir=T.unsafe(nil)); end

  # Renames the given file to the new name. Raises a
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # if the file cannot be renamed.
  #
  # ```ruby
  # File.rename("afile", "afile.bak")   #=> 0
  # ```
  sig do
    params(
        old: T.any(String, Pathname),
        new: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def self.rename(old, new); end

  # Returns `true` if the named file has the setgid bit set.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.setgid?(file); end

  # Returns `true` if the named file has the setuid bit set.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.setuid?(file); end

  # Returns the size of `file_name`.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(Integer)
  end
  def self.size(file); end

  # Returns `nil` if `file_name` doesn't exist or has zero size, the size of the
  # file otherwise.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T.nilable(Integer))
  end
  def self.size?(file); end

  # Returns `true` if the named file is a socket.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.socket?(file); end

  # Splits the given string into a directory and a file component and returns
  # them in a two-element array. See also
  # [`File::dirname`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-dirname)
  # and
  # [`File::basename`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-basename).
  #
  # ```ruby
  # File.split("/home/gumby/.profile")   #=> ["/home/gumby", ".profile"]
  # ```
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns([String, String])
  end
  def self.split(file); end

  # Returns a [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html)
  # object for the named file (see
  # [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html)).
  #
  # ```ruby
  # File.stat("testfile").mtime   #=> Tue Apr 08 12:58:04 CDT 2003
  # ```
  sig do
    params(
        file: BasicObject,
    )
    .returns(File::Stat)
  end
  def self.stat(file); end

  # Returns `true` if the named file has the sticky bit set.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.sticky?(file); end

  # Creates a symbolic link called *new\_name* for the existing file
  # *old\_name*. Raises a NotImplemented exception on platforms that do not
  # support symbolic links.
  #
  # ```ruby
  # File.symlink("testfile", "link2test")   #=> 0
  # ```
  sig do
    params(
        old: T.any(String, Pathname),
        new: T.any(String, Pathname),
    )
    .returns(Integer)
  end
  def self.symlink(old, new); end

  # Returns `true` if the named file is a symbolic link.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T::Boolean)
  end
  def self.symlink?(file); end

  # Truncates the file *file\_name* to be at most *integer* bytes long. Not
  # available on all platforms.
  #
  # ```ruby
  # f = File.new("out", "w")
  # f.write("1234567890")     #=> 10
  # f.close                   #=> nil
  # File.truncate("out", 5)   #=> 0
  # File.size("out")          #=> 5
  # ```
  sig do
    params(
        file: T.any(String, Pathname),
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.truncate(file, arg0); end

  # Returns the current umask value for this process. If the optional argument
  # is given, set the umask to that value and return the previous value. Umask
  # values are *subtracted* from the default permissions, so a umask of `0222`
  # would make a file read-only for everyone.
  #
  # ```ruby
  # File.umask(0006)   #=> 18
  # File.umask         #=> 6
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def self.umask(arg0=T.unsafe(nil)); end

  # Sets the access and modification times of each named file to the first two
  # arguments. If a file is a symlink, this method acts upon its referent rather
  # than the link itself; for the inverse behavior see
  # [`File.lutime`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-lutime).
  # Returns the number of file names in the argument list.
  sig do
    params(
        atime: Time,
        mtime: Time,
        files: String,
    )
    .returns(Integer)
  end
  def self.utime(atime, mtime, *files); end

  # If *file\_name* is readable by others, returns an integer representing the
  # file permission bits of *file\_name*. Returns `nil` otherwise. The meaning
  # of the bits is platform dependent; on Unix systems, see `stat(2)`.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  #
  # ```ruby
  # File.world_readable?("/etc/passwd")           #=> 420
  # m = File.world_readable?("/etc/passwd")
  # sprintf("%o", m)                              #=> "644"
  # ```
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T.nilable(Integer))
  end
  def self.world_readable?(file); end

  # If *file\_name* is writable by others, returns an integer representing the
  # file permission bits of *file\_name*. Returns `nil` otherwise. The meaning
  # of the bits is platform dependent; on Unix systems, see `stat(2)`.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  #
  # ```ruby
  # File.world_writable?("/tmp")                  #=> 511
  # m = File.world_writable?("/tmp")
  # sprintf("%o", m)                              #=> "777"
  # ```
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T.nilable(Integer))
  end
  def self.world_writable?(file); end

  # Returns `true` if the named file is writable by the effective user and group
  # id of this process. See eaccess(3).
  #
  # Note that some OS-level security features may cause this to return true even
  # though the file is not writable by the effective user/group.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T.nilable(Integer))
  end
  def self.writable?(file); end

  # Returns `true` if the named file is writable by the real user and group id
  # of this process. See access(3).
  #
  # Note that some OS-level security features may cause this to return true even
  # though the file is not writable by the real user/group.
  sig do
    params(
        file: T.any(String, Pathname),
    )
    .returns(T.nilable(Integer))
  end
  def self.writable_real?(file); end

  # Returns `true` if the named file exists and has a zero size.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  sig do
    params(
        file: T.any(String, Pathname, IO),
    )
    .returns(T::Boolean)
  end
  def self.zero?(file); end

  # Returns the last access time (a
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) object) for *file*,
  # or epoch if *file* has not been accessed.
  #
  # ```ruby
  # File.new("testfile").atime   #=> Wed Dec 31 18:00:00 CST 1969
  # ```
  sig {returns(Time)}
  def atime(); end

  # Returns the birth time for *file*.
  #
  # ```ruby
  # File.new("testfile").birthtime   #=> Wed Apr 09 08:53:14 CDT 2003
  # ```
  #
  # If the platform doesn't have birthtime, raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html).
  sig {returns(Time)}
  def birthtime(); end

  # Changes permission bits on *file* to the bit pattern represented by
  # *mode\_int*. Actual effects are platform dependent; on Unix systems, see
  # `chmod(2)` for details. Follows symbolic links. Also see File#lchmod.
  #
  # ```ruby
  # f = File.new("out", "w");
  # f.chmod(0644)   #=> 0
  # ```
  sig do
    params(
        mode: Integer,
    )
    .returns(Integer)
  end
  def chmod(mode); end

  # Changes the owner and group of *file* to the given numeric owner and group
  # id's. Only a process with superuser privileges may change the owner of a
  # file. The current owner of a file may change the file's group to any group
  # to which the owner belongs. A `nil` or -1 owner or group id is ignored.
  # Follows symbolic links. See also File#lchown.
  #
  # ```ruby
  # File.new("testfile").chown(502, 1000)
  # ```
  sig do
    params(
        owner: T.nilable(Integer),
        group: T.nilable(Integer),
    )
    .returns(Integer)
  end
  def chown(owner, group); end

  # Returns the change time for *file* (that is, the time directory information
  # about the file was changed, not the file itself).
  #
  # Note that on Windows (NTFS), returns creation time (birth time).
  #
  # ```ruby
  # File.new("testfile").ctime   #=> Wed Apr 09 08:53:14 CDT 2003
  # ```
  sig {returns(Time)}
  def ctime(); end

  # Locks or unlocks a file according to *locking\_constant* (a logical *or* of
  # the values in the table below). Returns `false` if File::LOCK\_NB is
  # specified and the operation would otherwise have blocked. Not available on
  # all platforms.
  #
  # Locking constants (in class
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)):
  #
  # ```
  # LOCK_EX   | Exclusive lock. Only one process may hold an
  #           | exclusive lock for a given file at a time.
  # ----------+------------------------------------------------
  # LOCK_NB   | Don't block when locking. May be combined
  #           | with other lock options using logical or.
  # ----------+------------------------------------------------
  # LOCK_SH   | Shared lock. Multiple processes may each hold a
  #           | shared lock for a given file at the same time.
  # ----------+------------------------------------------------
  # LOCK_UN   | Unlock.
  # ```
  #
  # Example:
  #
  # ```ruby
  # # update a counter using write lock
  # # don't use "w" because it truncates the file before lock.
  # File.open("counter", File::RDWR|File::CREAT, 0644) {|f|
  #   f.flock(File::LOCK_EX)
  #   value = f.read.to_i + 1
  #   f.rewind
  #   f.write("#{value}\n")
  #   f.flush
  #   f.truncate(f.pos)
  # }
  #
  # # read the counter using read lock
  # File.open("counter", "r") {|f|
  #   f.flock(File::LOCK_SH)
  #   p f.read
  # }
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(T.any(Integer, TrueClass, FalseClass))
  end
  def flock(arg0); end

  sig do
    params(
        file: T.any(String, Pathname),
        mode: T.any(String, Integer),
        perm: String,
        opt: T.untyped,
    )
    .void
  end
  def initialize(file, mode=T.unsafe(nil), perm=T.unsafe(nil), **opt); end

  # Same as
  # [`IO#stat`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-stat), but
  # does not follow the last symbolic link. Instead, reports on the link itself.
  #
  # ```ruby
  # File.symlink("testfile", "link2test")   #=> 0
  # File.stat("testfile").size              #=> 66
  # f = File.new("link2test")
  # f.lstat.size                            #=> 8
  # f.stat.size                             #=> 66
  # ```
  sig {returns(File::Stat)}
  def lstat(); end

  # Returns the modification time for *file*.
  #
  # ```ruby
  # File.new("testfile").mtime   #=> Wed Apr 09 08:53:14 CDT 2003
  # ```
  sig {returns(Time)}
  def mtime(); end

  # Returns the pathname used to create *file* as a string. Does not normalize
  # the name.
  #
  # The pathname may not point to the file corresponding to *file*. For
  # instance, the pathname becomes void when the file has been moved or deleted.
  #
  # This method raises
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) for a *file*
  # created using File::Constants::TMPFILE because they don't have a pathname.
  #
  # ```ruby
  # File.new("testfile").path               #=> "testfile"
  # File.new("/tmp/../tmp/xxx", "w").path   #=> "/tmp/../tmp/xxx"
  # ```
  sig {returns(String)}
  def path(); end

  # Returns the size of *file* in bytes.
  #
  # ```ruby
  # File.new("testfile").size   #=> 66
  # ```
  sig {returns(Integer)}
  def size(); end

  # Truncates *file* to at most *integer* bytes. The file must be opened for
  # writing. Not available on all platforms.
  #
  # ```ruby
  # f = File.new("out", "w")
  # f.syswrite("1234567890")   #=> 10
  # f.truncate(5)              #=> 0
  # f.close()                  #=> nil
  # File.size("out")           #=> 5
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def truncate(arg0); end

  # Returns true if `path` matches against `pattern`. The pattern is not a
  # regular expression; instead it follows rules similar to shell filename
  # globbing. It may contain the following metacharacters:
  #
  # `*`
  # :   Matches any file. Can be restricted by other values in the glob.
  #     Equivalent to `/ .* /x` in regexp.
  #
  #     `*`
  # :       Matches all files regular files
  #     `c*`
  # :       Matches all files beginning with `c`
  #     `*c`
  # :       Matches all files ending with `c`
  #     `*c*`
  # :       Matches all files that have `c` in them (including at the beginning
  #         or end).
  #
  #
  #     To match hidden files (that start with a `.` set the File::FNM\_DOTMATCH
  #     flag.
  #
  # `**`
  # :   Matches directories recursively or files expansively.
  #
  # `?`
  # :   Matches any one character. Equivalent to `/.{1}/` in regexp.
  #
  # `[set]`
  # :   Matches any one character in `set`. Behaves exactly like character sets
  #     in [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html),
  #     including set negation (`[^a-z]`).
  #
  # ` \ `
  # :   Escapes the next metacharacter.
  #
  # `{a,b}`
  # :   Matches pattern a and pattern b if File::FNM\_EXTGLOB flag is enabled.
  #     Behaves like a
  #     [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) union
  #     (`(?:a|b)`).
  #
  #
  # `flags` is a bitwise OR of the `FNM_XXX` constants. The same glob pattern
  # and flags are used by
  # [`Dir::glob`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-glob).
  #
  # Examples:
  #
  # ```ruby
  # File.fnmatch('cat',       'cat')        #=> true  # match entire string
  # File.fnmatch('cat',       'category')   #=> false # only match partial string
  #
  # File.fnmatch('c{at,ub}s', 'cats')                    #=> false # { } isn't supported by default
  # File.fnmatch('c{at,ub}s', 'cats', File::FNM_EXTGLOB) #=> true  # { } is supported on FNM_EXTGLOB
  #
  # File.fnmatch('c?t',     'cat')          #=> true  # '?' match only 1 character
  # File.fnmatch('c??t',    'cat')          #=> false # ditto
  # File.fnmatch('c*',      'cats')         #=> true  # '*' match 0 or more characters
  # File.fnmatch('c*t',     'c/a/b/t')      #=> true  # ditto
  # File.fnmatch('ca[a-z]', 'cat')          #=> true  # inclusive bracket expression
  # File.fnmatch('ca[^t]',  'cat')          #=> false # exclusive bracket expression ('^' or '!')
  #
  # File.fnmatch('cat', 'CAT')                     #=> false # case sensitive
  # File.fnmatch('cat', 'CAT', File::FNM_CASEFOLD) #=> true  # case insensitive
  # File.fnmatch('cat', 'CAT', File::FNM_SYSCASE)  #=> true or false # depends on the system default
  #
  # File.fnmatch('?',   '/', File::FNM_PATHNAME)  #=> false # wildcard doesn't match '/' on FNM_PATHNAME
  # File.fnmatch('*',   '/', File::FNM_PATHNAME)  #=> false # ditto
  # File.fnmatch('[/]', '/', File::FNM_PATHNAME)  #=> false # ditto
  #
  # File.fnmatch('\?',   '?')                       #=> true  # escaped wildcard becomes ordinary
  # File.fnmatch('\a',   'a')                       #=> true  # escaped ordinary remains ordinary
  # File.fnmatch('\a',   '\a', File::FNM_NOESCAPE)  #=> true  # FNM_NOESCAPE makes '\' ordinary
  # File.fnmatch('[\?]', '?')                       #=> true  # can escape inside bracket expression
  #
  # File.fnmatch('*',   '.profile')                      #=> false # wildcard doesn't match leading
  # File.fnmatch('*',   '.profile', File::FNM_DOTMATCH)  #=> true  # period by default.
  # File.fnmatch('.*',  '.profile')                      #=> true
  #
  # rbfiles = '**' '/' '*.rb' # you don't have to do like this. just write in single string.
  # File.fnmatch(rbfiles, 'main.rb')                    #=> false
  # File.fnmatch(rbfiles, './main.rb')                  #=> false
  # File.fnmatch(rbfiles, 'lib/song.rb')                #=> true
  # File.fnmatch('**.rb', 'main.rb')                    #=> true
  # File.fnmatch('**.rb', './main.rb')                  #=> false
  # File.fnmatch('**.rb', 'lib/song.rb')                #=> true
  # File.fnmatch('*',           'dave/.profile')                      #=> true
  #
  # pattern = '*' '/' '*'
  # File.fnmatch(pattern, 'dave/.profile', File::FNM_PATHNAME)  #=> false
  # File.fnmatch(pattern, 'dave/.profile', File::FNM_PATHNAME | File::FNM_DOTMATCH) #=> true
  #
  # pattern = '**' '/' 'foo'
  # File.fnmatch(pattern, 'a/b/c/foo', File::FNM_PATHNAME)     #=> true
  # File.fnmatch(pattern, '/a/b/c/foo', File::FNM_PATHNAME)    #=> true
  # File.fnmatch(pattern, 'c:/a/b/c/foo', File::FNM_PATHNAME)  #=> true
  # File.fnmatch(pattern, 'a/.b/c/foo', File::FNM_PATHNAME)    #=> false
  # File.fnmatch(pattern, 'a/.b/c/foo', File::FNM_PATHNAME | File::FNM_DOTMATCH) #=> true
  # ```
  sig do
    params(
        pattern: String,
        path: String,
        flags: Integer,
    )
    .returns(T::Boolean)
  end
  def self.fnmatch?(pattern, path, flags=T.unsafe(nil)); end

  # Deletes the named files, returning the number of names passed as arguments.
  # Raises an exception on any error. Since the underlying implementation relies
  # on the `unlink(2)` system call, the type of exception raised depends on its
  # error type (see https://linux.die.net/man/2/unlink) and has the form of e.g.
  # Errno::ENOENT.
  #
  # See also
  # [`Dir::rmdir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-rmdir).
  sig do
    params(
        files: String,
    )
    .returns(Integer)
  end
  def self.unlink(*files); end

  # Returns the pathname used to create *file* as a string. Does not normalize
  # the name.
  #
  # The pathname may not point to the file corresponding to *file*. For
  # instance, the pathname becomes void when the file has been moved or deleted.
  #
  # This method raises
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) for a *file*
  # created using File::Constants::TMPFILE because they don't have a pathname.
  #
  # ```ruby
  # File.new("testfile").path               #=> "testfile"
  # File.new("/tmp/../tmp/xxx", "w").path   #=> "/tmp/../tmp/xxx"
  # ```
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

# Objects of class
# [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html) encapsulate
# common status information for
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) objects. The
# information is recorded at the moment the
# [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html) object is
# created; changes made to the file after that point will not be reflected.
# [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html) objects are
# returned by
# [`IO#stat`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-stat),
# [`File::stat`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-stat),
# [`File#lstat`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-i-lstat),
# and
# [`File::lstat`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-lstat).
# Many of these methods return platform-specific values, and not all values are
# meaningful on all systems. See also
# [`Kernel#test`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-test).
class File::Stat < Object
  include Comparable

  # Compares [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html)
  # objects by comparing their respective modification times.
  #
  # `nil` is returned if `other_stat` is not a
  # [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html) object
  #
  # ```ruby
  # f1 = File.new("f1", "w")
  # sleep 1
  # f2 = File.new("f2", "w")
  # f1.stat <=> f2.stat   #=> -1
  # ```
  sig do
    params(
        other: Object,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  # Returns the last access time for this file as an object of class
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html).
  #
  # ```ruby
  # File.stat("testfile").atime   #=> Wed Dec 31 18:00:00 CST 1969
  # ```
  sig {returns(Time)}
  def atime(); end

  # Returns the birth time for *stat*.
  #
  # If the platform doesn't have birthtime, raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html).
  #
  # ```ruby
  # File.write("testfile", "foo")
  # sleep 10
  # File.write("testfile", "bar")
  # sleep 10
  # File.chmod(0644, "testfile")
  # sleep 10
  # File.read("testfile")
  # File.stat("testfile").birthtime   #=> 2014-02-24 11:19:17 +0900
  # File.stat("testfile").mtime       #=> 2014-02-24 11:19:27 +0900
  # File.stat("testfile").ctime       #=> 2014-02-24 11:19:37 +0900
  # File.stat("testfile").atime       #=> 2014-02-24 11:19:47 +0900
  # ```
  sig {returns(Time)}
  def birthtime(); end

  # Returns the native file system's block size. Will return `nil` on platforms
  # that don't support this information.
  #
  # ```ruby
  # File.stat("testfile").blksize   #=> 4096
  # ```
  sig {returns(T.nilable(Integer))}
  def blksize(); end

  # Returns `true` if the file is a block device, `false` if it isn't or if the
  # operating system doesn't support this feature.
  #
  # ```ruby
  # File.stat("testfile").blockdev?    #=> false
  # File.stat("/dev/hda1").blockdev?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def blockdev?(); end

  # Returns the number of native file system blocks allocated for this file, or
  # `nil` if the operating system doesn't support this feature.
  #
  # ```ruby
  # File.stat("testfile").blocks   #=> 2
  # ```
  sig {returns(T.nilable(Integer))}
  def blocks(); end

  # Returns `true` if the file is a character device, `false` if it isn't or if
  # the operating system doesn't support this feature.
  #
  # ```ruby
  # File.stat("/dev/tty").chardev?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def chardev?(); end

  # Returns the change time for *stat* (that is, the time directory information
  # about the file was changed, not the file itself).
  #
  # Note that on Windows (NTFS), returns creation time (birth time).
  #
  # ```ruby
  # File.stat("testfile").ctime   #=> Wed Apr 09 08:53:14 CDT 2003
  # ```
  sig {returns(Time)}
  def ctime(); end

  # Returns an integer representing the device on which *stat* resides.
  #
  # ```ruby
  # File.stat("testfile").dev   #=> 774
  # ```
  sig {returns(Integer)}
  def dev(); end

  # Returns the major part of `File_Stat#dev` or `nil`.
  #
  # ```ruby
  # File.stat("/dev/fd1").dev_major   #=> 2
  # File.stat("/dev/tty").dev_major   #=> 5
  # ```
  sig {returns(Integer)}
  def dev_major(); end

  # Returns the minor part of `File_Stat#dev` or `nil`.
  #
  # ```ruby
  # File.stat("/dev/fd1").dev_minor   #=> 1
  # File.stat("/dev/tty").dev_minor   #=> 0
  # ```
  sig {returns(Integer)}
  def dev_minor(); end

  # Returns `true` if the named file is a directory, or a symlink that points at
  # a directory, and `false` otherwise.
  #
  # *file\_name* can be an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object.
  #
  # ```ruby
  # File.directory?(".")
  # ```
  sig {returns(T::Boolean)}
  def directory?(); end

  # Returns `true` if *stat* is executable or if the operating system doesn't
  # distinguish executable files from nonexecutable files. The tests are made
  # using the effective owner of the process.
  #
  # ```ruby
  # File.stat("testfile").executable?   #=> false
  # ```
  sig {returns(T::Boolean)}
  def executable?(); end

  # Same as `executable?`, but tests using the real owner of the process.
  sig {returns(T::Boolean)}
  def executable_real?(); end

  # Returns `true` if *stat* is a regular file (not a device file, pipe, socket,
  # etc.).
  #
  # ```ruby
  # File.stat("testfile").file?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def file?(); end

  # Identifies the type of *stat*. The return string is one of: "`file`",
  # "`directory`", "`characterSpecial`", "`blockSpecial`", "`fifo`", "`link`",
  # "`socket`", or "`unknown`".
  #
  # ```ruby
  # File.stat("/dev/tty").ftype   #=> "characterSpecial"
  # ```
  sig {returns(String)}
  def ftype(); end

  # Returns the numeric group id of the owner of *stat*.
  #
  # ```ruby
  # File.stat("testfile").gid   #=> 500
  # ```
  sig {returns(Integer)}
  def gid(); end

  # Returns true if the effective group id of the process is the same as the
  # group id of *stat*. On Windows NT, returns `false`.
  #
  # ```ruby
  # File.stat("testfile").grpowned?      #=> true
  # File.stat("/etc/passwd").grpowned?   #=> false
  # ```
  sig {returns(T::Boolean)}
  def grpowned?(); end

  sig do
    params(
        file: String,
    )
    .void
  end
  def initialize(file); end

  # Returns the inode number for *stat*.
  #
  # ```ruby
  # File.stat("testfile").ino   #=> 1083669
  # ```
  sig {returns(Integer)}
  def ino(); end

  # Produce a nicely formatted description of *stat*.
  #
  # ```ruby
  # File.stat("/etc/passwd").inspect
  #    #=> "#<File::Stat dev=0xe000005, ino=1078078, mode=0100644,
  #    #    nlink=1, uid=0, gid=0, rdev=0x0, size=1374, blksize=4096,
  #    #    blocks=8, atime=Wed Dec 10 10:16:12 CST 2003,
  #    #    mtime=Fri Sep 12 15:41:41 CDT 2003,
  #    #    ctime=Mon Oct 27 11:20:27 CST 2003,
  #    #    birthtime=Mon Aug 04 08:13:49 CDT 2003>"
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns an integer representing the permission bits of *stat*. The meaning
  # of the bits is platform dependent; on Unix systems, see `stat(2)`.
  #
  # ```ruby
  # File.chmod(0644, "testfile")   #=> 1
  # s = File.stat("testfile")
  # sprintf("%o", s.mode)          #=> "100644"
  # ```
  sig {returns(Integer)}
  def mode(); end

  # Returns the modification time of *stat*.
  #
  # ```ruby
  # File.stat("testfile").mtime   #=> Wed Apr 09 08:53:14 CDT 2003
  # ```
  sig {returns(Time)}
  def mtime(); end

  # Returns the number of hard links to *stat*.
  #
  # ```ruby
  # File.stat("testfile").nlink             #=> 1
  # File.link("testfile", "testfile.bak")   #=> 0
  # File.stat("testfile").nlink             #=> 2
  # ```
  sig {returns(Integer)}
  def nlink(); end

  # Returns `true` if the effective user id of the process is the same as the
  # owner of *stat*.
  #
  # ```ruby
  # File.stat("testfile").owned?      #=> true
  # File.stat("/etc/passwd").owned?   #=> false
  # ```
  sig {returns(T::Boolean)}
  def owned?(); end

  # Returns `true` if the operating system supports pipes and *stat* is a pipe;
  # `false` otherwise.
  sig {returns(T::Boolean)}
  def pipe?(); end

  # Returns an integer representing the device type on which *stat* resides.
  # Returns `nil` if the operating system doesn't support this feature.
  #
  # ```ruby
  # File.stat("/dev/fd1").rdev   #=> 513
  # File.stat("/dev/tty").rdev   #=> 1280
  # ```
  sig {returns(T.nilable(Integer))}
  def rdev(); end

  # Returns the major part of `File_Stat#rdev` or `nil`.
  #
  # ```ruby
  # File.stat("/dev/fd1").rdev_major   #=> 2
  # File.stat("/dev/tty").rdev_major   #=> 5
  # ```
  sig {returns(Integer)}
  def rdev_major(); end

  # Returns the minor part of `File_Stat#rdev` or `nil`.
  #
  # ```ruby
  # File.stat("/dev/fd1").rdev_minor   #=> 1
  # File.stat("/dev/tty").rdev_minor   #=> 0
  # ```
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

  # Returns `true` if *stat* is readable by the effective user id of this
  # process.
  #
  # ```ruby
  # File.stat("testfile").readable?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def readable?(); end

  # Returns `true` if *stat* is readable by the real user id of this process.
  #
  # ```ruby
  # File.stat("testfile").readable_real?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def readable_real?(); end

  # Returns `true` if *stat* has the set-group-id permission bit set, `false` if
  # it doesn't or if the operating system doesn't support this feature.
  #
  # ```ruby
  # File.stat("/usr/sbin/lpc").setgid?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def setgid?(); end

  # Returns `true` if *stat* has the set-user-id permission bit set, `false` if
  # it doesn't or if the operating system doesn't support this feature.
  #
  # ```ruby
  # File.stat("/bin/su").setuid?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def setuid?(); end

  # Returns the size of *stat* in bytes.
  #
  # ```ruby
  # File.stat("testfile").size   #=> 66
  # ```
  sig {returns(Integer)}
  def size(); end

  # Returns the size of *stat* in bytes.
  #
  # ```ruby
  # File.stat("testfile").size   #=> 66
  # ```
  def size?(); end

  # Returns `true` if *stat* is a socket, `false` if it isn't or if the
  # operating system doesn't support this feature.
  #
  # ```ruby
  # File.stat("testfile").socket?   #=> false
  # ```
  sig {returns(T::Boolean)}
  def socket?(); end

  # Returns `true` if *stat* has its sticky bit set, `false` if it doesn't or if
  # the operating system doesn't support this feature.
  #
  # ```ruby
  # File.stat("testfile").sticky?   #=> false
  # ```
  sig {returns(T::Boolean)}
  def sticky?(); end

  # Returns `true` if *stat* is a symbolic link, `false` if it isn't or if the
  # operating system doesn't support this feature. As
  # [`File::stat`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-stat)
  # automatically follows symbolic links,
  # [`symlink?`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html#method-i-symlink-3F)
  # will always be `false` for an object returned by
  # [`File::stat`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-stat).
  #
  # ```ruby
  # File.symlink("testfile", "alink")   #=> 0
  # File.stat("alink").symlink?         #=> false
  # File.lstat("alink").symlink?        #=> true
  # ```
  sig {returns(T::Boolean)}
  def symlink?(); end

  # Returns the numeric user id of the owner of *stat*.
  #
  # ```ruby
  # File.stat("testfile").uid   #=> 501
  # ```
  sig {returns(Integer)}
  def uid(); end

  # If *stat* is readable by others, returns an integer representing the file
  # permission bits of *stat*. Returns `nil` otherwise. The meaning of the bits
  # is platform dependent; on Unix systems, see `stat(2)`.
  #
  # ```ruby
  # m = File.stat("/etc/passwd").world_readable?  #=> 420
  # sprintf("%o", m)                              #=> "644"
  # ```
  sig {returns(T.nilable(Integer))}
  def world_readable?(); end

  # If *stat* is writable by others, returns an integer representing the file
  # permission bits of *stat*. Returns `nil` otherwise. The meaning of the bits
  # is platform dependent; on Unix systems, see `stat(2)`.
  #
  # ```ruby
  # m = File.stat("/tmp").world_writable?         #=> 511
  # sprintf("%o", m)                              #=> "777"
  # ```
  sig {returns(T.nilable(Integer))}
  def world_writable?(); end

  # Returns `true` if *stat* is writable by the effective user id of this
  # process.
  #
  # ```ruby
  # File.stat("testfile").writable?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def writable?(); end

  # Returns `true` if *stat* is writable by the real user id of this process.
  #
  # ```ruby
  # File.stat("testfile").writable_real?   #=> true
  # ```
  sig {returns(T::Boolean)}
  def writable_real?(); end

  # Returns `true` if *stat* is a zero-length file; `false` otherwise.
  #
  # ```ruby
  # File.stat("testfile").zero?   #=> false
  # ```
  sig {returns(T::Boolean)}
  def zero?(); end
end
