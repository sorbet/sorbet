# typed: true

class Module; include T::Sig; end
extend T::Sig

class AbstractTreeNode
  extend T::Helpers
  abstract!
  sealed!

  sig { returns(T::Array[String]) }
  def self.get_leaf_nodes
    if self <= AbstractBranchNode
      res = self.left.get_leaf_nodes + self.right.get_leaf_nodes
      T.reveal_type(res) # error: `T::Array[String]`
      return res
    elsif self <= AbstractLeafNode
      res =  Array[self.value]
      T.reveal_type(res) # error: `T::Array[String]`
      return res
    else
      # There must be an error here, because we have not handled the singleton
      # class object itself, which necessarily exists, despite the class being
      # abstract (e.g., abstract singleton classes are an abomination)
      T.absurd(self) # error: Control flow could reach `T.absurd` because the type `T.self_type (of T.class_of(AbstractTreeNode))` wasn't handled
    end
  end
end

class AbstractBranchNode < AbstractTreeNode
  abstract!
  sealed!

  class << self
    sig { returns(T.class_of(AbstractTreeNode)) }
    attr_reader :left, :right
  end
end

class AbstractLeafNode < AbstractTreeNode
  abstract!
  sealed!

  class << self
    sig { returns(String) }
    attr_reader :value
  end
end

hello = Class.new(AbstractLeafNode) do
  @value = "hello"
end
world = Class.new(AbstractLeafNode) do
  @value = "world"
end
node = Class.new(AbstractBranchNode) do
  @left = hello
  @right = world
end
res = node.get_leaf_nodes.join(' ')
T.reveal_type(res) # error: `String`
puts(res)
