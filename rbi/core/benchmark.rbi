# typed: true
module Benchmark
  BENCHMARK_VERSION = T.let(T.unsafe(nil), String)
  CAPTION = T.let(T.unsafe(nil), String)
  FORMAT = T.let(T.unsafe(nil), String)

  Sorbet.sig(
      caption: String,
      label_width: Integer,
      format: String,
      labels: String,
  )
  .returns(T::Array[Benchmark::Tms])
  def self.benchmark(caption, label_width=_, format=_, *labels); end

  Sorbet.sig(
      label_width: Integer,
      labels: String,
      blk: T.proc(arg0: Process).returns(NilClass),
  )
  .returns(T::Array[Benchmark::Tms])
  def self.bm(label_width=_, *labels, &blk); end

  Sorbet.sig(
      width: Integer,
      blk: T.proc(arg0: Process).returns(NilClass),
  )
  .returns(T::Array[Benchmark::Tms])
  def self.bmbm(width=_, &blk); end

  Sorbet.sig(
      label: String,
  )
  .returns(Benchmark::Tms)
  def self.measure(label=_); end

  Sorbet.sig(
      blk: BasicObject,
  )
  .returns(Integer)
  def self.realtime(&blk); end
end

class Benchmark::Tms < Object
  CAPTION = T.let(T.unsafe(nil), String)
  FORMAT = T.let(T.unsafe(nil), String)
end
