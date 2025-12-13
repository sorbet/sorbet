# frozen_string_literal: true
# typed: strict

require 'benchmark'
require 'fileutils'
require 'optparse'
require_relative '../../gems/sorbet-runtime/lib/sorbet-runtime'

class Module
  include T::Sig
end

class BenchmarkCommand < T::Struct
  prop :argv, T::Array[String]
  prop :dir, String
  prop :env, T::Hash[String, T.untyped], factory: -> {{}}
end

class BenchmarkResult < T::Struct
  const :benchmark, String
  const :shortname, String
  const :interpreted_time, Float
  const :compiled_time, Float
end

class SuiteResult < T::Struct
  const :baseline, T.nilable(BenchmarkResult)
  const :benchmarks, T::Array[BenchmarkResult]
end

module SorbetBenchmark

  sig {params(command: T::Array[String], env: T::Hash[String, T.untyped],
              chdir: T.nilable(String), out: T.nilable(String), err: T.nilable(String)).void}
  def self.check_call(command, env: {}, chdir: nil, out: nil, err: nil)
    opts = {}
    if chdir
      opts[:chdir] = chdir
    end
    if out
      opts[:out] = out
    end
    if err
      opts[:err] = err
    end

    pid = Process.spawn(env, *command, opts)
    Process.wait(pid)
    status = $?
    if status.to_i != 0
      raise "#{command.join(" ")} exited with #{status.to_i}"
    end
  end

  sig {params(benchmark: String).void}
  def self.compile(benchmark)
    FileUtils.rm_rf(Dir.glob('tmp/bench/*'))

    target = 'tmp/bench/target.rb'
    FileUtils.cp(benchmark, target)
    check_call(['bash', 'test/run_sorbet.sh', target], env: {'llvmir' => '.', 'compiled_out_dir' => '.'}, out: "/dev/null", err: "/dev/null")
  end

  sig {params(ruby: String, sorbet: String).returns(T::Array[String])}
  def self.basic_ruby(ruby, sorbet)
    return [ruby, "--disable=gems", "--disable=did_you_mean",
            "-r", "rubygems",
            "-r", "#{sorbet}/gems/sorbet-runtime/lib/sorbet-runtime.rb"]
  end

  sig {params(ruby: String, sorbet: String).returns(BenchmarkCommand)}
  def self.startup_command(ruby, sorbet)
    BenchmarkCommand.new(
      argv: [*basic_ruby(ruby, sorbet), "-e", "1"],
      dir: 'tmp/bench',
    )
  end
  
  sig {params(ruby: String, sorbet: String).returns(BenchmarkCommand)}
  def self.interpreted_command(ruby, sorbet)
    BenchmarkCommand.new(
      argv: [*basic_ruby(ruby, sorbet), "./target.rb"],
      dir: 'tmp/bench',
    )
  end
  
  sig {params(topdir: String, ruby: String, sorbet: String).returns(BenchmarkCommand)}
  def self.compiled_command(topdir, ruby, sorbet)
    BenchmarkCommand.new(
      argv: [*basic_ruby(ruby, sorbet),
             "-r", "#{topdir}/test/patch_require.rb",
             "-e", "$__sorbet_ruby_realpath='target.rb'; require './target.rb.so'"],
      dir: 'tmp/bench',
      env: {'llvmir' => '.'},
    )
  end

  sig {params(command: BenchmarkCommand).returns(Float)}
  def self.measure(command)
    Benchmark.realtime do
      check_call(command.argv, env: command.env, chdir: command.dir,
                 out: "/dev/null", err: "/dev/null")
    end
  end

  sig {params(command: BenchmarkCommand, n_runs: Integer).returns(Float)}
  def self.average_runtime(command, n_runs: 10)
    n_runs.times.map do |_|
      measure(command)
    end.reduce(0.0, :+) / n_runs
  end

  sig {params(benchmark: String, interpreted_time: Float, compiled_time: Float, delimiter: String).void}
  def self.report(benchmark, interpreted_time, compiled_time, delimiter)
      puts "%<benchmark>s#{delimiter}%<interpreted>.3f#{delimiter}%<compiled>.3f".%(
             benchmark: benchmark,
             interpreted: interpreted_time,
             compiled: compiled_time,
           )
  end

  sig {params(rootdir: String, benchmark: String, ruby: String, sorbet: String).returns(BenchmarkResult)}
  def self.compare_interpreted_vs_compiled(rootdir, benchmark, ruby, sorbet)
    compile(benchmark)
    interpreted = average_runtime(interpreted_command(ruby, sorbet))
    compiled = average_runtime(compiled_command(rootdir, ruby, sorbet))
    BenchmarkResult.new(
      benchmark: benchmark,
      shortname: benchmark.sub(%r{^test/testdata/ruby_benchmark/}, ''),
      interpreted_time: interpreted,
      compiled_time: compiled,
    )
  end

  sig {params(topdir: String, benchmarks: T::Array[String], baseline: T.nilable(String), verbose: T::Boolean).returns(SuiteResult)}
  def self.gather_results(topdir, benchmarks, baseline, verbose)
    Dir.chdir(topdir)

    sorbet_ruby_target = "@sorbet_ruby_2_7_for_compiler//:ruby"

    check_call(["./bazel", "build", "//compiler:sorbet", sorbet_ruby_target, "-c", "opt"])
    check_call(["./bazel", "run", sorbet_ruby_target, "-c", "opt", "--", "--version"])

    FileUtils.mkdir_p('tmp/bench')

    pwd = Dir.pwd

    ruby = "#{pwd}/bazel-bin/external/sorbet_ruby_2_7_for_compiler/ruby"
    sorbet = "#{pwd}"

    startup_time = average_runtime(startup_command(ruby, sorbet))
    puts "ruby vm startup time: %.3f" % startup_time

    baseline_result = nil

    if baseline
      baseline_result = compare_interpreted_vs_compiled(pwd, baseline, ruby, sorbet)
    end

    benchmark_results = benchmarks.map do |b|
      compare_interpreted_vs_compiled(pwd, b, ruby, sorbet)
    end

    SuiteResult.new(
      baseline: baseline_result,
      benchmarks: benchmark_results,
    )
  end

  sig {params(topdir: String, benchmarks: T::Array[String], delimiter: String, baseline: T.nilable(String), verbose: T::Boolean).void}
  def self.run(topdir:, benchmarks:, delimiter:, baseline:, verbose:)
    suite = gather_results(topdir, benchmarks, baseline, verbose)

    baseline_result = suite.baseline
    baseline_interpreted = 0.0
    baseline_compiled = 0.0

    if baseline_result
      baseline_interpreted = baseline_result.interpreted_time
      baseline_compiled = baseline_result.compiled_time
      report(baseline_result.shortname, baseline_interpreted, baseline_compiled, delimiter)
    end

    suite.benchmarks.each do |result|
      report(result.shortname, result.interpreted_time, result.compiled_time, delimiter)

      if baseline_result
        report("#{result.shortname} - baseline",
               result.interpreted_time - baseline_interpreted,
               result.compiled_time - baseline_compiled, delimiter)
      end
    end
  end
end

if __FILE__ == $0
  topdir = File.dirname($0) + '/../..'

  baseline = nil
  delimiter = "\t"
  benchmarks = []
  verbose = false

  OptionParser.new do |opts|

    opts.banner = "Usage: run-benchmarks.rb [options] -f [benchmark1] -f [benchmark2] ..."

    opts.on '-b PATH', '--baseline=PATH', 'Path to a baseline Ruby benchmark to subtract out' do |path|
      baseline = path
    end

    opts.on '-d STRING', '--delimiter=STRING', 'Delimiter character' do |d|
      delimiter = d
    end

    opts.on '-f PATH', '--file=PATH', 'Path to a Ruby benchmark' do |path|
      benchmarks.push(path)
    end

    opts.on '-v' do
      verbose = true
    end

  end.parse!

  if benchmarks.empty?
    if baseline
      raise "-b is not supported with no provided benchmarks!"
    end

    benchmarks = Dir.glob('test/testdata/ruby_benchmark/**/*.rb').filter do |filename|
      filename.include?('disabled') || filename.include?('too_slow')
    end.sort
  end

  SorbetBenchmark.run(topdir: topdir, benchmarks: benchmarks, delimiter: delimiter, baseline: baseline, verbose: verbose)
end
