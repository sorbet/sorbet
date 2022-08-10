# typed: true
# spacer for exclude-from-file-update

# Pretend this is a source file that implements a method with the type from
# the RBI. This files does not have any calls to `foo`

class A
  def foo
    if T.unsafe(nil)
      return 0
    else
      return '' # error: Expected `Integer` but found `String("")` for method result type
    end
  end
end
