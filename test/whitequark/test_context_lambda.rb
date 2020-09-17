# typed: strict

module A; ->(a = (return; 1)) {}; end
class B; ->(a = (return; 1)) {}; end
->(a) { module M; _1; end } # Method _1 does not exist on T.class_of(M)
