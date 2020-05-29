# typed: strict

class MyModel; end

class MyOtherModel
  include T::Props
  prop :my_model, String, foreign: MyModel
end
