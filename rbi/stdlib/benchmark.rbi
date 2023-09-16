# typed: __STDLIB_INTERNAL

# The [`Benchmark`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html) module
# provides methods to measure and report the time used to execute Ruby code.
#
# *   Measure the time to construct the string given by the expression
#     `"a"*1_000_000_000`:
#
# ```ruby
# require 'benchmark'
#
# puts Benchmark.measure { "a"*1_000_000_000 }
# ```
#
#     On my machine (OSX 10.8.3 on i5 1.7 GHz) this generates:
#
# ```
# 0.350000   0.400000   0.750000 (  0.835234)
# ```
#
#     This report shows the user CPU time, system CPU time, the sum of the user
#     and system CPU times, and the elapsed real time. The unit of time is
#     seconds.
#
# *   Do some experiments sequentially using the
#     [`bm`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-bm)
#     method:
#
# ```ruby
# require 'benchmark'
#
# n = 5000000
# Benchmark.bm do |x|
#   x.report { for i in 1..n; a = "1"; end }
#   x.report { n.times do   ; a = "1"; end }
#   x.report { 1.upto(n) do ; a = "1"; end }
# end
# ```
#
#     The result:
#
# ```
#     user     system      total        real
# 1.010000   0.000000   1.010000 (  1.014479)
# 1.000000   0.000000   1.000000 (  0.998261)
# 0.980000   0.000000   0.980000 (  0.981335)
# ```
#
# *   Continuing the previous example, put a label in each report:
#
# ```ruby
# require 'benchmark'
#
# n = 5000000
# Benchmark.bm(7) do |x|
#   x.report("for:")   { for i in 1..n; a = "1"; end }
#   x.report("times:") { n.times do   ; a = "1"; end }
#   x.report("upto:")  { 1.upto(n) do ; a = "1"; end }
# end
# ```
#
#
# The result:
#
# ```
#               user     system      total        real
# for:      1.010000   0.000000   1.010000 (  1.015688)
# times:    1.000000   0.000000   1.000000 (  1.003611)
# upto:     1.030000   0.000000   1.030000 (  1.028098)
# ```
#
# *   The times for some benchmarks depend on the order in which items are run.
#     These differences are due to the cost of memory allocation and garbage
#     collection. To avoid these discrepancies, the
#     [`bmbm`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-bmbm)
#     method is provided. For example, to compare ways to sort an array of
#     floats:
#
# ```ruby
# require 'benchmark'
#
# array = (1..1000000).map { rand }
#
# Benchmark.bmbm do |x|
#   x.report("sort!") { array.dup.sort! }
#   x.report("sort")  { array.dup.sort  }
# end
# ```
#
#     The result:
#
# ```
# Rehearsal -----------------------------------------
# sort!   1.490000   0.010000   1.500000 (  1.490520)
# sort    1.460000   0.000000   1.460000 (  1.463025)
# -------------------------------- total: 2.960000sec
#
#             user     system      total        real
# sort!   1.460000   0.000000   1.460000 (  1.460465)
# sort    1.450000   0.010000   1.460000 (  1.448327)
# ```
#
# *   Report statistics of sequential experiments with unique labels, using the
#     [`benchmark`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-benchmark)
#     method:
#
# ```ruby
# require 'benchmark'
# include Benchmark         # we need the CAPTION and FORMAT constants
#
# n = 5000000
# Benchmark.benchmark(CAPTION, 7, FORMAT, ">total:", ">avg:") do |x|
#   tf = x.report("for:")   { for i in 1..n; a = "1"; end }
#   tt = x.report("times:") { n.times do   ; a = "1"; end }
#   tu = x.report("upto:")  { 1.upto(n) do ; a = "1"; end }
#   [tf+tt+tu, (tf+tt+tu)/3]
# end
# ```
#
#     The result:
#
# ```
#              user     system      total        real
# for:      0.950000   0.000000   0.950000 (  0.952039)
# times:    0.980000   0.000000   0.980000 (  0.984938)
# upto:     0.950000   0.000000   0.950000 (  0.946787)
# >total:   2.880000   0.000000   2.880000 (  2.883764)
# >avg:     0.960000   0.000000   0.960000 (  0.961255)
# ```
module Benchmark
  BENCHMARK_VERSION = T.let(T.unsafe(nil), String)
  # The default caption string (heading above the output times).
  CAPTION = T.let(T.unsafe(nil), String)
  # The default format string used to display times. See also
  # [`Benchmark::Tms#format`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#method-i-format).
  FORMAT = T.let(T.unsafe(nil), String)

  # Invokes the block with a Benchmark::Report object, which may be used to
  # collect and report on the results of individual benchmark tests. Reserves
  # `label_width` leading spaces for labels on each line. Prints `caption` at
  # the top of the report, and uses `format` to format each line. Returns an
  # array of
  # [`Benchmark::Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html)
  # objects.
  #
  # If the block returns an array of
  # [`Benchmark::Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html)
  # objects, these will be used to format additional lines of output. If
  # `labels` parameter are given, these are used to label these extra lines.
  #
  # *Note*: Other methods provide a simpler interface to this one, and are
  # suitable for nearly all benchmarking requirements. See the examples in
  # [`Benchmark`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html), and the
  # [`bm`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-bm) and
  # [`bmbm`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-bmbm)
  # methods.
  #
  # Example:
  #
  # ```ruby
  # require 'benchmark'
  # include Benchmark          # we need the CAPTION and FORMAT constants
  #
  # n = 5000000
  # Benchmark.benchmark(CAPTION, 7, FORMAT, ">total:", ">avg:") do |x|
  #   tf = x.report("for:")   { for i in 1..n; a = "1"; end }
  #   tt = x.report("times:") { n.times do   ; a = "1"; end }
  #   tu = x.report("upto:")  { 1.upto(n) do ; a = "1"; end }
  #   [tf+tt+tu, (tf+tt+tu)/3]
  # end
  # ```
  #
  # Generates:
  #
  # ```
  #               user     system      total        real
  # for:      0.970000   0.000000   0.970000 (  0.970493)
  # times:    0.990000   0.000000   0.990000 (  0.989542)
  # upto:     0.970000   0.000000   0.970000 (  0.972854)
  # >total:   2.930000   0.000000   2.930000 (  2.932889)
  # >avg:     0.976667   0.000000   0.976667 (  0.977630)
  # ```
  sig do
    params(
        caption: String,
        label_width: T.nilable(Integer),
        format: T.nilable(String),
        labels: String,
        blk: T.proc.params(report: Report).void,
    )
    .returns(T::Array[Benchmark::Tms])
  end
  def self.benchmark(caption, label_width=T.unsafe(nil), format=T.unsafe(nil), *labels, &blk); end

  # A simple interface to the
  # [`benchmark`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-benchmark)
  # method,
  # [`bm`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-bm)
  # generates sequential reports with labels. `label_width` and `labels`
  # parameters have the same meaning as for
  # [`benchmark`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-benchmark).
  #
  # ```ruby
  # require 'benchmark'
  #
  # n = 5000000
  # Benchmark.bm(7) do |x|
  #   x.report("for:")   { for i in 1..n; a = "1"; end }
  #   x.report("times:") { n.times do   ; a = "1"; end }
  #   x.report("upto:")  { 1.upto(n) do ; a = "1"; end }
  # end
  # ```
  #
  # Generates:
  #
  # ```
  #               user     system      total        real
  # for:      0.960000   0.000000   0.960000 (  0.957966)
  # times:    0.960000   0.000000   0.960000 (  0.960423)
  # upto:     0.950000   0.000000   0.950000 (  0.954864)
  # ```
  sig do
    params(
        label_width: Integer,
        labels: String,
        blk: T.proc.params(report: Report).void,
    )
    .returns(T::Array[Benchmark::Tms])
  end
  def self.bm(label_width=T.unsafe(nil), *labels, &blk); end

  # Sometimes benchmark results are skewed because code executed earlier
  # encounters different garbage collection overheads than that run later.
  # [`bmbm`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-bmbm)
  # attempts to minimize this effect by running the tests twice, the first time
  # as a rehearsal in order to get the runtime environment stable, the second
  # time for real.
  # [`GC.start`](https://docs.ruby-lang.org/en/2.7.0/GC.html#method-c-start) is
  # executed before the start of each of the real timings; the cost of this is
  # not included in the timings. In reality, though, there's only so much that
  # [`bmbm`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-bmbm)
  # can do, and the results are not guaranteed to be isolated from garbage
  # collection and other effects.
  #
  # Because
  # [`bmbm`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-bmbm)
  # takes two passes through the tests, it can calculate the required label
  # width.
  #
  # ```ruby
  # require 'benchmark'
  #
  # array = (1..1000000).map { rand }
  #
  # Benchmark.bmbm do |x|
  #   x.report("sort!") { array.dup.sort! }
  #   x.report("sort")  { array.dup.sort  }
  # end
  # ```
  #
  # Generates:
  #
  # ```
  # Rehearsal -----------------------------------------
  # sort!   1.440000   0.010000   1.450000 (  1.446833)
  # sort    1.440000   0.000000   1.440000 (  1.448257)
  # -------------------------------- total: 2.890000sec
  #
  #             user     system      total        real
  # sort!   1.460000   0.000000   1.460000 (  1.458065)
  # sort    1.450000   0.000000   1.450000 (  1.455963)
  # ```
  #
  # [`bmbm`](https://docs.ruby-lang.org/en/2.7.0/Benchmark.html#method-i-bmbm)
  # yields a Benchmark::Job object and returns an array of
  # [`Benchmark::Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html)
  # objects.
  sig do
    params(
        width: Integer,
        blk: T.proc.params(job: Job).void,
    )
    .returns(T::Array[Benchmark::Tms])
  end
  def self.bmbm(width=T.unsafe(nil), &blk); end

  # Returns the time used to execute the given block as a
  # [`Benchmark::Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html)
  # object. Takes `label` option.
  #
  # ```ruby
  # require 'benchmark'
  #
  # n = 1000000
  #
  # time = Benchmark.measure do
  #   n.times { a = "1" }
  # end
  # puts time
  # ```
  #
  # Generates:
  #
  # ```
  # 0.220000   0.000000   0.220000 (  0.227313)
  # ```
  sig do
    params(
        label: Object,
        blk: T.proc.void,
    )
    .returns(Benchmark::Tms)
  end
  def self.measure(label=T.unsafe(nil), &blk); end

  # Returns the elapsed real time used to execute the given block.
  sig do
    params(
        blk: T.proc.void,
    )
    .returns(Float)
  end
  def self.realtime(&blk); end
