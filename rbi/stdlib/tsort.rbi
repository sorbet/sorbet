# typed: strict
module TSort
  def self.each_strongly_connected_component(each_node, each_child); end
  def self.each_strongly_connected_component_from(node, each_child, id_map = nil, stack = nil); end
  def self.strongly_connected_components(each_node, each_child); end
  def self.tsort(each_node, each_child); end
  def self.tsort_each(each_node, each_child); end
  def each_strongly_connected_component(&block); end
  def each_strongly_connected_component_from(node, id_map = nil, stack = nil, &block); end
  def strongly_connected_components; end
  def tsort; end
  def tsort_each(&block); end
  def tsort_each_child(node); end
  def tsort_each_node; end
end
class TSort::Cyclic < StandardError
end
