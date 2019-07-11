# typed: __STDLIB_INTERNAL
class OptionParser
  sig { params(to: T.untyped, name: T.untyped).returns(T.untyped) }
  def compsys(to, name = File.basename($0)); end

  sig { params(args: T.untyped, block: T.untyped).returns(T.untyped) }
  def self.with(*args, &block); end

  sig { params(arg: T.untyped, default: T.untyped).returns(T.untyped) }
  def self.inc(arg, default = nil); end

  sig { params(args: T.untyped).returns(T.untyped) }
  def inc(*args); end

  sig { params(banner: T.untyped, width: T.untyped, indent: T.untyped).void }
  def initialize(banner = nil, width = 32, indent = ' ' * 4); end

  sig { returns(T.untyped) }
  def add_officious(); end

  sig { params(arg: T.untyped).returns(T.untyped) }
  def terminate(arg = nil); end

  sig { params(arg: T.untyped).returns(T.untyped) }
  def self.terminate(arg = nil); end

  sig { returns(T.untyped) }
  def self.top(); end

  sig { params(args: T.untyped, blk: T.untyped).returns(T.untyped) }
  def accept(*args, &blk); end

  sig { params(args: T.untyped, blk: T.untyped).returns(T.untyped) }
  def self.accept(*args, &blk); end

  sig { params(args: T.untyped, blk: T.untyped).returns(T.untyped) }
  def reject(*args, &blk); end

  sig { params(args: T.untyped, blk: T.untyped).returns(T.untyped) }
  def self.reject(*args, &blk); end

  sig { params(value: T.untyped).returns(T.untyped) }
  def banner=(value); end

  sig { params(value: T.untyped).returns(T.untyped) }
  def program_name=(value); end

  sig { returns(T.untyped) }
  def summary_width(); end

  sig { params(value: T.untyped).returns(T.untyped) }
  def summary_width=(value); end

  sig { returns(T.untyped) }
  def summary_indent(); end

  sig { params(value: T.untyped).returns(T.untyped) }
  def summary_indent=(value); end

  sig { returns(T.untyped) }
  def default_argv(); end

  sig { params(value: T.untyped).returns(T.untyped) }
  def default_argv=(value); end

  sig { returns(T.untyped) }
  def banner(); end

  sig { returns(T.untyped) }
  def program_name(); end

  sig { params(value: T.untyped).returns(T.untyped) }
  def version=(value); end

  sig { params(value: T.untyped).returns(T.untyped) }
  def release=(value); end

  sig { returns(T.untyped) }
  def version(); end

  sig { returns(T.untyped) }
  def release(); end

  sig { returns(T.untyped) }
  def ver(); end

  sig { params(mesg: T.untyped).returns(T.untyped) }
  def warn(mesg = $!); end

  sig { params(mesg: T.untyped).returns(T.untyped) }
  def abort(mesg = $!); end

  sig { returns(T.untyped) }
  def top(); end

  sig { returns(T.untyped) }
  def base(); end

  sig { returns(T.untyped) }
  def new(); end

  sig { returns(T.untyped) }
  def remove(); end

  sig do
    params(
      to: T.untyped,
      width: T.untyped,
      max: T.untyped,
      indent: T.untyped,
      blk: T.untyped
    ).returns(T.untyped)
  end
  def summarize(to = [], width = @summary_width, max = width - 1, indent = @summary_indent, &blk); end

  sig { returns(T.untyped) }
  def help(); end

  sig { returns(T.untyped) }
  def to_a(); end

  sig { params(obj: T.untyped, prv: T.untyped, msg: T.untyped).returns(T.untyped) }
  def notwice(obj, prv, msg); end

  sig { params(opts: T.untyped, block: T.untyped).returns(T.untyped) }
  def make_switch(opts, block = nil); end

  sig { params(opts: T.untyped, block: T.untyped).returns(T.untyped) }
  def define(*opts, &block); end

  sig { params(opts: T.untyped, block: T.untyped).returns(T.untyped) }
  def on(*opts, &block); end

  sig { params(opts: T.untyped, block: T.untyped).returns(T.untyped) }
  def define_head(*opts, &block); end

  sig { params(opts: T.untyped, block: T.untyped).returns(T.untyped) }
  def on_head(*opts, &block); end

  sig { params(opts: T.untyped, block: T.untyped).returns(T.untyped) }
  def define_tail(*opts, &block); end

  sig { params(opts: T.untyped, block: T.untyped).returns(T.untyped) }
  def on_tail(*opts, &block); end

  sig { params(string: T.untyped).returns(T.untyped) }
  def separator(string); end

  sig { params(argv: T.untyped, into: T.untyped, nonopt: T.untyped).returns(T.untyped) }
  def order(*argv, into: nil, &nonopt); end

  sig { params(argv: T.untyped, into: T.untyped, nonopt: T.untyped).returns(T.untyped) }
  def order!(argv = default_argv, into: nil, &nonopt); end

  sig { params(argv: T.untyped, setter: T.untyped, nonopt: T.untyped).returns(T.untyped) }
  def parse_in_order(argv = default_argv, setter = nil, &nonopt); end

  sig { params(argv: T.untyped, into: T.untyped).returns(T.untyped) }
  def permute(*argv, into: nil); end

  sig { params(argv: T.untyped, into: T.untyped).returns(T.untyped) }
  def permute!(argv = default_argv, into: nil); end

  sig { params(argv: T.untyped, into: T.untyped).returns(T.untyped) }
  def parse(*argv, into: nil); end

  sig { params(argv: T.untyped, into: T.untyped).returns(T.untyped) }
  def parse!(argv = default_argv, into: nil); end

  sig { params(args: T.untyped).returns(T.untyped) }
  def getopts(*args); end

  sig { params(args: T.untyped).returns(T.untyped) }
  def self.getopts(*args); end

  sig { params(id: T.untyped, args: T.untyped, block: T.untyped).returns(T.untyped) }
  def visit(id, *args, &block); end

  sig { params(id: T.untyped, key: T.untyped).returns(T.untyped) }
  def search(id, key); end

  sig do
    params(
      typ: T.untyped,
      opt: T.untyped,
      icase: T.untyped,
      pat: T.untyped
    ).returns(T.untyped)
  end
  def complete(typ, opt, icase = false, *pat); end

  sig { params(word: T.untyped).returns(T.untyped) }
  def candidate(word); end

  sig { params(filename: T.untyped).returns(T.untyped) }
  def load(filename = nil); end

  sig { params(env: T.untyped).returns(T.untyped) }
  def environment(env = File.basename($0, '.*')); end

  module Completion
    sig { params(key: T.untyped, icase: T.untyped).returns(T.untyped) }
    def self.regexp(key, icase); end

    sig do
      params(
        key: T.untyped,
        icase: T.untyped,
        pat: T.untyped,
        block: T.untyped
      ).returns(T.untyped)
    end
    def self.candidate(key, icase = false, pat = nil, &block); end

    sig { params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped) }
    def candidate(key, icase = false, pat = nil); end

    sig { params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped) }
    def complete(key, icase = false, pat = nil); end

    sig { params(opt: T.untyped, val: T.untyped).returns(T.untyped) }
    def convert(opt = nil, val = nil); end
  end

  class OptionMap < Hash
    include OptionParser::Completion

    sig { params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped) }
    def candidate(key, icase = false, pat = nil); end

    sig { params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped) }
    def complete(key, icase = false, pat = nil); end

    sig { params(opt: T.untyped, val: T.untyped).returns(T.untyped) }
    def convert(opt = nil, val = nil); end
  end

  class Switch
    sig { returns(T.untyped) }
    def pattern(); end

    sig { returns(T.untyped) }
    def conv(); end

    sig { returns(T.untyped) }
    def short(); end

    sig { returns(T.untyped) }
    def long(); end

    sig { returns(T.untyped) }
    def arg(); end

    sig { returns(T.untyped) }
    def desc(); end

    sig { returns(T.untyped) }
    def block(); end

    sig { params(arg: T.untyped).returns(T.untyped) }
    def self.guess(arg); end

    sig { params(arg: T.untyped, t: T.untyped).returns(T.untyped) }
    def self.incompatible_argument_styles(arg, t); end

    sig { returns(T.untyped) }
    def self.pattern(); end

    sig do
      params(
        pattern: T.untyped,
        conv: T.untyped,
        short: T.untyped,
        long: T.untyped,
        arg: T.untyped,
        desc: T.untyped,
        block: T.untyped,
        _block: T.untyped
      ).void
    end
    def initialize(pattern = nil, conv = nil, short = nil, long = nil, arg = nil, desc = ([] if short or long), block = nil, &_block); end

    sig { params(arg: T.untyped).returns(T.untyped) }
    def parse_arg(arg); end

    sig { params(arg: T.untyped, val: T.untyped).returns(T.untyped) }
    def conv_arg(arg, val = []); end

    sig do
      params(
        sdone: T.untyped,
        ldone: T.untyped,
        width: T.untyped,
        max: T.untyped,
        indent: T.untyped
      ).returns(T.untyped)
    end
    def summarize(sdone = [], ldone = [], width = 1, max = width - 1, indent = ""); end

    sig { params(to: T.untyped).returns(T.untyped) }
    def add_banner(to); end

    sig { params(str: T.untyped).returns(T::Boolean) }
    def match_nonswitch?(str); end

    sig { returns(T.untyped) }
    def switch_name(); end

    sig { params(sdone: T.untyped, ldone: T.untyped).returns(T.untyped) }
    def compsys(sdone, ldone); end

    class NoArgument < OptionParser::Switch
      sig { params(arg: T.untyped, argv: T.untyped).returns(T.untyped) }
      def parse(arg, argv); end

      sig { returns(T.untyped) }
      def self.incompatible_argument_styles(); end

      sig { returns(T.untyped) }
      def self.pattern(); end

      sig { returns(T.untyped) }
      def pattern(); end

      sig { returns(T.untyped) }
      def conv(); end

      sig { returns(T.untyped) }
      def short(); end

      sig { returns(T.untyped) }
      def long(); end

      sig { returns(T.untyped) }
      def arg(); end

      sig { returns(T.untyped) }
      def desc(); end

      sig { returns(T.untyped) }
      def block(); end

      sig { params(arg: T.untyped).returns(T.untyped) }
      def self.guess(arg); end

      sig do
        params(
          pattern: T.untyped,
          conv: T.untyped,
          short: T.untyped,
          long: T.untyped,
          arg: T.untyped,
          desc: T.untyped,
          block: T.untyped,
          _block: T.untyped
        ).void
      end
      def initialize(pattern = nil, conv = nil, short = nil, long = nil, arg = nil, desc = ([] if short or long), block = nil, &_block); end

      sig { params(arg: T.untyped).returns(T.untyped) }
      def parse_arg(arg); end

      sig { params(arg: T.untyped, val: T.untyped).returns(T.untyped) }
      def conv_arg(arg, val = []); end

      sig do
        params(
          sdone: T.untyped,
          ldone: T.untyped,
          width: T.untyped,
          max: T.untyped,
          indent: T.untyped
        ).returns(T.untyped)
      end
      def summarize(sdone = [], ldone = [], width = 1, max = width - 1, indent = ""); end

      sig { params(to: T.untyped).returns(T.untyped) }
      def add_banner(to); end

      sig { params(str: T.untyped).returns(T::Boolean) }
      def match_nonswitch?(str); end

      sig { returns(T.untyped) }
      def switch_name(); end

      sig { params(sdone: T.untyped, ldone: T.untyped).returns(T.untyped) }
      def compsys(sdone, ldone); end
    end

    class RequiredArgument < OptionParser::Switch
      sig { params(arg: T.untyped, argv: T.untyped).returns(T.untyped) }
      def parse(arg, argv); end

      sig { returns(T.untyped) }
      def pattern(); end

      sig { returns(T.untyped) }
      def conv(); end

      sig { returns(T.untyped) }
      def short(); end

      sig { returns(T.untyped) }
      def long(); end

      sig { returns(T.untyped) }
      def arg(); end

      sig { returns(T.untyped) }
      def desc(); end

      sig { returns(T.untyped) }
      def block(); end

      sig { params(arg: T.untyped).returns(T.untyped) }
      def self.guess(arg); end

      sig { params(arg: T.untyped, t: T.untyped).returns(T.untyped) }
      def self.incompatible_argument_styles(arg, t); end

      sig { returns(T.untyped) }
      def self.pattern(); end

      sig do
        params(
          pattern: T.untyped,
          conv: T.untyped,
          short: T.untyped,
          long: T.untyped,
          arg: T.untyped,
          desc: T.untyped,
          block: T.untyped,
          _block: T.untyped
        ).void
      end
      def initialize(pattern = nil, conv = nil, short = nil, long = nil, arg = nil, desc = ([] if short or long), block = nil, &_block); end

      sig { params(arg: T.untyped).returns(T.untyped) }
      def parse_arg(arg); end

      sig { params(arg: T.untyped, val: T.untyped).returns(T.untyped) }
      def conv_arg(arg, val = []); end

      sig do
        params(
          sdone: T.untyped,
          ldone: T.untyped,
          width: T.untyped,
          max: T.untyped,
          indent: T.untyped
        ).returns(T.untyped)
      end
      def summarize(sdone = [], ldone = [], width = 1, max = width - 1, indent = ""); end

      sig { params(to: T.untyped).returns(T.untyped) }
      def add_banner(to); end

      sig { params(str: T.untyped).returns(T::Boolean) }
      def match_nonswitch?(str); end

      sig { returns(T.untyped) }
      def switch_name(); end

      sig { params(sdone: T.untyped, ldone: T.untyped).returns(T.untyped) }
      def compsys(sdone, ldone); end
    end

    class OptionalArgument < OptionParser::Switch
      sig { params(arg: T.untyped, argv: T.untyped, error: T.untyped).returns(T.untyped) }
      def parse(arg, argv, &error); end

      sig { returns(T.untyped) }
      def pattern(); end

      sig { returns(T.untyped) }
      def conv(); end

      sig { returns(T.untyped) }
      def short(); end

      sig { returns(T.untyped) }
      def long(); end

      sig { returns(T.untyped) }
      def arg(); end

      sig { returns(T.untyped) }
      def desc(); end

      sig { returns(T.untyped) }
      def block(); end

      sig { params(arg: T.untyped).returns(T.untyped) }
      def self.guess(arg); end

      sig { params(arg: T.untyped, t: T.untyped).returns(T.untyped) }
      def self.incompatible_argument_styles(arg, t); end

      sig { returns(T.untyped) }
      def self.pattern(); end

      sig do
        params(
          pattern: T.untyped,
          conv: T.untyped,
          short: T.untyped,
          long: T.untyped,
          arg: T.untyped,
          desc: T.untyped,
          block: T.untyped,
          _block: T.untyped
        ).void
      end
      def initialize(pattern = nil, conv = nil, short = nil, long = nil, arg = nil, desc = ([] if short or long), block = nil, &_block); end

      sig { params(arg: T.untyped).returns(T.untyped) }
      def parse_arg(arg); end

      sig { params(arg: T.untyped, val: T.untyped).returns(T.untyped) }
      def conv_arg(arg, val = []); end

      sig do
        params(
          sdone: T.untyped,
          ldone: T.untyped,
          width: T.untyped,
          max: T.untyped,
          indent: T.untyped
        ).returns(T.untyped)
      end
      def summarize(sdone = [], ldone = [], width = 1, max = width - 1, indent = ""); end

      sig { params(to: T.untyped).returns(T.untyped) }
      def add_banner(to); end

      sig { params(str: T.untyped).returns(T::Boolean) }
      def match_nonswitch?(str); end

      sig { returns(T.untyped) }
      def switch_name(); end

      sig { params(sdone: T.untyped, ldone: T.untyped).returns(T.untyped) }
      def compsys(sdone, ldone); end
    end

    class PlacedArgument < OptionParser::Switch
      sig { params(arg: T.untyped, argv: T.untyped, error: T.untyped).returns(T.untyped) }
      def parse(arg, argv, &error); end

      sig { returns(T.untyped) }
      def pattern(); end

      sig { returns(T.untyped) }
      def conv(); end

      sig { returns(T.untyped) }
      def short(); end

      sig { returns(T.untyped) }
      def long(); end

      sig { returns(T.untyped) }
      def arg(); end

      sig { returns(T.untyped) }
      def desc(); end

      sig { returns(T.untyped) }
      def block(); end

      sig { params(arg: T.untyped).returns(T.untyped) }
      def self.guess(arg); end

      sig { params(arg: T.untyped, t: T.untyped).returns(T.untyped) }
      def self.incompatible_argument_styles(arg, t); end

      sig { returns(T.untyped) }
      def self.pattern(); end

      sig do
        params(
          pattern: T.untyped,
          conv: T.untyped,
          short: T.untyped,
          long: T.untyped,
          arg: T.untyped,
          desc: T.untyped,
          block: T.untyped,
          _block: T.untyped
        ).void
      end
      def initialize(pattern = nil, conv = nil, short = nil, long = nil, arg = nil, desc = ([] if short or long), block = nil, &_block); end

      sig { params(arg: T.untyped).returns(T.untyped) }
      def parse_arg(arg); end

      sig { params(arg: T.untyped, val: T.untyped).returns(T.untyped) }
      def conv_arg(arg, val = []); end

      sig do
        params(
          sdone: T.untyped,
          ldone: T.untyped,
          width: T.untyped,
          max: T.untyped,
          indent: T.untyped
        ).returns(T.untyped)
      end
      def summarize(sdone = [], ldone = [], width = 1, max = width - 1, indent = ""); end

      sig { params(to: T.untyped).returns(T.untyped) }
      def add_banner(to); end

      sig { params(str: T.untyped).returns(T::Boolean) }
      def match_nonswitch?(str); end

      sig { returns(T.untyped) }
      def switch_name(); end

      sig { params(sdone: T.untyped, ldone: T.untyped).returns(T.untyped) }
      def compsys(sdone, ldone); end
    end
  end

  class List
    sig { returns(T.untyped) }
    def atype(); end

    sig { returns(T.untyped) }
    def short(); end

    sig { returns(T.untyped) }
    def long(); end

    sig { returns(T.untyped) }
    def list(); end

    sig { void }
    def initialize(); end

    sig { params(t: T.untyped, pat: T.untyped, block: T.untyped).returns(T.untyped) }
    def accept(t, pat = /.*/m, &block); end

    sig { params(t: T.untyped).returns(T.untyped) }
    def reject(t); end

    sig do
      params(
        sw: T.untyped,
        sopts: T.untyped,
        lopts: T.untyped,
        nsw: T.untyped,
        nlopts: T.untyped
      ).returns(T.untyped)
    end
    def update(sw, sopts, lopts, nsw = nil, nlopts = nil); end

    sig { params(args: T.untyped).returns(T.untyped) }
    def prepend(*args); end

    sig { params(args: T.untyped).returns(T.untyped) }
    def append(*args); end

    sig { params(id: T.untyped, key: T.untyped).returns(T.untyped) }
    def search(id, key); end

    sig do
      params(
        id: T.untyped,
        opt: T.untyped,
        icase: T.untyped,
        pat: T.untyped,
        block: T.untyped
      ).returns(T.untyped)
    end
    def complete(id, opt, icase = false, *pat, &block); end

    sig { params(block: T.untyped).returns(T.untyped) }
    def each_option(&block); end

    sig { params(args: T.untyped, block: T.untyped).returns(T.untyped) }
    def summarize(*args, &block); end

    sig { params(to: T.untyped).returns(T.untyped) }
    def add_banner(to); end

    sig { params(args: T.untyped, block: T.untyped).returns(T.untyped) }
    def compsys(*args, &block); end
  end

  class CompletingHash < Hash
    include OptionParser::Completion

    sig { params(key: T.untyped).returns(T.untyped) }
    def match(key); end

    sig { params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped) }
    def candidate(key, icase = false, pat = nil); end

    sig { params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped) }
    def complete(key, icase = false, pat = nil); end

    sig { params(opt: T.untyped, val: T.untyped).returns(T.untyped) }
    def convert(opt = nil, val = nil); end
  end

  class ParseError < RuntimeError
    sig { params(args: T.untyped).void }
    def initialize(*args); end

    sig { returns(T.untyped) }
    def args(); end

    sig { params(value: T.untyped).returns(T.untyped) }
    def reason=(value); end

    sig { params(argv: T.untyped).returns(T.untyped) }
    def recover(argv); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def self.filter_backtrace(array); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def set_backtrace(array); end

    sig { params(opt: T.untyped, eq: T.untyped).returns(T.untyped) }
    def set_option(opt, eq); end

    sig { returns(T.untyped) }
    def reason(); end

    sig { returns(T.untyped) }
    def inspect(); end

    sig { returns(T.untyped) }
    def message(); end
  end

  class AmbiguousOption < OptionParser::ParseError
    sig { params(args: T.untyped).void }
    def initialize(*args); end

    sig { returns(T.untyped) }
    def args(); end

    sig { params(value: T.untyped).returns(T.untyped) }
    def reason=(value); end

    sig { params(argv: T.untyped).returns(T.untyped) }
    def recover(argv); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def self.filter_backtrace(array); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def set_backtrace(array); end

    sig { params(opt: T.untyped, eq: T.untyped).returns(T.untyped) }
    def set_option(opt, eq); end

    sig { returns(T.untyped) }
    def reason(); end

    sig { returns(T.untyped) }
    def inspect(); end

    sig { returns(T.untyped) }
    def message(); end
  end

  class NeedlessArgument < OptionParser::ParseError
    sig { params(args: T.untyped).void }
    def initialize(*args); end

    sig { returns(T.untyped) }
    def args(); end

    sig { params(value: T.untyped).returns(T.untyped) }
    def reason=(value); end

    sig { params(argv: T.untyped).returns(T.untyped) }
    def recover(argv); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def self.filter_backtrace(array); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def set_backtrace(array); end

    sig { params(opt: T.untyped, eq: T.untyped).returns(T.untyped) }
    def set_option(opt, eq); end

    sig { returns(T.untyped) }
    def reason(); end

    sig { returns(T.untyped) }
    def inspect(); end

    sig { returns(T.untyped) }
    def message(); end
  end

  class MissingArgument < OptionParser::ParseError
    sig { params(args: T.untyped).void }
    def initialize(*args); end

    sig { returns(T.untyped) }
    def args(); end

    sig { params(value: T.untyped).returns(T.untyped) }
    def reason=(value); end

    sig { params(argv: T.untyped).returns(T.untyped) }
    def recover(argv); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def self.filter_backtrace(array); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def set_backtrace(array); end

    sig { params(opt: T.untyped, eq: T.untyped).returns(T.untyped) }
    def set_option(opt, eq); end

    sig { returns(T.untyped) }
    def reason(); end

    sig { returns(T.untyped) }
    def inspect(); end

    sig { returns(T.untyped) }
    def message(); end
  end

  class InvalidOption < OptionParser::ParseError
    sig { params(args: T.untyped).void }
    def initialize(*args); end

    sig { returns(T.untyped) }
    def args(); end

    sig { params(value: T.untyped).returns(T.untyped) }
    def reason=(value); end

    sig { params(argv: T.untyped).returns(T.untyped) }
    def recover(argv); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def self.filter_backtrace(array); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def set_backtrace(array); end

    sig { params(opt: T.untyped, eq: T.untyped).returns(T.untyped) }
    def set_option(opt, eq); end

    sig { returns(T.untyped) }
    def reason(); end

    sig { returns(T.untyped) }
    def inspect(); end

    sig { returns(T.untyped) }
    def message(); end
  end

  class InvalidArgument < OptionParser::ParseError
    sig { params(args: T.untyped).void }
    def initialize(*args); end

    sig { returns(T.untyped) }
    def args(); end

    sig { params(value: T.untyped).returns(T.untyped) }
    def reason=(value); end

    sig { params(argv: T.untyped).returns(T.untyped) }
    def recover(argv); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def self.filter_backtrace(array); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def set_backtrace(array); end

    sig { params(opt: T.untyped, eq: T.untyped).returns(T.untyped) }
    def set_option(opt, eq); end

    sig { returns(T.untyped) }
    def reason(); end

    sig { returns(T.untyped) }
    def inspect(); end

    sig { returns(T.untyped) }
    def message(); end
  end

  class AmbiguousArgument < OptionParser::InvalidArgument
    sig { params(args: T.untyped).void }
    def initialize(*args); end

    sig { returns(T.untyped) }
    def args(); end

    sig { params(value: T.untyped).returns(T.untyped) }
    def reason=(value); end

    sig { params(argv: T.untyped).returns(T.untyped) }
    def recover(argv); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def self.filter_backtrace(array); end

    sig { params(array: T.untyped).returns(T.untyped) }
    def set_backtrace(array); end

    sig { params(opt: T.untyped, eq: T.untyped).returns(T.untyped) }
    def set_option(opt, eq); end

    sig { returns(T.untyped) }
    def reason(); end

    sig { returns(T.untyped) }
    def inspect(); end

    sig { returns(T.untyped) }
    def message(); end
  end

  module Arguable
    sig { params(opt: T.untyped).returns(T.untyped) }
    def options=(opt); end

    sig { returns(T.untyped) }
    def options(); end

    sig { params(blk: T.untyped).returns(T.untyped) }
    def order!(&blk); end

    sig { returns(T.untyped) }
    def permute!(); end

    sig { returns(T.untyped) }
    def parse!(); end

    sig { params(args: T.untyped).returns(T.untyped) }
    def getopts(*args); end

    sig { params(obj: T.untyped).returns(T.untyped) }
    def self.extend_object(obj); end

    sig { params(args: T.untyped).void }
    def initialize(*args); end
  end

  module Acceptables
  end
end
