# typed: true

module C
  class Bar; end

  class BarChild < Bar;
    include ::X
    some_method do
      include ::TodoX # Currently missing from dep analysis
    end
  end

  module TestExtend
    extend ::Y
    some_method do
      extend ::TodoY # Currently missing from dep analysis
    end
  end
end

