# typed: true

def takes_untyped_block(&block)
end

def example1(&xyz)
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
