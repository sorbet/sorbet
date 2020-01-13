# typed: __STDLIB_INTERNAL

# [`TSort`](https://docs.ruby-lang.org/en/2.6.0/TSort.html) implements
# topological sorting using Tarjan's algorithm for strongly connected
# components.
#
# [`TSort`](https://docs.ruby-lang.org/en/2.6.0/TSort.html) is designed to be
# able to be used with any object which can be interpreted as a directed graph.
#
# [`TSort`](https://docs.ruby-lang.org/en/2.6.0/TSort.html) requires two methods
# to interpret an object as a graph,
# [`tsort_each_node`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-tsort_each_node)
# and tsort\_each\_child.
#
# *   [`tsort_each_node`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-tsort_each_node)
#     is used to iterate for all nodes over a graph.
# *   [`tsort_each_child`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-tsort_each_child)
#     is used to iterate for child nodes of a given node.
#
#
# The equality of nodes are defined by eql? and hash since
# [`TSort`](https://docs.ruby-lang.org/en/2.6.0/TSort.html) uses
# [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) internally.
#
# ## A Simple Example
#
# The following example demonstrates how to mix the
# [`TSort`](https://docs.ruby-lang.org/en/2.6.0/TSort.html) module into an
# existing class (in this case,
# [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html)). Here, we're treating
# each key in the hash as a node in the graph, and so we simply alias the
# required
# [`tsort_each_node`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-tsort_each_node)
# method to Hash's each\_key method. For each key in the hash, the associated
# value is an array of the node's child nodes. This choice in turn leads to our
# implementation of the required
# [`tsort_each_child`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-tsort_each_child)
# method, which fetches the array of child nodes and then iterates over that
# array using the user-supplied block.
#
# ```ruby
# require 'tsort'
#
# class Hash
#   include TSort
#   alias tsort_each_node each_key
#   def tsort_each_child(node, &block)
#     fetch(node).each(&block)
#   end
# end
#
# {1=>[2, 3], 2=>[3], 3=>[], 4=>[]}.tsort
# #=> [3, 2, 1, 4]
#
# {1=>[2], 2=>[3, 4], 3=>[2], 4=>[]}.strongly_connected_components
# #=> [[4], [2, 3], [1]]
# ```
#
# ## A More Realistic Example
#
# A very simple 'make' like tool can be implemented as follows:
#
# ```ruby
# require 'tsort'
#
# class Make
#   def initialize
#     @dep = {}
#     @dep.default = []
#   end
#
#   def rule(outputs, inputs=[], &block)
#     triple = [outputs, inputs, block]
#     outputs.each {|f| @dep[f] = [triple]}
#     @dep[triple] = inputs
#   end
#
#   def build(target)
#     each_strongly_connected_component_from(target) {|ns|
#       if ns.length != 1
#         fs = ns.delete_if {|n| Array === n}
#         raise TSort::Cyclic.new("cyclic dependencies: #{fs.join ', '}")
#       end
#       n = ns.first
#       if Array === n
#         outputs, inputs, block = n
#         inputs_time = inputs.map {|f| File.mtime f}.max
#         begin
#           outputs_time = outputs.map {|f| File.mtime f}.min
#         rescue Errno::ENOENT
#           outputs_time = nil
#         end
#         if outputs_time == nil ||
#            inputs_time != nil && outputs_time <= inputs_time
#           sleep 1 if inputs_time != nil && inputs_time.to_i == Time.now.to_i
#           block.call
#         end
#       end
#     }
#   end
#
#   def tsort_each_child(node, &block)
#     @dep[node].each(&block)
#   end
#   include TSort
# end
#
# def command(arg)
#   print arg, "\n"
#   system arg
# end
#
# m = Make.new
# m.rule(%w[t1]) { command 'date > t1' }
# m.rule(%w[t2]) { command 'date > t2' }
# m.rule(%w[t3]) { command 'date > t3' }
# m.rule(%w[t4], %w[t1 t3]) { command 'cat t1 t3 > t4' }
# m.rule(%w[t5], %w[t4 t2]) { command 'cat t4 t2 > t5' }
# m.build('t5')
# ```
#
# ## Bugs
#
# *   'tsort.rb' is wrong name because this library uses Tarjan's algorithm for
#     strongly connected components. Although
#     'strongly\_connected\_components.rb' is correct but too long.
#
#
# ## References
#
#     1.  Tarjan, "Depth First Search and Linear Graph Algorithms",
#
#
# *SIAM Journal on Computing*, Vol. 1, No. 2, pp. 146-160, June 1972.
module TSort
  # The iterator version of the
  # [`TSort.strongly_connected_components`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-c-strongly_connected_components)
  # method.
  #
  # The graph is represented by *each\_node* and *each\_child*. *each\_node*
  # should have `call` method which yields for each node in the graph.
  # *each\_child* should have `call` method which takes a node argument and
  # yields for each child node.
  #
  # ```ruby
  # g = {1=>[2, 3], 2=>[4], 3=>[2, 4], 4=>[]}
  # each_node = lambda {|&b| g.each_key(&b) }
  # each_child = lambda {|n, &b| g[n].each(&b) }
  # TSort.each_strongly_connected_component(each_node, each_child) {|scc| p scc }
  # #=> [4]
  # #   [2]
  # #   [3]
  # #   [1]
  #
  # g = {1=>[2], 2=>[3, 4], 3=>[2], 4=>[]}
  # each_node = lambda {|&b| g.each_key(&b) }
  # each_child = lambda {|n, &b| g[n].each(&b) }
  # TSort.each_strongly_connected_component(each_node, each_child) {|scc| p scc }
  # #=> [4]
  # #   [2, 3]
  # #   [1]
  # ```
  def self.each_strongly_connected_component(each_node, each_child); end
  # Iterates over strongly connected components in a graph. The graph is
  # represented by *node* and *each\_child*.
  #
  # *node* is the first node. *each\_child* should have `call` method which
  # takes a node argument and yields for each child node.
  #
  # Return value is unspecified.
  #
  # #TSort.each\_strongly\_connected\_component\_from is a class method and it
  # doesn't need a class to represent a graph which includes
  # [`TSort`](https://docs.ruby-lang.org/en/2.6.0/TSort.html).
  #
  # ```ruby
  # graph = {1=>[2], 2=>[3, 4], 3=>[2], 4=>[]}
  # each_child = lambda {|n, &b| graph[n].each(&b) }
  # TSort.each_strongly_connected_component_from(1, each_child) {|scc|
  #   p scc
  # }
  # #=> [4]
  # #   [2, 3]
  # #   [1]
  # ```
  def self.each_strongly_connected_component_from(node, each_child, id_map = nil, stack = nil); end
  # Returns strongly connected components as an array of arrays of nodes. The
  # array is sorted from children to parents. Each elements of the array
  # represents a strongly connected component.
  #
  # The graph is represented by *each\_node* and *each\_child*. *each\_node*
  # should have `call` method which yields for each node in the graph.
  # *each\_child* should have `call` method which takes a node argument and
  # yields for each child node.
  #
  # ```ruby
  # g = {1=>[2, 3], 2=>[4], 3=>[2, 4], 4=>[]}
  # each_node = lambda {|&b| g.each_key(&b) }
  # each_child = lambda {|n, &b| g[n].each(&b) }
  # p TSort.strongly_connected_components(each_node, each_child)
  # #=> [[4], [2], [3], [1]]
  #
  # g = {1=>[2], 2=>[3, 4], 3=>[2], 4=>[]}
  # each_node = lambda {|&b| g.each_key(&b) }
  # each_child = lambda {|n, &b| g[n].each(&b) }
  # p TSort.strongly_connected_components(each_node, each_child)
  # #=> [[4], [2, 3], [1]]
  # ```
  def self.strongly_connected_components(each_node, each_child); end
  # Returns a topologically sorted array of nodes. The array is sorted from
  # children to parents, i.e. the first element has no child and the last node
  # has no parent.
  #
  # The graph is represented by *each\_node* and *each\_child*. *each\_node*
  # should have `call` method which yields for each node in the graph.
  # *each\_child* should have `call` method which takes a node argument and
  # yields for each child node.
  #
  # If there is a cycle,
  # [`TSort::Cyclic`](https://docs.ruby-lang.org/en/2.6.0/TSort/Cyclic.html) is
  # raised.
  #
  # ```ruby
  # g = {1=>[2, 3], 2=>[4], 3=>[2, 4], 4=>[]}
  # each_node = lambda {|&b| g.each_key(&b) }
  # each_child = lambda {|n, &b| g[n].each(&b) }
  # p TSort.tsort(each_node, each_child) #=> [4, 2, 3, 1]
  #
  # g = {1=>[2], 2=>[3, 4], 3=>[2], 4=>[]}
  # each_node = lambda {|&b| g.each_key(&b) }
  # each_child = lambda {|n, &b| g[n].each(&b) }
  # p TSort.tsort(each_node, each_child) # raises TSort::Cyclic
  # ```
  def self.tsort(each_node, each_child); end
  # The iterator version of the
  # [`TSort.tsort`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-c-tsort)
  # method.
  #
  # The graph is represented by *each\_node* and *each\_child*. *each\_node*
  # should have `call` method which yields for each node in the graph.
  # *each\_child* should have `call` method which takes a node argument and
  # yields for each child node.
  #
  # ```ruby
  # g = {1=>[2, 3], 2=>[4], 3=>[2, 4], 4=>[]}
  # each_node = lambda {|&b| g.each_key(&b) }
  # each_child = lambda {|n, &b| g[n].each(&b) }
  # TSort.tsort_each(each_node, each_child) {|n| p n }
  # #=> 4
  # #   2
  # #   3
  # #   1
  # ```
  def self.tsort_each(each_node, each_child); end
  # The iterator version of the
  # [`strongly_connected_components`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-strongly_connected_components)
  # method. `obj.each_strongly_connected_component` is similar to
  # `obj.strongly_connected_components.each`, but modification of *obj* during
  # the iteration may lead to unexpected results.
  #
  # [`each_strongly_connected_component`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-each_strongly_connected_component)
  # returns `nil`.
  #
  # ```ruby
  # class G
  #   include TSort
  #   def initialize(g)
  #     @g = g
  #   end
  #   def tsort_each_child(n, &b) @g[n].each(&b) end
  #   def tsort_each_node(&b) @g.each_key(&b) end
  # end
  #
  # graph = G.new({1=>[2, 3], 2=>[4], 3=>[2, 4], 4=>[]})
  # graph.each_strongly_connected_component {|scc| p scc }
  # #=> [4]
  # #   [2]
  # #   [3]
  # #   [1]
  #
  # graph = G.new({1=>[2], 2=>[3, 4], 3=>[2], 4=>[]})
  # graph.each_strongly_connected_component {|scc| p scc }
  # #=> [4]
  # #   [2, 3]
  # #   [1]
  # ```
  def each_strongly_connected_component(&block); end
  # Iterates over strongly connected component in the subgraph reachable from
  # *node*.
  #
  # Return value is unspecified.
  #
  # [`each_strongly_connected_component_from`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-each_strongly_connected_component_from)
  # doesn't call
  # [`tsort_each_node`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-tsort_each_node).
  #
  # ```ruby
  # class G
  #   include TSort
  #   def initialize(g)
  #     @g = g
  #   end
  #   def tsort_each_child(n, &b) @g[n].each(&b) end
  #   def tsort_each_node(&b) @g.each_key(&b) end
  # end
  #
  # graph = G.new({1=>[2, 3], 2=>[4], 3=>[2, 4], 4=>[]})
  # graph.each_strongly_connected_component_from(2) {|scc| p scc }
  # #=> [4]
  # #   [2]
  #
  # graph = G.new({1=>[2], 2=>[3, 4], 3=>[2], 4=>[]})
  # graph.each_strongly_connected_component_from(2) {|scc| p scc }
  # #=> [4]
  # #   [2, 3]
  # ```
  def each_strongly_connected_component_from(node, id_map = nil, stack = nil, &block); end
  # Returns strongly connected components as an array of arrays of nodes. The
  # array is sorted from children to parents. Each elements of the array
  # represents a strongly connected component.
  #
  # ```ruby
  # class G
  #   include TSort
  #   def initialize(g)
  #     @g = g
  #   end
  #   def tsort_each_child(n, &b) @g[n].each(&b) end
  #   def tsort_each_node(&b) @g.each_key(&b) end
  # end
  #
  # graph = G.new({1=>[2, 3], 2=>[4], 3=>[2, 4], 4=>[]})
  # p graph.strongly_connected_components #=> [[4], [2], [3], [1]]
  #
  # graph = G.new({1=>[2], 2=>[3, 4], 3=>[2], 4=>[]})
  # p graph.strongly_connected_components #=> [[4], [2, 3], [1]]
  # ```
  def strongly_connected_components; end
  # Returns a topologically sorted array of nodes. The array is sorted from
  # children to parents, i.e. the first element has no child and the last node
  # has no parent.
  #
  # If there is a cycle,
  # [`TSort::Cyclic`](https://docs.ruby-lang.org/en/2.6.0/TSort/Cyclic.html) is
  # raised.
  #
  # ```ruby
  # class G
  #   include TSort
  #   def initialize(g)
  #     @g = g
  #   end
  #   def tsort_each_child(n, &b) @g[n].each(&b) end
  #   def tsort_each_node(&b) @g.each_key(&b) end
  # end
  #
  # graph = G.new({1=>[2, 3], 2=>[4], 3=>[2, 4], 4=>[]})
  # p graph.tsort #=> [4, 2, 3, 1]
  #
  # graph = G.new({1=>[2], 2=>[3, 4], 3=>[2], 4=>[]})
  # p graph.tsort # raises TSort::Cyclic
  # ```
  def tsort; end
  # The iterator version of the
  # [`tsort`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-tsort)
  # method. `obj.tsort_each` is similar to `obj.tsort.each`, but modification of
  # *obj* during the iteration may lead to unexpected results.
  #
  # [`tsort_each`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-tsort_each)
  # returns `nil`. If there is a cycle,
  # [`TSort::Cyclic`](https://docs.ruby-lang.org/en/2.6.0/TSort/Cyclic.html) is
  # raised.
  #
  # ```ruby
  # class G
  #   include TSort
  #   def initialize(g)
  #     @g = g
  #   end
  #   def tsort_each_child(n, &b) @g[n].each(&b) end
  #   def tsort_each_node(&b) @g.each_key(&b) end
  # end
  #
  # graph = G.new({1=>[2, 3], 2=>[4], 3=>[2, 4], 4=>[]})
  # graph.tsort_each {|n| p n }
  # #=> 4
  # #   2
  # #   3
  # #   1
  # ```
  def tsort_each(&block); end
  # Should be implemented by a extended class.
  #
  # [`tsort_each_child`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-tsort_each_child)
  # is used to iterate for child nodes of *node*.
  def tsort_each_child(node); end
  # Should be implemented by a extended class.
  #
  # [`tsort_each_node`](https://docs.ruby-lang.org/en/2.6.0/TSort.html#method-i-tsort_each_node)
  # is used to iterate for all nodes over a graph.
  def tsort_each_node; end
end

class TSort::Cyclic < StandardError
end
