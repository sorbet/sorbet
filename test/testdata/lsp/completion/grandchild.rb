# typed: true

# The aaa/bbb/ccc bits would cause the completion items to be sorted
# alphabetically (and thus incorrectly) by an LSP client, but we set an
# explicit `sortText` on each item to override this behavior, and ensure that
# our heuristics are in sole control of the sort order.

class Parent
  def method_from_aaa_parent; end
end

class Child < Parent
  def method_from_bbb_child; end
end

class GrandChild < Child
  def method_from_ccc_grand_child; end
end

GrandChild.new.method_from_ # error: does not exist
#                          ^ completion: method_from_ccc_grand_child, method_from_bbb_child, method_from_aaa_parent
