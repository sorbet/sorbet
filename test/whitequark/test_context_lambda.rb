# typed: strict

module A; ->(a = (return; 1)) {}; end
class B; ->(a = (return; 1)) {}; end
