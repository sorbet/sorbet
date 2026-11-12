# typed: true
module Root
end
module Root::User
  extend T::Helpers
  mixes_in_class_methods Root
end
