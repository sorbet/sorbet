# typed: __STDLIB_INTERNAL

# Objects of class [`Dir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html) are
# directory streams representing directories in the underlying file system. They
# provide a variety of ways to list directories and their contents. See also
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html).
#
# The directory used in these examples contains the two regular files
# (`config.h` and `main.rb`), the parent directory (`..`), and the directory
# itself (`.`).
class Dir
  # [`Dir.mktmpdir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-mktmpdir)
  # creates a temporary directory.
  #
  # The directory is created with 0700 permission. Application should not change
  # the permission to make the temporary directory accessible from other users.
  #
  # The prefix and suffix of the name of the directory is specified by the
  # optional first argument, *prefix\_suffix*.
  # *   If it is not specified or nil, "d" is used as the prefix and no suffix
  #     is used.
  # *   If it is a string, it is used as the prefix and no suffix is used.
  # *   If it is an array, first element is used as the prefix and second
  #     element is used as a suffix.
  #
  #
  # ```ruby
  # Dir.mktmpdir {|dir| dir is ".../d..." }
  # Dir.mktmpdir("foo") {|dir| dir is ".../foo..." }
  # Dir.mktmpdir(["foo", "bar"]) {|dir| dir is ".../foo...bar" }
  # ```
  #
  # The directory is created under
  # [`Dir.tmpdir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-tmpdir)
  # or the optional second argument *tmpdir* if non-nil value is given.
  #
  # ```ruby
  # Dir.mktmpdir {|dir| dir is "#{Dir.tmpdir}/d..." }
  # Dir.mktmpdir(nil, "/var/tmp") {|dir| dir is "/var/tmp/d..." }
  # ```
  #
  # If a block is given, it is yielded with the path of the directory. The
  # directory and its contents are removed using
  # [`FileUtils.remove_entry`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-remove_entry)
  # before
  # [`Dir.mktmpdir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-mktmpdir)
  # returns. The value of the block is returned.
  #
  # ```
  # Dir.mktmpdir {|dir|
  #   # use the directory...
  #   open("#{dir}/foo", "w") { ... }
  # }
  # ```
  #
  # If a block is not given, The path of the directory is returned. In this
  # case,
  # [`Dir.mktmpdir`](https://docs.ruby-lang.org/en/2.7.0/Dir.html#method-c-mktmpdir)
  # doesn't remove the directory.
  #
  # ```
  # dir = Dir.mktmpdir
  # begin
  #   # use the directory...
  #   open("#{dir}/foo", "w") { ... }
  # ensure
  #   # remove the directory.
  #   FileUtils.remove_entry dir
  # end
  # ```
  sig do
    params(
      prefix_suffix: T.nilable(T.any(String, T::Array[String])),
      tmpdir: T.nilable(String),
    )
    .returns(String)
  end
  sig do
    type_parameters(:U).params(
      prefix_suffix: T.nilable(T.any(String, T::Array[String])),
      tmpdir: T.nilable(String),
      blk: T.proc.params(dir: String).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def self.mktmpdir(prefix_suffix=nil, tmpdir=nil, &blk); end
end

module Dir::Tmpname
  sig do
    params(
      basename: T.any(String, T::Array[String]),
      tmpdir: T.nilable(String),
      max_try: T.nilable(Integer),
      opts: T.untyped,
      blk: T.proc.params(
        path: String,
        n: T.nilable(Integer),
        opts: T::Hash[Symbol, T.untyped],
        origdir: T.nilable(String)
      ).void
    )
    .returns(String)
  end
  def self.create(basename, tmpdir=nil, max_try: nil, **opts, &blk); end

  sig {returns(String)}
  def self.tmpdir(); end
end
