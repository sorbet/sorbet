# typed: true

def takes_untyped_block(&block)
end

def example1(&xyz)
  # This would be an error if we naively said that `xyz` had type `T.nilable(Proc)`
  xyz.call
  # This can't be treated as unnecessary use of safe navigation
  xyz&.call

  T.reveal_type(xyz) # error: `T.untyped`
  if xyz
    xyz = proc { "default" }
    takes_untyped_block(&xyz)
  else
    T.reveal_type(xyz) # error: `NilClass`
    takes_untyped_block(&xyz)
  end
end

def example2(&xyz)
  if T.unsafe(nil)
    xyz = false
  end
  T.reveal_type(xyz) # error: `T.untyped`
  unless xyz
    T.reveal_type(xyz) # error: `T.nilable(FalseClass)`
  end
end
