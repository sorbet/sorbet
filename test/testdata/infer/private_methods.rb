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
    assigned_self.using_symbol # error: Non-private call to private method `using_symbol` on `Test`
  end

  private
  def subsequent_visibility
    3
  end

  attr_reader :subsequent_visibility_attr_reader
end

class TestChild < Test
  def calling_private_in_parent
    using_symbol
    self.using_symbol
    assigned_self = self
    assigned_self.using_symbol # error: Non-private call to private method `using_symbol` on `TestChild`
  end
end

Object.new.foo # error: Non-private call to private method `foo` on `Object`

Test.new.using_symbol # error: Non-private call to private method `using_symbol` on `Test`
Test.new.using_symbol_returned_by_def # error: Non-private call to private method `using_symbol_returned_by_def` on `Test`
Test.new.calling_private
Test.new.using_symbol { 123 } # error: Non-private call to private method `using_symbol` on `Test`
Test.new.block_call # error: Non-private call to private method `block_call` on `Test`
Test.new.block_call { 123 } # error: Non-private call to private method `block_call` on `Test`
Test.new.block_call(&:foo) # error: Non-private call to private method `block_call` on `Test`

TestChild.new.using_symbol # error: Non-private call to private method `using_symbol` on `TestChild`

T.unsafe(Test.new).using_symbol

# TODO: The following call should also error out. It currently does not do that due to issues with the intrinsics.
Test.new.splat_call(*T.unsafe(nil)) # Currently no Error, since T.unsafe makes Magic_callWithSplat#apply return prematurely due to the untyped argument.

Test.new.splat_and_block_call(*[1, 'a'], &nil) # error: Non-private call to private method `splat_and_block_call` on `Test`
Test.new.block_call(&nil) # error: Non-private call to private method `block_call` on `Test`

Test.new.subsequent_visibility # error: Non-private call to private method `subsequent_visibility`
Test.new.subsequent_visibility_attr_reader # error: Non-private call to private method `subsequent_visibility_attr_reader`
