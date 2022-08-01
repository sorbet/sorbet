# typed: __STDLIB_INTERNAL

# The `Find` module supports the top-down traversal of a set of file paths.
#
# For example, to total the size of all files under your home directory,
# ignoring anything in a "dot" directory (e.g. $HOME/.ssh):
#
# ```ruby
# require 'find'
#
# total_size = 0
#
# Find.find(ENV["HOME"]) do |path|
#   if FileTest.directory?(path)
#     if File.basename(path)[0] == ?.
#       Find.prune       # Don't look any further into this directory.
#     else
#       next
#     end
#   else
#     total_size += FileTest.size(path)
#   end
# end
# ```
module Find
  class << self
    # Calls the associated block with the name of every file and directory listed
    # as arguments, then recursively on their subdirectories, and so on.
    #
    # Returns an enumerator if no block is given.
    #
    # See the `Find` module documentation for an example.
    sig do
      params(
        paths: String,
        ignore_error: T::Boolean,
      ).returns(T::Enumerator[String])
    end
    sig do
      params(
        paths: String,
        ignore_error: T::Boolean,
        blk: T.proc.params(path: String).void,
      ).void
    end
    def find(*paths, ignore_error: true, &blk); end
  end

  # Skips the current file or directory, restarting the loop with the next
  # entry. If the current file is a directory, that directory will not be
  # recursively entered. Meaningful only within the block associated with
  # [`Find::find`](https://docs.ruby-lang.org/en/2.6.0/Find.html#method-c-find).
  #
  # See the `Find` module documentation for an example.
  sig { void }
  module_function def prune; end
end
