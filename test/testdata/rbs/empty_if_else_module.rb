# typed: false
# enable-experimental-rbs-comments: true

# This test replicates a crash that occurred in CommentsAssociator.cc when processing rake files.
# The crash happens when:
# 1. We're inside a class/module (typeAliasAllowedInContext() returns true)
# 2. There's an if/unless with an empty branch (body=nullptr, node=nullptr)
# 3. There are RBS comments that trigger tree walking
#
# The RBS type alias comment below is needed to trigger tree walking.
# Without RBS comments, the CommentsAssociator short-circuits and skips the tree.

module EmptyIfInModule
  #: type a = Integer

  # Empty then branch - if_->then_ is nullptr
  # This crashes when walkBody is called with node=nullptr
  if ARGV.empty?
  else
    STDERR.puts "not empty"
  end
end

module EmptyElseInModule
  #: type b = String

  # Empty else branch - if_->else_ is nullptr
  if ARGV.any?
    STDERR.puts "any"
  else
  end
end
