# typed: true

class Depth2
  # Parent with really good documentation
  def duplicate_method_1; end
  # Parent with really good documentation
  def duplicate_method_2; end
end

class Depth1 < Depth2
  def duplicate_method_1; end
  def duplicate_method_2; end
end

# We want to only show the method with that name at the lowest depth.
# Note this isn't captured by the test, but we currently show the docs from
# the child, even if the parent's docs would be better to show.

Depth1.new.duplicate_ # error: does not exist
#                    ^ completion: duplicate_method_1, duplicate_method_2
