# typed: __STDLIB_INTERNAL

# ## Summary
#
# Ruby extension for GNU dbm (gdbm) -- a simple database engine for storing
# key-value pairs on disk.
#
# ## Description
#
# GNU dbm is a library for simple databases. A database is a file that stores
# key-value pairs. Gdbm allows the user to store, retrieve, and delete data by
# key. It furthermore allows a non-sorted traversal of all key-value pairs. A
# gdbm database thus provides the same functionality as a hash. As with objects
# of the [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) class, elements
# can be accessed with `[]`. Furthermore,
# [`GDBM`](https://docs.ruby-lang.org/en/2.6.0/GDBM.html) mixes in the
# [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) module,
# thus providing convenient methods such as
# [`find`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-find),
# [`collect`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-collect),
# [`map`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-map),
# etc.
#
# A process is allowed to open several different databases at the same time. A
# process can open a database as a "reader" or a "writer". Whereas a reader has
# only read-access to the database, a writer has read- and write-access. A
# database can be accessed either by any number of readers or by exactly one
# writer at the same time.
#
# ## Examples
#
# 1.  Opening/creating a database, and filling it with some entries:
#
# ```ruby
# require 'gdbm'
#
# gdbm = GDBM.new("fruitstore.db")
# gdbm["ananas"]    = "3"
# gdbm["banana"]    = "8"
# gdbm["cranberry"] = "4909"
# gdbm.close
# ```
#
# 2.  Reading out a database:
#
# ```ruby
# require 'gdbm'
#
# gdbm = GDBM.new("fruitstore.db")
# gdbm.each_pair do |key, value|
#   print "#{key}: #{value}\n"
# end
# gdbm.close
# ```
#
#     produces
#
# ```
# banana: 8
# ananas: 3
# cranberry: 4909
# ```
#
#
# ## Links
#
# *   http://www.gnu.org/software/gdbm/
class GDBM
  include(::Enumerable)

  Elem = type_member(:out)

  # flag for new and
  # [`open`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-open).
  # this flag is obsolete for gdbm >= 1.8
  FAST = T.let(T.unsafe(nil), Integer)

  # open database as a writer; overwrite any existing databases
  NEWDB = T.let(T.unsafe(nil), Integer)

  # flag for new and
  # [`open`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-open)
  NOLOCK = T.let(T.unsafe(nil), Integer)

  # open database as a reader
  READER = T.let(T.unsafe(nil), Integer)

  # flag for new and
  # [`open`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-open).
  # only for gdbm >= 1.8
  SYNC = T.let(T.unsafe(nil), Integer)

  # version of the gdbm library
  VERSION = T.let(T.unsafe(nil), String)

  # open database as a writer; if the database does not exist, create a new one
  WRCREAT = T.let(T.unsafe(nil), Integer)

  # open database as a writer
  WRITER = T.let(T.unsafe(nil), Integer)

  def initialize(*_); end

  # Retrieves the *value* corresponding to *key*.
  def [](_); end

  # Associates the value *value* with the specified *key*.
  def []=(_, _); end

  # Sets the size of the internal bucket cache to *size*.
  def cachesize=(_); end

  # Removes all the key-value pairs within *gdbm*.
  def clear; end

  # Closes the associated database file.
  def close; end

  # Returns true if the associated database file has been closed.
  def closed?; end

  # Removes the key-value-pair with the specified *key* from this database and
  # returns the corresponding *value*. Returns nil if the database is empty.
  def delete(_); end

  # Deletes every key-value pair from *gdbm* for which *block* evaluates to
  # true.
  def delete_if; end

  # Executes *block* for each key in the database, passing the *key* and the
  # corresponding *value* as a parameter.
  def each; end

  # Executes *block* for each key in the database, passing the *key* as a
  # parameter.
  def each_key; end

  # Executes *block* for each key in the database, passing the *key* and the
  # corresponding *value* as a parameter.
  def each_pair; end

  # Executes *block* for each key in the database, passing the corresponding
  # *value* as a parameter.
  def each_value; end

  # Returns true if the database is empty.
  def empty?; end

  # Turns the database's fast mode on or off. If fast mode is turned on, gdbm
  # does not wait for writes to be flushed to the disk before continuing.
  #
  # This option is obsolete for gdbm >= 1.8 since fast mode is turned on by
  # default. See also:
  # [`syncmode=`](https://docs.ruby-lang.org/en/2.6.0/GDBM.html#method-i-syncmode-3D)
  def fastmode=(_); end

  # Retrieves the *value* corresponding to *key*. If there is no value
  # associated with *key*, *default* will be returned instead.
  def fetch(*_); end

  # Returns true if the given key *k* exists within the database. Returns false
  # otherwise.
  def has_key?(_); end

  # Returns true if the given value *v* exists within the database. Returns
  # false otherwise.
  def has_value?(_); end

  # Returns true if the given key *k* exists within the database. Returns false
  # otherwise.
  def include?(_); end

  def index(_); end

  # Returns a hash created by using *gdbm*'s values as keys, and the keys as
  # values.
  def invert; end

  # Returns the *key* for a given *value*. If several keys may map to the same
  # value, the key that is found first will be returned.
  def key(_); end

  # Returns true if the given key *k* exists within the database. Returns false
  # otherwise.
  def key?(_); end

  # Returns an array of all keys of this database.
  def keys; end

  # Returns the number of key-value pairs in this database.
  def length; end

  # Returns true if the given key *k* exists within the database. Returns false
  # otherwise.
  def member?(_); end

  # Returns a hash copy of *gdbm* where all key-value pairs from *gdbm* for
  # which *block* evaluates to true are removed. See also:
  # [`delete_if`](https://docs.ruby-lang.org/en/2.6.0/GDBM.html#method-i-delete_if)
  def reject; end

  # Deletes every key-value pair from *gdbm* for which *block* evaluates to
  # true.
  def reject!; end

  # Reorganizes the database file. This operation removes reserved space of
  # elements that have already been deleted. It is only useful after a lot of
  # deletions in the database.
  def reorganize; end

  # Replaces the content of *gdbm* with the key-value pairs of *other*. *other*
  # must have an
  # [`each_pair`](https://docs.ruby-lang.org/en/2.6.0/GDBM.html#method-i-each_pair)
  # method.
  def replace(_); end

  # Returns a new array of all key-value pairs of the database for which *block*
  # evaluates to true.
  def select; end

  # Removes a key-value-pair from this database and returns it as a two-item
  # array [ *key*, *value* ]. Returns nil if the database is empty.
  def shift; end

  # Returns the number of key-value pairs in this database.
  def size; end

  # Associates the value *value* with the specified *key*.
  def store(_, _); end

  # Unless the *gdbm* object has been opened with the **SYNC** flag, it is not
  # guaranteed that database modification operations are immediately applied to
  # the database file. This method ensures that all recent modifications to the
  # database are written to the file. Blocks until all writing operations to the
  # disk have been finished.
  def sync; end

  # Turns the database's synchronization mode on or off. If the synchronization
  # mode is turned on, the database's in-memory state will be synchronized to
  # disk after every database modification operation. If the synchronization
  # mode is turned off, [`GDBM`](https://docs.ruby-lang.org/en/2.6.0/GDBM.html)
  # does not wait for writes to be flushed to the disk before continuing.
  #
  # This option is only available for gdbm >= 1.8 where syncmode is turned off
  # by default. See also:
  # [`fastmode=`](https://docs.ruby-lang.org/en/2.6.0/GDBM.html#method-i-fastmode-3D)
  def syncmode=(_); end

  # Returns an array of all key-value pairs contained in the database.
  def to_a; end

  # Returns a hash of all key-value pairs contained in the database.
  def to_hash; end

  # Adds the key-value pairs of *other* to *gdbm*, overwriting entries with
  # duplicate keys with those from *other*. *other* must have an
  # [`each_pair`](https://docs.ruby-lang.org/en/2.6.0/GDBM.html#method-i-each_pair)
  # method.
  def update(_); end

  # Returns true if the given value *v* exists within the database. Returns
  # false otherwise.
  def value?(_); end

  # Returns an array of all values of this database.
  def values; end

  # Returns an array of the values associated with each specified *key*.
  def values_at(*_); end

  # If called without a block, this is synonymous to
  # [`GDBM::new`](https://docs.ruby-lang.org/en/2.6.0/GDBM.html#method-c-new).
  # If a block is given, the new
  # [`GDBM`](https://docs.ruby-lang.org/en/2.6.0/GDBM.html) instance will be
  # passed to the block as a parameter, and the corresponding database file will
  # be closed after the execution of the block code has been finished.
  #
  # Example for an open call with a block:
  #
  # ```ruby
  # require 'gdbm'
  # GDBM.open("fruitstore.db") do |gdbm|
  #   gdbm.each_pair do |key, value|
  #     print "#{key}: #{value}\n"
  #   end
  # end
  # ```
  def self.open(*_); end
end

class GDBMError < ::StandardError; end

class GDBMFatalError < ::Exception; end
