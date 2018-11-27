# typed: true

module Benchmark
  FORMAT = T.let(T.unsafe(nil), String)

  class Job
    def report(label = _, &blk)
    end

    def item(label = _, &blk)
    end

    def list()
    end

    def width()
    end
  end

  class Tms
    CAPTION = T.let(T.unsafe(nil), String)

    FORMAT = T.let(T.unsafe(nil), String)

    def add!(&blk)
    end

    def stime()
    end

    def cutime()
    end

    def cstime()
    end

    def total()
    end

    def format(format = _, *args)
    end

    def *(x)
    end

    def +(other)
    end

    def to_s()
    end

    def -(other)
    end

    def add(&blk)
    end

    def /(x)
    end

    def to_a()
    end

    def real()
    end

    def utime()
    end

    def label()
    end
  end

  class Report
    def report(label = _, *format, &blk)
    end

    def item(label = _, *format, &blk)
    end

    def list()
    end
  end

  BENCHMARK_VERSION = T.let(T.unsafe(nil), String)

  CAPTION = T.let(T.unsafe(nil), String)
end
