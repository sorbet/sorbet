# typed: strict

# This "forward declaration" of ExportsSelf should not have any impact on
# visibility, despite this declaration happening in a file owned by
# ExportsSelf::Nested.
module ExportsSelf; end

module ExportsSelf::Nested
  class InsideNested
    extend T::Sig

    sig { void }
    def initialize
    end
  end
end
