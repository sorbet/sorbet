# typed: true
class Test
  private
  def subsequent_visibility; end
end

Test.new.subsequent_visibility
