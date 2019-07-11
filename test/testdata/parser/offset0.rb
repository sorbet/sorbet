# typed: true
module Model # error-with-dupes: module definition in method body
  def a-b; end # error-with-dupes: unexpected token
end
