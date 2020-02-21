# typed: strict

class MyModel; end

class MyOtherModel
  prop :my_model, String, foreign: MyModel
end