end

class Benchmark::Job
  # Registers the given label and block pair in the job list.
  sig do
    params(
        label: Object,
        blk: T.proc.void
    )
    .returns(T.self_type)
  end
  def item(label=T.unsafe(nil), &blk); end

  # An array of 2-element arrays, consisting of label and block pairs.
  sig {returns(T::Array[T.untyped])}
  def list; end

  # Registers the given label and block pair in the job list.
  sig do
    params(
        label: Object,
        blk: T.proc.void
    )
    .returns(T.self_type)
  end
  def report(label=T.unsafe(nil), &blk); end

  # Length of the widest label in the #list.
  sig {returns(Integer)}
  def width; end
end

class Benchmark::Report
  # Prints the `label` and measured time for the block,
  # formatted by `format`. See Tms#format for the
  # formatting rules.
  sig do
    params(
        label: Object,
        format: T.untyped,
        blk: T.proc.void
    )
    .returns(T.self_type)
  end
  def item(label=T.unsafe(nil), *format, &blk); end

  # An array of Benchmark::Tms objects representing each item.
  sig {returns(T::Array[Benchmark::Tms])}
  def list; end

  # Prints the `label` and measured time for the block,
  # formatted by `format`. See Tms#format for the
  # formatting rules.
  sig do
    params(
        label: Object,
        format: T.untyped,
        blk: T.proc.void
    )
    .returns(T.self_type)
  end
  def report(label=T.unsafe(nil), *format, &blk); end
