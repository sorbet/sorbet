class Foo
  def baz
  a = self.does_not_exist # error: does not exist
  a.bla # should NOT error
  end
end
