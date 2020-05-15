# typed: __STDLIB_INTERNAL

# [`SDBM`](https://docs.ruby-lang.org/en/2.6.0/SDBM.html) provides a simple
# file-based key-value store, which can only store
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) keys and values.
#
# Note that Ruby comes with the source code for
# [`SDBM`](https://docs.ruby-lang.org/en/2.6.0/SDBM.html), while the
# [`DBM`](https://docs.ruby-lang.org/en/2.6.0/DBM.html) and
# [`GDBM`](https://docs.ruby-lang.org/en/2.6.0/GDBM.html) standard libraries
# rely on external libraries and headers.
#
# ### Examples
#
# Insert values:
#
# ```ruby
# require 'sdbm'
#
# SDBM.open 'my_database' do |db|
#   db['apple'] = 'fruit'
#   db['pear'] = 'fruit'
#   db['carrot'] = 'vegetable'
#   db['tomato'] = 'vegetable'
# end
# ```
#
# Bulk update:
#
# ```ruby
# require 'sdbm'
#
# SDBM.open 'my_database' do |db|
#   db.update('peach' => 'fruit', 'tomato' => 'fruit')
# end
# ```
#
# Retrieve values:
#
# ```ruby
# require 'sdbm'
#
# SDBM.open 'my_database' do |db|
#   db.each do |key, value|
#     puts "Key: #{key}, Value: #{value}"
#   end
# end
# ```
#
# Outputs:
#
# ```
# Key: apple, Value: fruit
# Key: pear, Value: fruit
# Key: carrot, Value: vegetable
# Key: peach, Value: fruit
# Key: tomato, Value: fruit
# ```
class SDBM
  include(::Enumerable)

  Elem = type_member(:out)

  # Creates a new database handle by opening the given `filename`.
  # [`SDBM`](https://docs.ruby-lang.org/en/2.6.0/SDBM.html) actually uses two
  # physical files, with extensions '.dir' and '.pag'. These extensions will
  # automatically be appended to the `filename`.
  #
  # If the file does not exist, a new file will be created using the given
  # `mode`, unless `mode` is explicitly set to nil. In the latter case, no
  # database will be created.
  #
  # If the file exists, it will be opened in read/write mode. If this fails, it
  # will be opened in read-only mode.
  def self.new(*_); end

  # Returns the `value` in the database associated with the given `key` string.
  #
  # If no value is found, returns `nil`.
  def [](_); end

  # Stores a new `value` in the database with the given `key` as an index.
  #
  # If the `key` already exists, this will update the `value` associated with
  # the `key`.
  #
  # Returns the given `value`.
  def []=(_, _); end

  # Deletes all data from the database.
  def clear; end

  # Closes the database file.
  #
  # Raises [`SDBMError`](https://docs.ruby-lang.org/en/2.6.0/SDBMError.html) if
  # the database is already closed.
  def close; end

  # Returns `true` if the database is closed.
  def closed?; end

  # Deletes the key-value pair corresponding to the given `key`. If the `key`
  # exists, the deleted value will be returned, otherwise `nil`.
  #
  # If a block is provided, the deleted `key` and `value` will be passed to the
  # block as arguments. If the `key` does not exist in the database, the value
  # will be `nil`.
  def delete(_); end

  # Iterates over the key-value pairs in the database, deleting those for which
  # the block returns `true`.
  def delete_if; end

  # Iterates over each key-value pair in the database.
  #
  # If no block is given, returns an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html).
  def each; end

  # Iterates over each `key` in the database.
  #
  # If no block is given, returns an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html).
  def each_key; end

  # Iterates over each key-value pair in the database.
  #
  # If no block is given, returns an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html).
  def each_pair; end

  # Iterates over each `value` in the database.
  #
  # If no block is given, returns an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html).
  def each_value; end

  # Returns `true` if the database is empty.
  def empty?; end

  # Returns the `value` in the database associated with the given `key` string.
  #
  # If a block is provided, the block will be called when there is no `value`
  # associated with the given `key`. The `key` will be passed in as an argument
  # to the block.
  #
  # If no block is provided and no value is associated with the given `key`,
  # then an [`IndexError`](https://docs.ruby-lang.org/en/2.6.0/IndexError.html)
  # will be raised.
  def fetch(*_); end

  # Returns `true` if the database contains the given `key`.
  def has_key?(_); end

  # Returns `true` if the database contains the given `value`.
  def has_value?(_); end

  # Returns `true` if the database contains the given `key`.
  def include?(_); end

  def index(_); end

  # Returns a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) in which
  # the key-value pairs have been inverted.
  #
  # Example:
  #
  # ```ruby
  # require 'sdbm'
  #
  # SDBM.open 'my_database' do |db|
  #   db.update('apple' => 'fruit', 'spinach' => 'vegetable')
  #
  #   db.invert  #=> {"fruit" => "apple", "vegetable" => "spinach"}
  # end
  # ```
  def invert; end

  # Returns the `key` associated with the given `value`. If more than one `key`
  # corresponds to the given `value`, then the first key to be found will be
  # returned. If no keys are found, `nil` will be returned.
  def key(_); end

  # Returns `true` if the database contains the given `key`.
  def key?(_); end

  # Returns a new [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
  # containing the keys in the database.
  def keys; end

  # Returns the number of keys in the database.
  def length; end

  # Returns `true` if the database contains the given `key`.
  def member?(_); end

  # Creates a new [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) using
  # the key-value pairs from the database, then calls
  # [`Hash#reject`](https://docs.ruby-lang.org/en/2.6.0/Hash.html#method-i-reject)
  # with the given block, which returns a
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) with only the
  # key-value pairs for which the block returns `false`.
  def reject; end

  # Iterates over the key-value pairs in the database, deleting those for which
  # the block returns `true`.
  def reject!; end

  # Empties the database, then inserts the given key-value pairs.
  #
  # This method will work with any object which implements an
  # [`each_pair`](https://docs.ruby-lang.org/en/2.6.0/SDBM.html#method-i-each_pair)
  # method, such as a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html).
  def replace(_); end

  # Returns a new [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # key-value pairs for which the block returns `true`.
  #
  # Example:
  #
  # ```ruby
  # require 'sdbm'
  #
  # SDBM.open 'my_database' do |db|
  #   db['apple'] = 'fruit'
  #   db['pear'] = 'fruit'
  #   db['spinach'] = 'vegetable'
  #
  #   veggies = db.select do |key, value|
  #     value == 'vegetable'
  #   end #=> [["apple", "fruit"], ["pear", "fruit"]]
  # end
  # ```
  def select; end

  # Removes a key-value pair from the database and returns them as an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html). If the database
  # is empty, returns `nil`.
  def shift; end

  # Returns the number of keys in the database.
  def size; end

  # Stores a new `value` in the database with the given `key` as an index.
  #
  # If the `key` already exists, this will update the `value` associated with
  # the `key`.
  #
  # Returns the given `value`.
  def store(_, _); end

  # Returns a new [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
  # containing each key-value pair in the database.
  #
  # Example:
  #
  # ```ruby
  # require 'sdbm'
  #
  # SDBM.open 'my_database' do |db|
  #   db.update('apple' => 'fruit', 'spinach' => 'vegetable')
  #
  #   db.to_a  #=> [["apple", "fruit"], ["spinach", "vegetable"]]
  # end
  # ```
  def to_a; end

  # Returns a new [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html)
  # containing each key-value pair in the database.
  def to_hash; end

  # Insert or update key-value pairs.
  #
  # This method will work with any object which implements an
  # [`each_pair`](https://docs.ruby-lang.org/en/2.6.0/SDBM.html#method-i-each_pair)
  # method, such as a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html).
  def update(_); end

  # Returns `true` if the database contains the given `value`.
  def value?(_); end

  # Returns a new [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
  # containing the values in the database.
  def values; end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # values corresponding to the given keys.
  def values_at(*_); end

  # If called without a block, this is the same as
  # [`SDBM.new`](https://docs.ruby-lang.org/en/2.6.0/SDBM.html#method-c-new).
  #
  # If a block is given, the new database will be passed to the block and will
  # be safely closed after the block has executed.
  #
  # Example:
  #
  # ```ruby
  # require 'sdbm'
  #
  # SDBM.open('my_database') do |db|
  #   db['hello'] = 'world'
  # end
  # ```
  def self.open(*_); end
end

# [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) class used
# to return errors from the sdbm library.
class SDBMError < ::StandardError; end
