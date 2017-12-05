# @typed
module Test
  class Baz
  def foo
  end
  end
  class Diff
    private def parse(diff)
        b_name = nil
        each do # error: each does not exist
            b_name = Baz.new if b_name && b_name.foo # error: MULTI , changing pinned + false errors
        end
    end
  end
end
