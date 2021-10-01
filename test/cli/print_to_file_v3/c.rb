# typed: true

module C
  class Bar; end

  class BarChild < Bar;
    include ::X
    some_method do
      include ::InMethodX
    end
  end

  module TestExtend
    extend ::Y
    some_method do
      extend ::InMethodY
    end

    app.include ::ViaMethod
  end
end

