# frozen_string_literal: true
# typed: true
# compiled: true

class Parent
  def pos(x, y, z)
    puts "pos: #{x}, #{y}, #{z}"
  end

  def pos_zsuperdo(x, y, z)
    yield "pos: #{x}, #{y}, #{z}"
  end

  def pos_kwargs(x, y, z, j:)
    puts "pos_kwargs: #{x}, #{y}, #{z}, j: #{j}"
  end

  def implicit_blk
    yield "from implicit_blk"
  end

  def splat_zsuperdo(*x)
    yield "splat_zsuperdo: #{x}"
  end

  def splat_kwsplat_zsuperdo(*x,**y)
    yield "splat_kwsplat_zsuperdo: x: #{x}, y: #{y}"
  end

  def splat_and_block(x, *y, &blk)
    yield "from splat_and_block: x #{x}, y #{y}"
  end

  def splat_and_block_and_kwargs(x, *y, j:, &blk)
    yield "from splat_and_block_and_kwargs: x #{x}, y #{y}, j: #{j}"
  end

  def splat_and_block_and_kwargs_and_kwargsplat(x, *y, j:, **kwargs, &blk)
    yield "from splat_and_block_and_kwargs_and_kwargsplat: x #{x}, y #{y}, j: #{j}, kwargs: #{kwargs}"
  end
end

class Child < Parent
  def pos(x, y, z)
    puts "in child"
    super
  end

  def pos_zsuperdo(x, y, z)
    puts "in child"
    super { |x| puts "In the block for pos_zsuperdo: #{x}" }
  end

  def pos_kwargs(x, y, z, j:)
    puts "in child"
    super
  end

  def splat_zsuperdo(*x)
    puts "in child"
    super { |x| puts "In the block for splat_zsuperdo: #{x}" }
  end

  def splat_kwsplat_zsuperdo(*x, **y)
    puts "in child"
    super { |x| puts "In the block for splat_kwsplat_zsuperdo: #{x}" }
  end

  def implicit_blk
    puts "in child"
    super
  end

  def splat_and_block(x, *y, &blk)
    puts "in child"
    super
  end

  def splat_and_block_and_kwargs(x, *y, j:, &blk)
    puts "in child"
    super
  end

  def splat_and_block_and_kwargs_and_kwargsplat(x, *y, j:, **kwargs, &blk)
    puts "in child"
    super
  end
end

c = Child.new

c.pos(1, 2, 3)
c.pos_zsuperdo(1, 2, 3)
c.pos_kwargs(4, 5, 6, j: 7)

c.splat_zsuperdo(1, 2, 3)
c.splat_kwsplat_zsuperdo(1, 2, 3, j: 33, q: 909)

c.implicit_blk { |x| puts "In the block passed to implicit_blk: #{x}" }
p = -> (x) { puts "In the lambda passed to implicit_blk: #{x}" }
c.implicit_blk(&p)

c.splat_and_block(3, "hello", "world", 22) { |x| puts "In the block passed to splat_and_block: #{x}"}
p = -> (x) { puts "In the lambda passed to splat_and_block: #{x}" }
c.splat_and_block(3, "hello", "world", 22, &p)

c.splat_and_block_and_kwargs(3, "hello", "world", 22, j: 33) { |x| puts "In the block passed to splat_and_block_and_kwargs: #{x}"}
p = -> (x) { puts "In the lambda passed to splat_and_block_and_kwargs: #{x}" }
c.splat_and_block_and_kwargs(3, "hello", "world", 22, j: 33, &p)

c.splat_and_block_and_kwargs_and_kwargsplat(3, "hello", "world", 22, j: 33, k: "howdy") { |x| puts "In the block passed to splat_and_block_and_kwargs_and_kwargsplat: #{x}"}
p = -> (x) { puts "In the lambda passed to splat_and_block_and_kwargs_and_kwargsplat: #{x}" }
c.splat_and_block_and_kwargs_and_kwargsplat(3, "hello", "world", 22, j: 33, k: "howdy", &p)
