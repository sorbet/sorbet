# typed: true

module S1
  interface!
end

class S2
  abstract!
end

class S3
  final!
end

class S4
  sealed!
end

module S5
  interface!
  final!
end

class S6
  abstract!
  sealed!
end

module S7
  mixes_in_class_methods(S1)
end
