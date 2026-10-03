# typed: true

class App::Test::Value < App::Value
  BAD = T.let("wrong", Integer)
end
