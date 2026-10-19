# typed: true

module Consumer
  class C
    include Producer::M
    #       ^^^^^^^^ error: `Producer` is not imported
    #       ^^^^^^^^ error: `Producer` is not imported
    extend Producer::M
    #      ^^^^^^^^ error: `Producer` is not imported
    #      ^^^^^^^^ error: `Producer` is not imported
  end
end
