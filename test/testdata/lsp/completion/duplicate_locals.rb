# typed: true

def reassigned_local
  xyz = nil
  xyz = !!xyz
  p xy # error: does not exist
  #   ^ completion: xyz, xyz
end

def local_in_each_branch
  if T.unsafe(nil)
    xyz = true
  else
    xyz = false
  end
  p xy # error: does not exist
  #   ^ completion: xyz, xyz
end
