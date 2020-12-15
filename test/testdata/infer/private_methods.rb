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
    assigned_self.using_symbol # error: Non-private call to private method `Test#using_symbol`
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
    assigned_self.using_symbol # error: Non-private call to private method `Test#using_symbol`
  end
end

class SelfReferentialPrivateMethodInvocationTest
  self.private
  def subsequent_visibility
  end
end

Object.new.foo # error: Non-private call to private method `Object#foo`

Test.new.using_symbol # error: Non-private call to private method `Test#using_symbol`
Test.new.using_symbol_returned_by_def # error: Non-private call to private method `Test#using_symbol_returned_by_def`
Test.new.calling_private
Test.new.using_symbol { 123 } # error: Non-private call to private method `Test#using_symbol`
Test.new.block_call # error: Non-private call to private method `Test#block_call`
Test.new.block_call { 123 } # error: Non-private call to private method `Test#block_call`
Test.new.block_call(&:foo) # error: Non-private call to private method `Test#block_call`

TestChild.new.using_symbol # error: Non-private call to private method `Test#using_symbol`

T.unsafe(Test.new).using_symbol

# TODO: The following methods should also error out. They currently do not do that due to issues with the instrinsics.
Test.new.splat_call(*T.unsafe(nil)) # Currently no Error, since T.unsafe makes Magic_callWithSplat#apply return prematurely due to the untyped argument.
Test.new.splat_and_block_call(*[1, 'a'], &nil) # Currently no Error, since the nil block makes Magic_callWithSplatAndBlock return prematurely.
Test.new.block_call(&nil) # Currently no Error, since the nil block makes Magic_callWithBlock return prematurely.

Test.new.subsequent_visibility # error: Non-private call to private method `Test#subsequent_visibility`
Test.new.subsequent_visibility_attr_reader # error: Non-private call to private method `Test#subsequent_visibility_attr_reader`

# This test case is not yet handled and will be addressed in a future release.
SelfReferentialPrivateMethodInvocationTest.new.subsequent_visibility