end

# A data object, representing the times associated with a benchmark measurement.
class Benchmark::Tms < Object
  # Default caption, see also Benchmark::CAPTION
  CAPTION = T.let(T.unsafe(nil), String)

  # Default format string, see also Benchmark::FORMAT
  FORMAT = T.let(T.unsafe(nil), String)

  # Returns an initialized
  # [`Tms`](https://docs.ruby-lang.org/en/2.6.0/Benchmark/Tms.html) object which
  # has `utime` as the user CPU time, `stime` as the system CPU time, `cutime`
  # as the children’s user CPU time, `cstime` as the children’s system CPU time,
  # `real` as the elapsed real time and `label` as the label.
  sig do
    params(
      utime: Numeric,
      stime: Numeric,
      cutime: Numeric,
      cstime: Numeric,
      real: Numeric,
      label: T.nilable(String)
    ).void
  end
  def initialize(utime = 0.0, stime = 0.0, cutime = 0.0, cstime = 0.0, real = 0.0, label = nil); end

  # Returns a new
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object
  # obtained by memberwise multiplication of the individual times for this
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object by
  # `x`.
  sig { params(x: T.any(Numeric, Benchmark::Tms)).returns(Benchmark::Tms) }
  def *(x); end

  # Returns a new
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object
  # obtained by memberwise summation of the individual times for this
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object with
  # those of the `other`
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object. This
  # method and #/() are useful for taking statistics.
  sig { params(other: T.any(Numeric, Benchmark::Tms)).returns(Benchmark::Tms) }
  def +(other); end

  # Returns a new
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object
  # obtained by memberwise subtraction of the individual times for the `other`
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object from
  # those of this
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object.
  sig { params(other: T.any(Numeric, Benchmark::Tms)).returns(Benchmark::Tms) }
  def -(other); end

  # Returns a new
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object
  # obtained by memberwise division of the individual times for this
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object by
  # `x`. This method and
  # [`+()`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#method-i-2B)
  # are useful for taking statistics.
  sig { params(x: T.any(Numeric, Benchmark::Tms)).returns(Benchmark::Tms) }
  def /(x); end

  # Returns a new
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object whose
  # times are the sum of the times for this
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object, plus
  # the time required to execute the code block (`blk`).
  sig { params(blk: T.proc.void).returns(Benchmark::Tms) }
  def add(&blk); end

  # An in-place version of
  # [`add`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#method-i-add).
  # Changes the times of this
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object by
  # making it the sum of the times for this
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object, plus
  # the time required to execute the code block (`blk`).
  sig { params(blk: T.proc.void).void }
  def add!(&blk); end

  # System CPU time of children
  sig { returns(Float) }
  def cstime; end

  # User CPU time of children
  sig { returns(Float) }
  def cutime; end

  # Returns the contents of this
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object as a
  # formatted string, according to a `format` string like that passed to
  # [`Kernel.format`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-format).
  # In addition,
  # [`format`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#method-i-format)
  # accepts the following extensions:
  #
  # `%u`
  # :   Replaced by the user CPU time, as reported by
  #     [`Tms#utime`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#attribute-i-utime).
  # `%y`
  # :   Replaced by the system CPU time, as reported by
  #     [`stime`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#attribute-i-stime)
  #     (Mnemonic: y of "s\*y\*stem")
  # `%U`
  # :   Replaced by the children's user CPU time, as reported by
  #     [`Tms#cutime`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#attribute-i-cutime)
  # `%Y`
  # :   Replaced by the children's system CPU time, as reported by
  #     [`Tms#cstime`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#attribute-i-cstime)
  # `%t`
  # :   Replaced by the total CPU time, as reported by
  #     [`Tms#total`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#attribute-i-total)
  # `%r`
  # :   Replaced by the elapsed real time, as reported by
  #     [`Tms#real`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#attribute-i-real)
  # `%n`
  # :   Replaced by the label string, as reported by
  #     [`Tms#label`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#attribute-i-label)
  #     (Mnemonic: n of "\*n\*ame")
  #
  #
  # If `format` is not given,
  # [`FORMAT`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html#FORMAT) is
  # used as default value, detailing the user, system and real elapsed time.
  sig { params(format: String, args: T.untyped).returns(String) }
  def format(format = _, *args); end

  # Label
  sig { returns(String) }
  def label; end

  # Elapsed real time
  sig { returns(Float) }
  def real; end

  # System CPU time
  sig { returns(Float) }
  def stime; end

  # Total time, that is `utime` + `stime` + `cutime` + `cstime`
  sig { returns(Float) }
  def total; end

  # User CPU time
  sig { returns(Float) }
  def utime; end

  protected

  # Returns a new
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object
  # obtained by memberwise operation `op` of the individual times for this
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object with
  # those of the other
  # [`Tms`](https://docs.ruby-lang.org/en/2.7.0/Benchmark/Tms.html) object
  # (`x`).
  #
  # `op` can be a mathematical operation such as `+`, `-`, `*`, `/`
  sig { params(op: T.any(String, Symbol), x: Numeric).returns(Benchmark::Tms) }
  def memberwise(op, x); end
end
