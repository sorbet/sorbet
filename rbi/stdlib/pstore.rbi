# typed: __STDLIB_INTERNAL

# [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html) implements a file
# based persistence mechanism based on a
# [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html). User code can store
# hierarchies of Ruby objects (values) into the data store file by name (keys).
# An object hierarchy may be just a single object. User code may later read
# values back from the data store or even update data, as needed.
#
# The transactional behavior ensures that any changes succeed or fail together.
# This can be used to ensure that the data store is not left in a transitory
# state, where some values were updated but others were not.
#
# Behind the scenes, Ruby objects are stored to the data store file with
# Marshal. That carries the usual limitations. Proc objects cannot be
# marshalled, for example.
#
# ## Usage example:
#
# ```ruby
# require "pstore"
#
# # a mock wiki object...
# class WikiPage
#   def initialize( page_name, author, contents )
#     @page_name = page_name
#     @revisions = Array.new
#
#     add_revision(author, contents)
#   end
#
#   attr_reader :page_name
#
#   def add_revision( author, contents )
#     @revisions << { :created  => Time.now,
#                     :author   => author,
#                     :contents => contents }
#   end
#
#   def wiki_page_references
#     [@page_name] + @revisions.last[:contents].scan(/\b(?:[A-Z]+[a-z]+){2,}/)
#   end
#
#   # ...
# end
#
# # create a new page...
# home_page = WikiPage.new( "HomePage", "James Edward Gray II",
#                           "A page about the JoysOfDocumentation..." )
#
# # then we want to update page data and the index together, or not at all...
# wiki = PStore.new("wiki_pages.pstore")
# wiki.transaction do  # begin transaction; do all of this or none of it
#   # store page...
#   wiki[home_page.page_name] = home_page
#   # ensure that an index has been created...
#   wiki[:wiki_index] ||= Array.new
#   # update wiki index...
#   wiki[:wiki_index].push(*home_page.wiki_page_references)
# end                   # commit changes to wiki data store file
#
# ### Some time later... ###
#
# # read wiki data...
# wiki.transaction(true) do  # begin read-only transaction, no changes allowed
#   wiki.roots.each do |data_root_name|
#     p data_root_name
#     p wiki[data_root_name]
#   end
# end
# ```
#
# ## Transaction modes
#
# By default, file integrity is only ensured as long as the operating system
# (and the underlying hardware) doesn't raise any unexpected I/O errors. If an
# I/O error occurs while
# [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html) is writing to its
# file, then the file will become corrupted.
#
# You can prevent this by setting *pstore.ultra\_safe = true*. However, this
# results in a minor performance loss, and only works on platforms that support
# atomic file renames. Please consult the documentation for `ultra_safe` for
# details.
#
# Needless to say, if you're storing valuable data with
# [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html), then you should
# backup the [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html) files
# from time to time.
class PStore
  # Constant for relieving Ruby's garbage collector.
  CHECKSUM_ALGO = Digest::SHA512

  EMPTY_MARSHAL_CHECKSUM = T.let(T.unsafe(nil), String)

  EMPTY_MARSHAL_DATA = T.let(T.unsafe(nil), String)

  EMPTY_STRING = T.let(T.unsafe(nil), String)

  RDWR_ACCESS = T.let(T.unsafe(nil), T::Hash[Symbol, T.untyped])

  RD_ACCESS = T.let(T.unsafe(nil), T::Hash[Symbol, T.untyped])

  WR_ACCESS = T.let(T.unsafe(nil), T::Hash[Symbol, T.untyped])

  # Retrieves a value from the
  # [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html) file data, by
  # *name*. The hierarchy of Ruby objects stored under that root *name* will be
  # returned.
  #
  # **WARNING**:  This method is only valid in a
  # [`PStore#transaction`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-transaction).
  # It will raise
  # [`PStore::Error`](https://docs.ruby-lang.org/en/2.6.0/PStore/Error.html) if
  # called at any other time.
  def [](name); end

  # Stores an individual Ruby object or a hierarchy of Ruby objects in the data
  # store file under the root *name*. Assigning to a *name* already in the data
  # store clobbers the old data.
  #
  # ## Example:
  #
  # ```ruby
  # require "pstore"
  #
  # store = PStore.new("data_file.pstore")
  # store.transaction do  # begin transaction
  #   # load some data into the store...
  #   store[:single_object] = "My data..."
  #   store[:obj_hierarchy] = { "Kev Jackson" => ["rational.rb", "pstore.rb"],
  #                             "James Gray"  => ["erb.rb", "pstore.rb"] }
  # end                   # commit changes to data store file
  # ```
  #
  # **WARNING**:  This method is only valid in a
  # [`PStore#transaction`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-transaction)
  # and it cannot be read-only. It will raise
  # [`PStore::Error`](https://docs.ruby-lang.org/en/2.6.0/PStore/Error.html) if
  # called at any other time.
  def []=(name, value); end

  # Ends the current
  # [`PStore#transaction`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-transaction),
  # discarding any changes to the data store.
  #
  # ## Example:
  #
  # ```ruby
  # require "pstore"
  #
  # store = PStore.new("data_file.pstore")
  # store.transaction do  # begin transaction
  #   store[:one] = 1     # this change is not applied, see below...
  #   store[:two] = 2     # this change is not applied, see below...
  #
  #   store.abort         # end transaction here, discard all changes
  #
  #   store[:three] = 3   # this change is never reached
  # end
  # ```
  #
  # **WARNING**:  This method is only valid in a
  # [`PStore#transaction`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-transaction).
  # It will raise
  # [`PStore::Error`](https://docs.ruby-lang.org/en/2.6.0/PStore/Error.html) if
  # called at any other time.
  def abort; end

  # Ends the current
  # [`PStore#transaction`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-transaction),
  # committing any changes to the data store immediately.
  #
  # ## Example:
  #
  # ```ruby
  # require "pstore"
  #
  # store = PStore.new("data_file.pstore")
  # store.transaction do  # begin transaction
  #   # load some data into the store...
  #   store[:one] = 1
  #   store[:two] = 2
  #
  #   store.commit        # end transaction here, committing changes
  #
  #   store[:three] = 3   # this change is never reached
  # end
  # ```
  #
  # **WARNING**:  This method is only valid in a
  # [`PStore#transaction`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-transaction).
  # It will raise
  # [`PStore::Error`](https://docs.ruby-lang.org/en/2.6.0/PStore/Error.html) if
  # called at any other time.
  def commit; end

  # Removes an object hierarchy from the data store, by *name*.
  #
  # **WARNING**:  This method is only valid in a
  # [`PStore#transaction`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-transaction)
  # and it cannot be read-only. It will raise
  # [`PStore::Error`](https://docs.ruby-lang.org/en/2.6.0/PStore/Error.html) if
  # called at any other time.
  def delete(name); end

  # This method is just like
  # [`PStore#[]`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-5B-5D),
  # save that you may also provide a *default* value for the object. In the
  # event the specified *name* is not found in the data store, your *default*
  # will be returned instead. If you do not specify a default,
  # [`PStore::Error`](https://docs.ruby-lang.org/en/2.6.0/PStore/Error.html)
  # will be raised if the object is not found.
  #
  # **WARNING**:  This method is only valid in a
  # [`PStore#transaction`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-transaction).
  # It will raise
  # [`PStore::Error`](https://docs.ruby-lang.org/en/2.6.0/PStore/Error.html) if
  # called at any other time.
  def fetch(name, default = PStore::Error); end

  # Returns the path to the data store file.
  def path; end

  # Returns true if the supplied *name* is currently in the data store.
  #
  # **WARNING**:  This method is only valid in a
  # [`PStore#transaction`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-transaction).
  # It will raise
  # [`PStore::Error`](https://docs.ruby-lang.org/en/2.6.0/PStore/Error.html) if
  # called at any other time.
  def root?(name); end

  # Returns the names of all object hierarchies currently in the store.
  #
  # **WARNING**:  This method is only valid in a
  # [`PStore#transaction`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-transaction).
  # It will raise
  # [`PStore::Error`](https://docs.ruby-lang.org/en/2.6.0/PStore/Error.html) if
  # called at any other time.
  def roots; end

  # Opens a new transaction for the data store. Code executed inside a block
  # passed to this method may read and write data to and from the data store
  # file.
  #
  # At the end of the block, changes are committed to the data store
  # automatically. You may exit the transaction early with a call to either
  # [`PStore#commit`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-commit)
  # or
  # [`PStore#abort`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-abort).
  # See those methods for details about how changes are handled. Raising an
  # uncaught [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html)
  # in the block is equivalent to calling
  # [`PStore#abort`](https://docs.ruby-lang.org/en/2.6.0/PStore.html#method-i-abort).
  #
  # If *read\_only* is set to `true`, you will only be allowed to read from the
  # data store during the transaction and any attempts to change the data will
  # raise a
  # [`PStore::Error`](https://docs.ruby-lang.org/en/2.6.0/PStore/Error.html).
  #
  # Note that [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html) does
  # not support nested transactions.
  def transaction(read_only = false); end

  # Whether [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html) should
  # do its best to prevent file corruptions, even when under unlikely-to-occur
  # error conditions such as out-of-space conditions and other unusual OS
  # filesystem errors. Setting this flag comes at the price in the form of a
  # performance loss.
  #
  # This flag only has effect on platforms on which file renames are atomic
  # (e.g. all POSIX platforms: Linux, MacOS X, FreeBSD, etc). The default value
  # is false.
  def ultra_safe; end

  # Whether [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html) should
  # do its best to prevent file corruptions, even when under unlikely-to-occur
  # error conditions such as out-of-space conditions and other unusual OS
  # filesystem errors. Setting this flag comes at the price in the form of a
  # performance loss.
  #
  # This flag only has effect on platforms on which file renames are atomic
  # (e.g. all POSIX platforms: Linux, MacOS X, FreeBSD, etc). The default value
  # is false.
  def ultra_safe=(_); end

  # To construct a [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html)
  # object, pass in the *file* path where you would like the data to be stored.
  #
  # [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html) objects are
  # always reentrant. But if *thread\_safe* is set to true, then it will become
  # thread-safe at the cost of a minor performance hit.
  def self.new(file, thread_safe = false); end
end

# The error type thrown by all
# [`PStore`](https://docs.ruby-lang.org/en/2.6.0/PStore.html) methods.
class PStore::Error < ::StandardError; end
