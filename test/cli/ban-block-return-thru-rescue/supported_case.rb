# frozen_string_literal: true
# typed: true
# compiled: true

# This is the return-from-block case that we should soon be able to support
# soon. The test here is just to make sure that the "return statements crossing
# both block and exception frames are not yet implemented" error is NOT being
# thrown in those cases. The more generic existing error message ("return
# through multiple stacks not implemented") is expected here for now; soon,
# no error message will be expected in this case, and this test will go away.
def g
  yield
end

def f
  if T.unsafe(false)
    i = 0
    while i < 100
      g { return 99 }
      i += 1
    end
  end
end
