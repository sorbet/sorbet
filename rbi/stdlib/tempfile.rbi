# typed: __STDLIB_INTERNAL

# A utility class for managing temporary files. When you create a
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object, it
# will create a temporary file with a unique filename. A
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) objects
# behaves just like a [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)
# object, and you can perform all the usual file operations on it: reading data,
# writing data, changing its permissions, etc. So although this class does not
# explicitly document all instance methods supported by
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html), you can in fact call
# any [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) instance method on
# a [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object.
#
# ## Synopsis
#
# ```ruby
# require 'tempfile'
#
# file = Tempfile.new('foo')
# file.path      # => A unique filename in the OS's temp directory,
#                #    e.g.: "/tmp/foo.24722.0"
#                #    This filename contains 'foo' in its basename.
# file.write("hello world")
# file.rewind
# file.read      # => "hello world"
# file.close
# file.unlink    # deletes the temp file
# ```
#
# ## Good practices
#
# ### Explicit close
#
# When a [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object
# is garbage collected, or when the Ruby interpreter exits, its associated
# temporary file is automatically deleted. This means that's it's unnecessary to
# explicitly delete a
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) after use,
# though it's good practice to do so: not explicitly deleting unused Tempfiles
# can potentially leave behind large amounts of tempfiles on the filesystem
# until they're garbage collected. The existence of these temp files can make it
# harder to determine a new
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) filename.
#
# Therefore, one should always call
# [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-unlink)
# or close in an ensure block, like this:
#
# ```ruby
# file = Tempfile.new('foo')
# begin
#    # ...do something with file...
# ensure
#    file.close
#    file.unlink   # deletes the temp file
# end
# ```
#
# ### Unlink after creation
#
# On POSIX systems, it's possible to unlink a file right after creating it, and
# before closing it. This removes the filesystem entry without closing the file
# handle, so it ensures that only the processes that already had the file handle
# open can access the file's contents. It's strongly recommended that you do
# this if you do not want any other processes to be able to read from or write
# to the [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html), and
# you do not need to know the Tempfile's filename either.
#
# For example, a practical use case for unlink-after-creation would be this: you
# need a large byte buffer that's too large to comfortably fit in RAM, e.g. when
# you're writing a web server and you want to buffer the client's file upload
# data.
#
# Please refer to
# [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-unlink)
# for more information and a code example.
#
# ## Minor notes
#
# Tempfile's filename picking method is both thread-safe and inter-process-safe:
# it guarantees that no other threads or processes will pick the same filename.
#
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) itself however
# may not be entirely thread-safe. If you access the same
# [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object from
# multiple threads then you should protect it with a mutex.
class Tempfile
  extend T::Sig

  extend T::Generic
  Elem = type_member(:out, fixed: String)

  # Creates a temporary file as usual
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) object (not
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html)). It doesn't
  # use finalizer and delegation.
  #
  # If no block is given, this is similar to
  # [`Tempfile.new`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-c-new)
  # except creating [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)
  # instead of [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html).
  # The created file is not removed automatically. You should use
  # [`File.unlink`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-unlink)
  # to remove it.
  #
  # If a block is given, then a
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) object will be
  # constructed, and the block is invoked with the object as the argument. The
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) object will be
  # automatically closed and the temporary file is removed after the block
  # terminates. The call returns the value of the block.
  #
  # In any case, all arguments (`basename`, `tmpdir`, `mode`, and `**options`)
  # will be treated as
  # [`Tempfile.new`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-c-new).
  #
  # ```ruby
  # Tempfile.create('foo', '/home/temp') do |f|
  #    # ... do something with f ...
  # end
  # ```
  sig do
    params(
      basename: T.any(String, [String, String]),
      tmpdir: T.nilable(String),
      mode: Integer,
      options: T.untyped,
      blk: T.nilable(T.proc.params(arg0: File).returns(T.untyped)),
    )
    .returns(T.untyped)
  end
  def self.create(basename="", tmpdir=nil, mode: 0, **options, &blk); end

  # Creates a new
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html).
  #
  # If no block is given, this is a synonym for
  # [`Tempfile.new`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-c-new).
  #
  # If a block is given, then a
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object will
  # be constructed, and the block is run with said object as argument. The
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) object will
  # be automatically closed after the block terminates. The call returns the
  # value of the block.
  #
  # In any case, all arguments (`*args`) will be passed to
  # [`Tempfile.new`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-c-new).
  #
  # ```ruby
  # Tempfile.open('foo', '/home/temp') do |f|
  #    # ... do something with f ...
  # end
  #
  # # Equivalent:
  # f = Tempfile.open('foo', '/home/temp')
  # begin
  #    # ... do something with f ...
  # ensure
  #    f.close
  # end
  # ```
  sig do
    params(
      basename: T.any(String, [String, String]),
      tmpdir: T.nilable(String),
      mode: Integer,
      options: T.untyped,
      blk: T.nilable(T.proc.params(arg0: Tempfile).returns(T.untyped)),
    )
    .returns(T.untyped)
  end
  def self.open(basename='', tmpdir=nil, mode: 0, **options, &blk); end

  sig do
    params(
      basename: T.any(String, [String, String]),
      tmpdir: T.nilable(String),
      mode: Integer,
      options: T.untyped,
    )
    .void
  end
  def initialize(basename='', tmpdir=nil, mode: 0, **options); end

  # Closes the file. If `unlink_now` is true, then the file will be unlinked
  # (deleted) after closing. Of course, you can choose to later call
  # [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-unlink)
  # if you do not unlink it now.
  #
  # If you don't explicitly unlink the temporary file, the removal will be
  # delayed until the object is finalized.
  sig {params(unlink_now: T::Boolean).void}
  def close(unlink_now=false); end

  # Closes and unlinks (deletes) the file. Has the same effect as called
  # `close(true)`.
  sig {void}
  def close!; end

  # Alias for:
  # [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-unlink)
  sig {returns(T::Boolean)}
  def delete; end

  # Alias for:
  # [`size`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-size)
  sig {returns(Integer)}
  def length; end

  # Opens or reopens the file with mode "r+".
  sig {returns(Tempfile)}
  def open; end

  ### path returns nil if the Tempfile has been unlinked.
  # Returns the full path name of the temporary file. This will be nil if
  # [`unlink`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-unlink)
  # has been called.
  sig {returns(T.nilable(String))}
  def path; end

  # Returns the size of the temporary file. As a side effect, the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) buffer is flushed before
  # determining the size.
  #
  # Also aliased as:
  # [`length`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-length)
  sig {returns(Integer)}
  def size; end

  # Unlinks (deletes) the file from the filesystem. One should always unlink the
  # file after using it, as is explained in the "Explicit close" good practice
  # section in the
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) overview:
  #
  # ```ruby
  # file = Tempfile.new('foo')
  # begin
  #    # ...do something with file...
  # ensure
  #    file.close
  #    file.unlink   # deletes the temp file
  # end
  # ```
  #
  # ### Unlink-before-close
  #
  # On POSIX systems it's possible to unlink a file before closing it. This
  # practice is explained in detail in the
  # [`Tempfile`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html) overview
  # (section "Unlink after creation"); please refer there for more information.
  #
  # However, unlink-before-close may not be supported on non-POSIX operating
  # systems. Microsoft Windows is the most notable case: unlinking a non-closed
  # file will result in an error, which this method will silently ignore. If you
  # want to practice unlink-before-close whenever possible, then you should
  # write code like this:
  #
  # ```ruby
  # file = Tempfile.new('foo')
  # file.unlink   # On Windows this silently fails.
  # begin
  #    # ... do something with file ...
  # ensure
  #    file.close!   # Closes the file handle. If the file wasn't unlinked
  #                  # because #unlink failed, then this method will attempt
  #                  # to do so again.
  # end
  # ```
  #
  #
  # Also aliased as:
  # [`delete`](https://docs.ruby-lang.org/en/2.7.0/Tempfile.html#method-i-delete)
  sig {returns(T::Boolean)}
  def unlink; end

  sig do
    params(
        length: Integer,
        outbuf: String,
    )
    .returns(String)
  end
  def read(length=T.unsafe(nil), outbuf=T.unsafe(nil)); end

  sig do
    params(
        arg0: Object,
    )
    .returns(Integer)
  end
  def write(arg0); end
end
