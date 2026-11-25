# typed: false

# `MultiTargetNode` in the `lefts` of a `MultiWriteNode`
(x, y), a = []
a, (x, y) = []

# `MultiTargetNode` in the `rest` of a `MultiWriteNode` is a syntax error
# *(x, y) = []
# ^~~~~~ unexpected write target
# ^~~~~~ unexpected write target

# `MultiTargetNode` in the `rights` of a `MultiWriteNode`
*head, (x, y), a = []
*head, a, (x, y) = []

Proc.new do |p1, (x2, y2)|
end

for p1, (x2, y2) in a
end

p2, (head, *) = [[1, 2], [3, 4]]
p3, (head, *tail) = [[1, 2], [3, 4]]

p4, (*, last) = [[1, 2], [3, 4]]
p5, (*init, last) = [[1, 2], [3, 4]]

# Implicit rest node

p6, (x2,) = [[1, 2], [3, 4]]

# Proc.new do |p, (x2,)| # Not valid Ruby
# end

for p7, (x2,) in a
end

# `MultiTargetNode` in method parameter lists

def method_empty((x, y))
end

def method_single((x, y))
  statement1()
end

def method_multi((x, y))
  statement1()
  statement2()
end


def self.method_empty((x, y))
end

def self.method_single((x, y))
  statement1()
end

def self.method_multi((x, y))
  statement1()
  statement2()
end

# `MultiTargetNode` in block parameter lists\

block_empty do |(x, y)|
end

block_single do |(x, y)|
   statement1()
end

block_multi do |(x, y)|
   statement1()
   statement2()
end

# `MultiTargetNode` in lambda parameter lists

-> ((x, y)) {}
-> ((x, y)) { statement1() }
-> ((x, y)) { statement1(); statement2() }
