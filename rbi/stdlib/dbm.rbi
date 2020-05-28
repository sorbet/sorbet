# typed: __STDLIB_INTERNAL

# ## Introduction
#
# The [`DBM`](https://docs.ruby-lang.org/en/2.6.0/DBM.html) class provides a
# wrapper to a Unix-style [dbm](http://en.wikipedia.org/wiki/Dbm) or Database
# Manager library.
#
# Dbm databases do not have tables or columns; they are simple key-value data
# stores, like a Ruby [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html)
# except not resident in RAM. Keys and values must be strings.
#
# The exact library used depends on how Ruby was compiled. It could be any of
# the following:
#
# *   The original ndbm library is released in 4.3BSD. It is based on dbm
#     library in Unix Version 7 but has different API to support multiple
#     databases in a process.
# *   [Berkeley DB](http://en.wikipedia.org/wiki/Berkeley_DB) versions 1 thru 6,
#     also known as BDB and Sleepycat DB, now owned by Oracle Corporation.
# *   Berkeley DB 1.x, still found in 4.4BSD derivatives (FreeBSD, OpenBSD,
#     etc).
# *   [gdbm](http://www.gnu.org/software/gdbm/), the GNU implementation of dbm.
# *   [qdbm](http://fallabs.com/qdbm/index.html), another open source
#     reimplementation of dbm.
#
#
# All of these dbm implementations have their own Ruby interfaces available,
# which provide richer (but varying) APIs.
#
# ## Cautions
#
# Before you decide to use
# [`DBM`](https://docs.ruby-lang.org/en/2.6.0/DBM.html), there are some issues
# you should consider:
#
# *   Each implementation of dbm has its own file format. Generally, dbm
#     libraries will not read each other's files. This makes dbm files a bad
#     choice for data exchange.
#
# *   Even running the same OS and the same dbm implementation, the database
#     file format may depend on the CPU architecture. For example, files may not
#     be portable between PowerPC and 386, or between 32 and 64 bit Linux.
#
# *   Different versions of Berkeley DB use different file formats. A change to
#     the OS may therefore break
#     [`DBM`](https://docs.ruby-lang.org/en/2.6.0/DBM.html) access to existing
#     files.
#
# *   [`Data`](https://docs.ruby-lang.org/en/2.6.0/Data.html) size limits vary
#     between implementations. Original Berkeley DB was limited to 2GB of data.
#     Dbm libraries also sometimes limit the total size of a key/value pair, and
#     the total size of all the keys that hash to the same value. These limits
#     can be as little as 512 bytes. That said, gdbm and recent versions of
#     Berkeley DB do away with these limits.
#
#
# Given the above cautions,
# [`DBM`](https://docs.ruby-lang.org/en/2.6.0/DBM.html) is not a good choice for
# long term storage of important data. It is probably best used as a fast and
# easy alternative to a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html)
# for processing large amounts of data.
#
# ## Example
#
# ```ruby
# require 'dbm'
# db = DBM.open('rfcs', 0666, DBM::WRCREAT)
# db['822'] = 'Standard for the Format of ARPA Internet Text Messages'
# db['1123'] = 'Requirements for Internet Hosts - Application and Support'
# db['3068'] = 'An Anycast Prefix for 6to4 Relay Routers'
# puts db['822']
# ```
class DBM
  include(::Enumerable)

  Elem = type_member(:out)

  # Indicates that dbm\_open() should open the database in read/write mode,
  # create it if it does not already exist, and delete all contents if it does
  # already exist.
  NEWDB = T.let(T.unsafe(nil), Integer)

  # Indicates that dbm\_open() should open the database in read-only mode
  READER = T.let(T.unsafe(nil), Integer)

  # Identifies ndbm library version.
  #
  # Examples:
  #
  # *   "ndbm (4.3BSD)"
  # *   "Berkeley DB 4.8.30: (April  9, 2010)"
  # *   "Berkeley DB (unknown)" (4.4BSD, maybe)
  # *   "GDBM version 1.8.3. 10/15/2002 (built Jul  1 2011 12:32:45)"
  # *   "QDBM 1.8.78"
  VERSION = T.let(T.unsafe(nil), String)

  # Indicates that dbm\_open() should open the database in read/write mode, and
  # create it if it does not already exist
  WRCREAT = T.let(T.unsafe(nil), Integer)

  # Indicates that dbm\_open() should open the database in read/write mode
  WRITER = T.let(T.unsafe(nil), Integer)

  # Return a value from the database by locating the key string provided. If the
  # key is not found, returns nil.
  def [](key); end

  # Stores the specified string value in the database, indexed via the string
  # key provided.
  def []=(key, value); end

  # Deletes all data from the database.
  def clear; end

  # Closes the database.
  def close; end

  # Returns true if the database is closed, false otherwise.
  def closed?; end

  # Deletes an entry from the database.
  def delete(key); end

  # Deletes all entries for which the code block returns true. Returns self.
  def delete_if(&blk); end

  # Calls the block once for each key string in the database. Returns self.
  def each_key(&blk); end

  # Calls the block once for each [key, value] pair in the database. Returns
  # self.
  def each_pair(&blk); end

  # Calls the block once for each value string in the database. Returns self.
  def each_value(&blk); end

  # Returns true if the database is empty, false otherwise.
  def empty?; end

  # Return a value from the database by locating the key string provided. If the
  # key is not found, returns `ifnone`. If `ifnone` is not given, raises
  # [`IndexError`](https://docs.ruby-lang.org/en/2.6.0/IndexError.html).
  def fetch(key, if_none = nil); end

  # Returns true if the database contains the specified key, false otherwise.
  def has_key?(key); end

  # Returns true if the database contains the specified string value, false
  # otherwise.
  def has_value?(value); end

  # Returns true if the database contains the specified key, false otherwise.
  def include?(key); end

  # Returns a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) (not a
  # [`DBM`](https://docs.ruby-lang.org/en/2.6.0/DBM.html) database) created by
  # using each value in the database as a key, with the corresponding key as its
  # value.
  def invert; end

  # Returns the key for the specified value.
  def key(value); end

  # Returns true if the database contains the specified key, false otherwise.
  def key?(value); end

  # Returns an array of all the string keys in the database.
  def keys; end

  # Returns the number of entries in the database.
  def length; end

  # Returns true if the database contains the specified key, false otherwise.
  def member?(key); end

  # Converts the contents of the database to an in-memory
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html), then calls
  # [`Hash#reject`](https://docs.ruby-lang.org/en/2.6.0/Hash.html#method-i-reject)
  # with the specified code block, returning a new
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html).
  def reject(&blk); end

  # Deletes all entries for which the code block returns true. Returns self.
  def reject!; end

  # Replaces the contents of the database with the contents of the specified
  # object. Takes any object which implements the
  # [`each_pair`](https://docs.ruby-lang.org/en/2.6.0/DBM.html#method-i-each_pair)
  # method, including [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html)
  # and [`DBM`](https://docs.ruby-lang.org/en/2.6.0/DBM.html) objects.
  def replace(_); end

  # Returns a new array consisting of the [key, value] pairs for which the code
  # block returns true.
  def select; end

  # Removes a [key, value] pair from the database, and returns it. If the
  # database is empty, returns nil. The order in which values are
  # removed/returned is not guaranteed.
  def shift; end

  # Returns the number of entries in the database.
  def size; end

  # Stores the specified string value in the database, indexed via the string
  # key provided.
  def store(_, _); end

  # Converts the contents of the database to an array of [key, value] arrays,
  # and returns it.
  def to_a; end

  # Converts the contents of the database to an in-memory
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) object, and returns
  # it.
  def to_hash; end

  # Updates the database with multiple values from the specified object. Takes
  # any object which implements the
  # [`each_pair`](https://docs.ruby-lang.org/en/2.6.0/DBM.html#method-i-each_pair)
  # method, including [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html)
  # and [`DBM`](https://docs.ruby-lang.org/en/2.6.0/DBM.html) objects.
  def update(_); end

  # Returns true if the database contains the specified string value, false
  # otherwise.
  def value?(_); end

  # Returns an array of all the string values in the database.
  def values; end

  # Returns an array containing the values associated with the given keys.
  def values_at(*_); end

  # Open a dbm database with the specified name, which can include a directory
  # path. Any file extensions needed will be supplied automatically by the dbm
  # library. For example, Berkeley DB appends '.db', and GNU gdbm uses two
  # physical files with extensions '.dir' and '.pag'.
  #
  # The mode should be an integer, as for Unix chmod.
  #
  # Flags should be one of
  # [`READER`](https://docs.ruby-lang.org/en/2.6.0/DBM.html#READER),
  # [`WRITER`](https://docs.ruby-lang.org/en/2.6.0/DBM.html#WRITER),
  # [`WRCREAT`](https://docs.ruby-lang.org/en/2.6.0/DBM.html#WRCREAT) or
  # [`NEWDB`](https://docs.ruby-lang.org/en/2.6.0/DBM.html#NEWDB).
  def self.new(*_); end

  # Open a dbm database and yields it if a block is given. See also `DBM.new`.
  def self.open(*_); end
end

# [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) class used
# to return errors from the dbm library.
class DBMError < ::StandardError; end
