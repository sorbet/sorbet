# typed: true

class G
  extend T::Generic

  MYPARAM = type_member
end

class Correct1 < G
  extend T::Generic

  MYPARAM = type_member(fixed: String)
end

class Correct2 < Correct1
end

class Correct3 < Correct2
end

class Correct4 < Correct3
end
