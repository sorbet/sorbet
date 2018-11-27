# typed: true

module IRB
  IRBRC_EXT = T.let(T.unsafe(nil), String)

  class ReadlineInputMethod < ::IRB::InputMethod
    include(Readline)

    def line(line_no)
    end

    def eof?()
    end

    def encoding()
    end

    def readable_after_eof?()
    end

    def gets()
    end
  end

  class Abort < ::Exception

  end

  class Inspector
    INSPECTORS = T.let(T.unsafe(nil), Hash)

    def init()
    end

    def inspect_value(v)
    end
  end

  class SLex
    extend(Exception2MessageMapper)

    D_DEBUG = T.let(T.unsafe(nil), IRB::Notifier::LeveledNotifier)

    D_DETAIL = T.let(T.unsafe(nil), IRB::Notifier::LeveledNotifier)

    class Node
      def match(chrs, op = _)
      end

      def preproc()
      end

      def postproc()
      end

      def search(chrs, opt = _)
      end

      def preproc=(_)
      end

      def postproc=(_)
      end

      def create_subnode(chrs, preproc = _, postproc = _)
      end

      def match_io(io, op = _)
      end
    end

    class ErrNodeNothing < ::StandardError

    end

    class ErrNodeAlreadyExists < ::StandardError

    end

    DOUT = T.let(T.unsafe(nil), IRB::Notifier::CompositeNotifier)

    D_WARN = T.let(T.unsafe(nil), IRB::Notifier::LeveledNotifier)

    def create(token, preproc = _, postproc = _)
    end

    def match(token)
    end

    def preproc(token, proc)
    end

    def postproc(token)
    end

    def def_rules(*tokens, &block)
    end

    def search(token)
    end

    def def_rule(token, preproc = _, postproc = _, &block)
    end

    def inspect()
    end

    def Raise(err = _, *rest)
    end

    def Fail(err = _, *rest)
    end
  end

  class WorkSpace
    def code_around_binding()
    end

    def main()
    end

    def binding()
    end

    def evaluate(context, statements, file = _, line = _)
    end

    def filter_backtrace(bt)
    end
  end

  class DefaultEncodings < ::Struct
    def internal=(_)
    end

    def external()
    end

    def internal()
    end

    def external=(_)
    end
  end

  class StdioInputMethod < ::IRB::InputMethod
    def line(line_no)
    end

    def eof?()
    end

    def encoding()
    end

    def readable_after_eof?()
    end

    def gets()
    end
  end

  class FileInputMethod < ::IRB::InputMethod
    def gets()
    end

    def eof?()
    end

    def encoding()
    end

    def file_name()
    end
  end

  module Notifier
    extend(Exception2MessageMapper)

    class NoMsgNotifier < ::IRB::Notifier::LeveledNotifier
      def notify?()
      end
    end

    class ErrUndefinedNotifier < ::StandardError

    end

    class ErrUnrecognizedLevel < ::StandardError

    end

    class CompositeNotifier < ::IRB::Notifier::AbstractNotifier
      def level_notifier=(value)
      end

      def def_notifier(level, prefix = _)
      end

      def level=(value)
      end

      def notifiers()
      end

      def level()
      end

      def level_notifier()
      end
    end

    class AbstractNotifier
      def printn(*opts)
      end

      def ppx(prefix, *objs)
      end

      def exec_if()
      end

      def prefix()
      end

      def notify?()
      end

      def print(*opts)
      end

      def printf(format, *opts)
      end

      def puts(*objs)
      end

      def pp(*objs)
      end
    end

    D_NOMSG = T.let(T.unsafe(nil), IRB::Notifier::NoMsgNotifier)

    class LeveledNotifier < ::IRB::Notifier::AbstractNotifier
      include(Comparable)

      def notify?()
      end

      def <=>(other)
      end

      def level()
      end
    end

    def Raise(err = _, *rest)
    end

    def Fail(err = _, *rest)
    end
  end

  module ContextExtender

  end

  module MethodExtender
    def def_pre_proc(base_method, extend_method)
    end

    def def_post_proc(base_method, extend_method)
    end

    def new_alias_name(name, prefix = _, postfix = _)
    end
  end

  class StdioOutputMethod < ::IRB::OutputMethod
    def print(*opts)
    end
  end

  STDIN_FILE_NAME = T.let(T.unsafe(nil), String)

  class InputMethod
    def readable_after_eof?()
    end

    def gets()
    end

    def prompt()
    end

    def prompt=(_)
    end

    def file_name()
    end
  end

  class OutputMethod
    extend(Exception2MessageMapper)

    class NotImplementedError < ::StandardError

    end

    def parse_printf_format(format, opts)
    end

    def ppx(prefix, *objs)
    end

    def puts(*objs)
    end

    def printf(format, *opts)
    end

    def print(*opts)
    end

    def Raise(err = _, *rest)
    end

    def Fail(err = _, *rest)
    end

    def pp(*objs)
    end

    def printn(*opts)
    end
  end

  MagicFile = T.let(T.unsafe(nil), Object)

  class Irb
    ATTR_TTY = T.let(T.unsafe(nil), String)

    ATTR_PLAIN = T.let(T.unsafe(nil), String)

    def scanner()
    end

    def scanner=(_)
    end

    def signal_handle()
    end

    def eval_input()
    end

    def suspend_name(path = _, name = _)
    end

    def inspect()
    end

    def suspend_workspace(workspace)
    end

    def suspend_input_method(input_method)
    end

    def signal_status(status)
    end

    def context()
    end

    def suspend_context(context)
    end

    def prompt(prompt, ltype, indent, line_no)
    end

    def output_value()
    end

    def run(conf = _)
    end
  end

  class Context
    NO_INSPECTING_IVARS = T.let(T.unsafe(nil), Array)

    IDNAME_IVARS = T.let(T.unsafe(nil), Array)

    NOPRINTING_IVARS = T.let(T.unsafe(nil), Array)

    def prompt_mode=(mode)
    end

    def back_trace_limit()
    end

    def irb_path()
    end

    def irb_path=(_)
    end

    def irb_name()
    end

    def irb_name=(_)
    end

    def save_history=(*opts, &b)
    end

    def use_readline?()
    end

    def workspace=(_)
    end

    def debug_level=(value)
    end

    def main()
    end

    def use_readline()
    end

    def ignore_sigint()
    end

    def workspace_home()
    end

    def ignore_eof()
    end

    def echo()
    end

    def rc?()
    end

    def last_value()
    end

    def ignore_sigint?()
    end

    def return_format()
    end

    def io()
    end

    def inspect_last_value()
    end

    def set_last_value(value)
    end

    def exit(ret = _)
    end

    def __to_s__()
    end

    def inspect?()
    end

    def verbose=(_)
    end

    def file_input?()
    end

    def inspect()
    end

    def use_readline=(opt)
    end

    def debug?()
    end

    def __inspect__()
    end

    def load_modules()
    end

    def verbose()
    end

    def to_s()
    end

    def io=(_)
    end

    def irb=(_)
    end

    def thread()
    end

    def ap_name=(_)
    end

    def load_modules=(_)
    end

    def prompt_i=(_)
    end

    def workspace()
    end

    def prompt_s=(_)
    end

    def prompt_n=(_)
    end

    def rc=(_)
    end

    def irb()
    end

    def prompt_mode()
    end

    def auto_indent_mode=(_)
    end

    def return_format=(_)
    end

    def echo=(_)
    end

    def ignore_eof=(_)
    end

    def prompt_c=(_)
    end

    def back_trace_limit=(_)
    end

    def rc()
    end

    def prompt_s()
    end

    def prompt_c()
    end

    def prompt_n()
    end

    def prompt_i()
    end

    def ignore_sigint=(_)
    end

    def prompting?()
    end

    def auto_indent_mode()
    end

    def debug_level()
    end

    def verbose?()
    end

    def ignore_eof?()
    end

    def ap_name()
    end

    def inspect_mode()
    end

    def inspect_mode=(opt)
    end

    def evaluate(line, line_no)
    end

    def use_tracer=(*opts, &b)
    end

    def echo?()
    end

    def use_loader=(*opts, &b)
    end

    def eval_history=(*opts, &b)
    end
  end

  module ExtendCommandBundle
    NO_OVERRIDE = T.let(T.unsafe(nil), Integer)

    OVERRIDE_PRIVATE_ONLY = T.let(T.unsafe(nil), Integer)

    OVERRIDE_ALL = T.let(T.unsafe(nil), Integer)

    def irb_help(*opts, &b)
    end

    def irb_push_workspace(*opts, &b)
    end

    def irb_context()
    end

    def install_alias_method(to, from, override = _)
    end

    def irb_current_working_workspace(*opts, &b)
    end

    def irb_pop_workspace(*opts, &b)
    end

    def irb_exit(ret = _)
    end

    def irb_load(*opts, &b)
    end

    def irb_require(*opts, &b)
    end

    def irb_source(*opts, &b)
    end

    def irb_change_workspace(*opts, &b)
    end

    def irb_jobs(*opts, &b)
    end

    def irb_fg(*opts, &b)
    end

    def irb(*opts, &b)
    end

    def irb_kill(*opts, &b)
    end

    def irb_workspaces(*opts, &b)
    end
  end

  class Locale
    LOCALE_NAME_RE = T.let(T.unsafe(nil), Regexp)

    LOCALE_DIR = T.let(T.unsafe(nil), String)

    def require(file, priv = _)
    end

    def format(*opts)
    end

    def load(file, priv = _)
    end

    def String(mes)
    end

    def territory()
    end

    def modifier()
    end

    def lang()
    end

    def encoding()
    end

    def gets(*rs)
    end

    def printf(*opts)
    end

    def print(*opts)
    end

    def puts(*opts)
    end

    def readline(*rs)
    end

    def find(file, paths = _)
    end
  end
end
