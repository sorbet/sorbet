# typed: true

class SelfReferentialPrivateMethodInvocationTest
  self.private
  def subsequent_visibility
  end
end

SelfReferentialPrivateMethodInvocationTest.new.subsequent_visibility # error: Non-private call to private method `SelfReferentialPrivateMethodInvocationTest#subsequent_visibility`

