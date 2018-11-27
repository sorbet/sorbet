class OptionParser
  class NeedlessArgument < OptionParser::ParseError
    Reason = T.let(_, String)
  end

  class MissingArgument < OptionParser::ParseError
    Reason = T.let(_, String)
  end

  class CompletingHash < Hash
    include(OptionParser::Completion)

    def match(key)
    end
  end

  class AmbiguousArgument < OptionParser::InvalidArgument
    Reason = T.let(_, String)
  end

  ArgumentStyle = T.let(_, Hash)

  DefaultList = T.let(_, OptionParser::List)

  COMPSYS_HEADER = T.let(_, String)

  Officious = T.let(_, Hash)

  class List
    def complete(id, opt, icase = _, *pat, &block)
    end

    def add_banner(to)
    end

    def compsys(*args, &block)
    end

    def accept(t, pat = _, &block)
    end

    def atype()
    end

    def prepend(*args)
    end

    def each_option(&block)
    end

    def short()
    end

    def long()
    end

    def reject(t)
    end

    def append(*args)
    end

    def search(id, key)
    end

    def list()
    end

    def summarize(*args, &block)
    end
  end

  class AmbiguousOption < OptionParser::ParseError
    Reason = T.let(_, String)
  end

  NoArgument = T.let(_, Array)

  NO_ARGUMENT = T.let(_, Symbol)

  OctalInteger = T.let(_, Regexp)

  DecimalNumeric = T.let(_, Regexp)

  DecimalInteger = T.let(_, Regexp)

  REQUIRED_ARGUMENT = T.let(_, Symbol)

  OptionalArgument = T.let(_, Array)

  OPTIONAL_ARGUMENT = T.let(_, Symbol)

  class InvalidOption < OptionParser::ParseError
    Reason = T.let(_, String)
  end

  module Completion
    def complete(key, icase = _, pat = _)
    end

    def candidate(key, icase = _, pat = _)
    end

    def convert(opt = _, val = _, *_)
    end
  end

  module Arguable
    def options()
    end

    def options=(opt)
    end

    def order!(&blk)
    end

    def permute!()
    end

    def parse!()
    end

    def getopts(*args)
    end
  end

  class ParseError < RuntimeError
    Reason = T.let(_, String)

    def reason()
    end

    def recover(argv)
    end

    def args()
    end

    def set_option(opt, eq)
    end

    def message()
    end

    def reason=(_)
    end

    def inspect()
    end

    def set_backtrace(array)
    end

    def to_s()
    end
  end

  RequiredArgument = T.let(_, Array)

  module Acceptables

  end

  SPLAT_PROC = T.let(_, Proc)

  class OptionMap < Hash
    include(OptionParser::Completion)
  end

  class Switch
    class OptionalArgument < OptionParser::Switch
      def parse(arg, argv, &error)
      end
    end

    class PlacedArgument < OptionParser::Switch
      def parse(arg, argv, &error)
      end
    end

    class NoArgument < OptionParser::Switch
      def parse(arg, argv)
      end
    end

    class RequiredArgument < OptionParser::Switch
      def parse(arg, argv)
      end
    end

    def add_banner(to)
    end

    def match_nonswitch?(str)
    end

    def compsys(sdone, ldone)
    end

    def desc()
    end

    def conv()
    end

    def long()
    end

    def arg()
    end

    def switch_name()
    end

    def short()
    end

    def pattern()
    end

    def block()
    end

    def summarize(sdone = _, ldone = _, width = _, max = _, indent = _)
    end
  end

  class InvalidArgument < OptionParser::ParseError
    Reason = T.let(_, String)
  end

  def remove()
  end

  def compsys(to, name = _)
  end

  def accept(*args, &blk)
  end

  def warn(mesg = _)
  end

  def new()
  end

  def top()
  end

  def environment(env = _)
  end

  def program_name()
  end

  def inc(*args)
  end

  def parse(*argv, into: _)
  end

  def add_officious()
  end

  def terminate(arg = _)
  end

  def summary_indent()
  end

  def default_argv()
  end

  def set_banner(_)
  end

  def banner=(_)
  end

  def banner()
  end

  def program_name=(_)
  end

  def set_summary_width(_)
  end

  def summary_width=(_)
  end

  def set_summary_indent(_)
  end

  def summary_indent=(_)
  end

  def set_program_name(_)
  end

  def help()
  end

  def version()
  end

  def summary_width()
  end

  def make_switch(opts, block = _)
  end

  def base()
  end

  def define(*opts, &block)
  end

  def on(*opts, &block)
  end

  def def_option(*opts, &block)
  end

  def define_head(*opts, &block)
  end

  def on_head(*opts, &block)
  end

  def def_head_option(*opts, &block)
  end

  def define_tail(*opts, &block)
  end

  def on_tail(*opts, &block)
  end

  def def_tail_option(*opts, &block)
  end

  def separator(string)
  end

  def order(*argv, into: _, &nonopt)
  end

  def release()
  end

  def order!(argv = _, into: _, &nonopt)
  end

  def load(filename = _)
  end

  def to_a()
  end

  def to_s()
  end

  def permute(*argv, into: _)
  end

  def permute!(argv = _, into: _)
  end

  def parse!(argv = _, into: _)
  end

  def version=(_)
  end

  def getopts(*args)
  end

  def reject(*args, &blk)
  end

  def abort(mesg = _)
  end

  def default_argv=(_)
  end

  def candidate(word)
  end

  def ver()
  end

  def release=(_)
  end

  def summarize(to = _, width = _, max = _, indent = _, &blk)
  end
end
