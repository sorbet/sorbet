# typed: strict
class PrettyPrint
  def self.format(output = nil, maxwidth = nil, newline = nil, genspace = nil); end
  def self.singleline_format(output = nil, maxwidth = nil, newline = nil, genspace = nil); end
  def break_outmost_groups; end
  def breakable(sep = nil, width = nil); end
  def current_group; end
  def fill_breakable(sep = nil, width = nil); end
  def flush; end
  def genspace; end
  def group(indent = nil, open_obj = nil, close_obj = nil, open_width = nil, close_width = nil); end
  def group_queue; end
  def group_sub; end
  def indent; end
  def initialize(output = nil, maxwidth = nil, newline = nil, &genspace); end
  def maxwidth; end
  def nest(indent); end
  def newline; end
  def output; end
  def text(obj, width = nil); end
end
class PrettyPrint::Text
  def add(obj, width); end
  def initialize; end
  def output(out, output_width); end
  def width; end
end
class PrettyPrint::Breakable
  def indent; end
  def initialize(sep, width, q); end
  def obj; end
  def output(out, output_width); end
  def width; end
end
class PrettyPrint::Group
  def break; end
  def break?; end
  def breakables; end
  def depth; end
  def first?; end
  def initialize(depth); end
end
class PrettyPrint::GroupQueue
  def delete(group); end
  def deq; end
  def enq(group); end
  def initialize(*groups); end
end
class PrettyPrint::SingleLine
  def breakable(sep = nil, width = nil); end
  def first?; end
  def flush; end
  def group(indent = nil, open_obj = nil, close_obj = nil, open_width = nil, close_width = nil); end
  def initialize(output, maxwidth = nil, newline = nil); end
  def nest(indent); end
  def text(obj, width = nil); end
end
class PP < PrettyPrint
  include PP::PPMethods
  def self.mcall(obj, mod, meth, *args, &block); end
  def self.pp(obj, out = nil, width = nil); end
  def self.sharing_detection; end
  def self.sharing_detection=(arg0); end
  def self.singleline_pp(obj, out = nil); end
end
module PP::PPMethods
  def check_inspect_key(id); end
  def comma_breakable; end
  def guard_inspect_key; end
  def object_address_group(obj, &block); end
  def object_group(obj, &block); end
  def pop_inspect_key(id); end
  def pp(obj); end
  def pp_hash(obj); end
  def pp_object(obj); end
  def push_inspect_key(id); end
  def seplist(list, sep = nil, iter_method = nil); end
end
class PP::SingleLine < PrettyPrint::SingleLine
  include PP::PPMethods
end
module PP::ObjectMixin
  def pretty_print(q); end
  def pretty_print_cycle(q); end
  def pretty_print_inspect; end
  def pretty_print_instance_variables; end
end
