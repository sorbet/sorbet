foobar do
  # TODO(nelhage): We should figure out where these should *actually*
  # live, but for now just make sure we don't crash.
  def a_method # error: dynamic method definition
  end

  def self.a_static_method # error: dynamic method definition
  end
end
