# typed: true

def foo; end

class Test
  def using_symbol
    1
  end
  private :using_symbol

  private def using_symbol_returned_by_def
    2
  end

  private def splat_call(*args)
    puts args
  end

  private def splat_and_block_call(foo, bar, &blk)
  end

  private def block_call(&blk)
  end

  def calling_private
    using_symbol
    self.using_symbol
    assigned_self = self
    assigned_self.using_symbol # error: Non-private call to private method `using_symbol`
  end

  private
  def subsequent_visibility
    3
  end
end

class TestChild < Test
  def calling_private_in_parent
    using_symbol
    self.using_symbol
    assigned_self = self
    assigned_self.using_symbol # error: Non-private call to private method `using_symbol`
  end
end

Object.new.foo # error: Non-private call to private method `foo`

Test.new.using_symbol # error: Non-private call to private method `using_symbol`
Test.new.using_symbol_returned_by_def # error: Non-private call to private method `using_symbol_returned_by_def`
Test.new.calling_private
Test.new.using_symbol { 123 } # error: Non-private call to private method `using_symbol`
Test.new.block_call # error: Non-private call to private method `block_call`
Test.new.block_call { 123 } # error: Non-private call to private method `block_call`
Test.new.block_call(&:foo) # error: Non-private call to private method `block_call`

TestChild.new.using_symbol # error: Non-private call to private method `using_symbol`

T.unsafe(Test.new).using_symbol

# TODO: The following methods should also error out. They currently do not do that due to issues with the instrinsics.
Test.new.splat_call(*T.unsafe(nil)) # Currently no Error, since T.unsafe makes Magic_callWithSplat#apply return prematurely due to the untyped argument.
Test.new.splat_and_block_call(*[1, 'a'], &nil) # Currently no Error, since the nil block makes Magic_callWithSplatAndBlock return prematurely.
Test.new.block_call(&nil) # Currently no Error, since the nil block makes Magic_callWithBlock return prematurely.

# TODO: The following method should contain an error. Sorbet currently does not support setting method
# visibility using the private/protected keywords that affect the visibility of subsequent methods.
Test.new.subsequent_visibility
