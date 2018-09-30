# typed: true
module Benchmark
  BENCHMARK_VERSION = T.let(T.unsafe(nil), String)
  CAPTION = T.let(T.unsafe(nil), String)
  FORMAT = T.let(T.unsafe(nil), String)

  sig do
    params(
        caption: String,
        label_width: Integer,
        format: String,
        labels: String,
    )
    .returns(T::Array[Benchmark::Tms])
  end
  def self.benchmark(caption, label_width=T.unsafe(nil), format=T.unsafe(nil), *labels); end

  sig do
    params(
        label_width: Integer,
        labels: String,
        blk: T.proc.params(arg0: Process).returns(NilClass),
    )
    .returns(T::Array[Benchmark::Tms])
  end
  def self.bm(label_width=T.unsafe(nil), *labels, &blk); end

  sig do
    params(
        width: Integer,
        blk: T.proc.params(arg0: Process).returns(NilClass),
    )
    .returns(T::Array[Benchmark::Tms])
  end
  def self.bmbm(width=T.unsafe(nil), &blk); end

  sig do
    params(
        label: String,
    )
    .returns(Benchmark::Tms)
  end
  def self.measure(label=T.unsafe(nil)); end

  sig do
    params(
        blk: BasicObject,
    )
    .returns(Integer)
  end
  def self.realtime(&blk); end
end

class Benchmark::Tms < Object
  CAPTION = T.let(T.unsafe(nil), String)
  FORMAT = T.let(T.unsafe(nil), String)
end
