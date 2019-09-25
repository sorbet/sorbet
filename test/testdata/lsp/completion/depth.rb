# typed: true

class Depth2
  def duplicate_method_1; end
  def duplicate_method_2; end
end

class Depth1 < Depth2
  def duplicate_method_1; end
  def duplicate_method_2; end
end

# This test is designed to capture existing behavior, not test that we don't
# regress this behavior. If this changes, it's fine.

Depth1.new.duplicate_ # error: does not exist
#                    ^ completion: duplicate_method_1, duplicate_method_2, duplicate_method_1, duplicate_method_2
