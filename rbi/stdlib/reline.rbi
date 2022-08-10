# typed: __STDLIB_INTERNAL

module Reline
  def eof?(*args, **, &block); end
  def readline(*args, **, &block); end
  def readmultiline(*args, **, &block); end
  def self.add_dialog_proc(*args, **, &block); end
  def self.ambiguous_width(*args, **, &block); end
  def self.auto_indent_proc(*args, **, &block); end
  def self.auto_indent_proc=(*args, **, &block); end
  def self.autocompletion(*args, **, &block); end
  def self.autocompletion=(*args, **, &block); end
  def self.basic_quote_characters(*args, **, &block); end
  def self.basic_quote_characters=(*args, **, &block); end
  def self.basic_word_break_characters(*args, **, &block); end
  def self.basic_word_break_characters=(*args, **, &block); end
  def self.completer_quote_characters(*args, **, &block); end
  def self.completer_quote_characters=(*args, **, &block); end
  def self.completer_word_break_characters(*args, **, &block); end
  def self.completer_word_break_characters=(*args, **, &block); end
  def self.completion_append_character(*args, **, &block); end
  def self.completion_append_character=(*args, **, &block); end
  def self.completion_case_fold(*args, **, &block); end
  def self.completion_case_fold=(*args, **, &block); end
  def self.completion_proc(*args, **, &block); end
  def self.completion_proc=(*args, **, &block); end
  def self.completion_quote_character(*args, **, &block); end
  def self.core; end
  def self.delete_text(*args, **, &block); end
  def self.dialog_proc(*args, **, &block); end
  def self.dig_perfect_match_proc(*args, **, &block); end
  def self.dig_perfect_match_proc=(*args, **, &block); end
  def self.emacs_editing_mode(*args, **, &block); end
  def self.emacs_editing_mode?(*args, **, &block); end
  def self.encoding_system_needs; end
  def self.eof?(*args, **, &block); end
  def self.filename_quote_characters(*args, **, &block); end
  def self.filename_quote_characters=(*args, **, &block); end
  def self.get_screen_size(*args, **, &block); end
  def self.input=(*args, **, &block); end
  def self.insert_text(*args, &block); end
  def self.last_incremental_search(*args, **, &block); end
  def self.last_incremental_search=(*args, **, &block); end
  def self.line_buffer(*args, **, &block); end
  def self.line_editor; end
  def self.output=(*args, **, &block); end
  def self.output_modifier_proc(*args, **, &block); end
  def self.output_modifier_proc=(*args, **, &block); end
  def self.point(*args, **, &block); end
  def self.point=(*args, **, &block); end
  def self.pre_input_hook(*args, **, &block); end
  def self.pre_input_hook=(*args, **, &block); end
  def self.prompt_proc(*args, **, &block); end
  def self.prompt_proc=(*args, **, &block); end
  def self.readline(*args, **, &block); end
  def self.readmultiline(*args, **, &block); end
  def self.redisplay(*args, **, &block); end
  def self.special_prefixes(*args, **, &block); end
  def self.special_prefixes=(*args, **, &block); end
  def self.ungetc(c); end
  def self.vi_editing_mode(*args, **, &block); end
  def self.vi_editing_mode?(*args, **, &block); end
  extend Forwardable
  extend SingleForwardable
end
class Reline::Config
  def add_default_key_binding(keystroke, target); end
  def add_default_key_binding_by_keymap(keymap, keystroke, target); end
  def add_oneshot_key_binding(keystroke, target); end
  def autocompletion; end
  def autocompletion=(val); end
  def bind_key(key, func_name); end
  def bind_tty_special_chars; end
  def bind_tty_special_chars=(arg0); end
  def bind_variable(name, value); end
  def blink_matching_paren; end
  def blink_matching_paren=(arg0); end
  def byte_oriented; end
  def byte_oriented=(arg0); end
  def completion_ignore_case; end
  def completion_ignore_case=(arg0); end
  def convert_meta; end
  def convert_meta=(arg0); end
  def default_inputrc_path; end
  def disable_completion; end
  def disable_completion=(arg0); end
  def editing_mode; end
  def editing_mode=(val); end
  def editing_mode_is?(*val); end
  def emacs_mode_string; end
  def emacs_mode_string=(arg0); end
  def enable_bracketed_paste; end
  def enable_bracketed_paste=(arg0); end
  def enable_keypad; end
  def enable_keypad=(arg0); end
  def expand_tilde; end
  def expand_tilde=(arg0); end
  def handle_directive(directive, file, no); end
  def history_preserve_point; end
  def history_preserve_point=(arg0); end
  def history_size; end
  def history_size=(arg0); end
  def horizontal_scroll_mode; end
  def horizontal_scroll_mode=(arg0); end
  def initialize; end
  def input_meta; end
  def input_meta=(arg0); end
  def inputrc_path; end
  def isearch_terminators; end
  def isearch_terminators=(arg0); end
  def key_bindings; end
  def key_notation_to_code(notation); end
  def keymap; end
  def keyseq_timeout; end
  def keyseq_timeout=(arg0); end
  def mark_directories; end
  def mark_directories=(arg0); end
  def mark_modified_lines; end
  def mark_modified_lines=(arg0); end
  def mark_symlinked_directories; end
  def mark_symlinked_directories=(arg0); end
  def match_hidden_files; end
  def match_hidden_files=(arg0); end
  def meta_flag; end
  def meta_flag=(arg0); end
  def output_meta; end
  def output_meta=(arg0); end
  def page_completions; end
  def page_completions=(arg0); end
  def parse_keyseq(str); end
  def prefer_visible_bell; end
  def prefer_visible_bell=(arg0); end
  def print_completions_horizontally; end
  def print_completions_horizontally=(arg0); end
  def read(file = nil); end
  def read_lines(lines, file = nil); end
  def reset; end
  def reset_default_key_bindings; end
  def reset_oneshot_key_bindings; end
  def retrieve_string(str); end
  def seven_bit_encoding?(encoding); end
  def show_all_if_ambiguous; end
  def show_all_if_ambiguous=(arg0); end
  def show_all_if_unmodified; end
  def show_all_if_unmodified=(arg0); end
  def show_mode_in_prompt; end
  def show_mode_in_prompt=(arg0); end
  def test_mode; end
  def vi_cmd_mode_string; end
  def vi_cmd_mode_string=(arg0); end
  def vi_ins_mode_string; end
  def vi_ins_mode_string=(arg0); end
  def visible_stats; end
  def visible_stats=(arg0); end
end
class Reline::Config::InvalidInputrc < RuntimeError
  def file; end
  def file=(arg0); end
  def lineno; end
  def lineno=(arg0); end
end
class Reline::KeyActor::Base
  def default_key_bindings; end
  def get_method(key); end
  def initialize; end
  def reset_default_key_bindings; end
end
class Reline::KeyActor::Emacs < Reline::KeyActor::Base
end
class Reline::KeyActor::ViCommand < Reline::KeyActor::Base
end
class Reline::KeyActor::ViInsert < Reline::KeyActor::Base
end
module Reline::KeyActor
end
class Reline::KeyStroke
  def compress_meta_key(ary); end
  def equal?(me, other); end
  def expand(input); end
  def initialize(config); end
  def key_mapping; end
  def match_status(input); end
  def start_with?(me, other); end
end
class Reline::KillRing
  def append(string, before_p = nil); end
  def each; end
  def initialize(max = nil); end
  def process; end
  def yank; end
  def yank_pop; end
  include Enumerable
  Elem = type_member(:out)
end
module Reline::KillRing::State
end
class Reline::KillRing::RingPoint < Struct
  def ==(other); end
  def backward; end
  def backward=(_); end
  def forward; end
  def forward=(_); end
  def initialize(str); end
  def self.[](*arg0); end
  def self.inspect; end
  def self.keyword_init?; end
  def self.members; end
  def self.new(*arg0); end
  def str; end
  def str=(_); end
  Elem = type_member(:out) {{fixed: T.untyped}}
end
class Reline::KillRing::RingBuffer
  def <<(point); end
  def empty?; end
  def head; end
  def initialize(max = nil); end
  def size; end
end
class Reline::Unicode::EastAsianWidth
end
class Reline::Unicode
  def self.calculate_width(str, allow_escape_code = nil); end
  def self.ed_transpose_words(line, byte_pointer); end
  def self.em_backward_word(line, byte_pointer); end
  def self.em_big_backward_word(line, byte_pointer); end
  def self.em_forward_word(line, byte_pointer); end
  def self.em_forward_word_with_capitalization(line, byte_pointer); end
  def self.escape_for_print(str); end
  def self.get_mbchar_byte_size_by_first_char(c); end
  def self.get_mbchar_width(mbchar); end
  def self.get_next_mbchar_size(line, byte_pointer); end
  def self.get_prev_mbchar_size(line, byte_pointer); end
  def self.split_by_width(str, max_width, encoding = nil); end
  def self.take_range(str, start_col, max_width, encoding = nil); end
  def self.vi_backward_word(line, byte_pointer); end
  def self.vi_big_backward_word(line, byte_pointer); end
  def self.vi_big_forward_end_word(line, byte_pointer); end
  def self.vi_big_forward_word(line, byte_pointer); end
  def self.vi_first_print(line); end
  def self.vi_forward_end_word(line, byte_pointer); end
  def self.vi_forward_word(line, byte_pointer, drop_terminate_spaces = nil); end
end
class Reline::LineEditor
  def add_dialog_proc(name, p, context = nil); end
  def argumentable?(method_obj); end
  def auto_indent_proc; end
  def auto_indent_proc=(arg0); end
  def backward_char(key, arg: nil); end
  def backward_delete_char(key, arg: nil); end
  def backward_word(key); end
  def beginning_of_line(key); end
  def byte_pointer; end
  def byte_pointer=(val); end
  def byteinsert(str, byte_pointer, other); end
  def byteslice!(str, byte_pointer, size); end
  def calculate_height_by_lines(lines, prompt); end
  def calculate_height_by_width(width); end
  def calculate_nearest_cursor(line_to_calc = nil, cursor = nil, started_from = nil, byte_pointer = nil, update = nil); end
  def calculate_scroll_partial_screen(highest_in_all, cursor_y); end
  def calculate_width(str, allow_escape_code = nil); end
  def call_completion_proc; end
  def call_completion_proc_with_checking_args(pre, target, post); end
  def capitalize_word(key); end
  def check_mode_string; end
  def check_multiline_prompt(buffer); end
  def clear_dialog; end
  def clear_each_dialog(dialog); end
  def clear_screen(key); end
  def clear_screen_buffer(prompt, prompt_list, prompt_width); end
  def complete(list, just_show_list = nil); end
  def complete_internal_proc(list, is_menu); end
  def completion_append_character; end
  def completion_append_character=(arg0); end
  def completion_proc; end
  def completion_proc=(arg0); end
  def confirm_multiline_termination; end
  def confirm_multiline_termination_proc; end
  def confirm_multiline_termination_proc=(arg0); end
  def copy_for_vi(text); end
  def delete_char(key); end
  def delete_char_or_list(key); end
  def delete_text(start = nil, length = nil); end
  def dig_perfect_match_proc; end
  def dig_perfect_match_proc=(arg0); end
  def downcase_word(key); end
  def ed_argument_digit(key); end
  def ed_clear_screen(key); end
  def ed_delete_next_char(key, arg: nil); end
  def ed_delete_prev_char(key, arg: nil); end
  def ed_delete_prev_word(key); end
  def ed_digit(key); end
  def ed_insert(key); end
  def ed_kill_line(key); end
  def ed_move_to_beg(key); end
  def ed_move_to_end(key); end
  def ed_newline(key); end
  def ed_next_char(key, arg: nil); end
  def ed_next_history(key, arg: nil); end
  def ed_prev_char(key, arg: nil); end
  def ed_prev_history(key, arg: nil); end
  def ed_prev_word(key); end
  def ed_quoted_insert(str, arg: nil); end
  def ed_search_next_history(key, arg: nil); end
  def ed_search_prev_history(key, arg: nil); end
  def ed_transpose_chars(key); end
  def ed_transpose_words(key); end
  def ed_unassigned(key); end
  def editing_mode; end
  def em_capitol_case(key); end
  def em_delete(key); end
  def em_delete_next_word(key); end
  def em_delete_or_list(key); end
  def em_delete_prev_char(key, arg: nil); end
  def em_exchange_mark(key); end
  def em_kill_line(key); end
  def em_kill_region(key); end
  def em_lower_case(key); end
  def em_next_word(key); end
  def em_set_mark(key); end
  def em_upper_case(key); end
  def em_yank(key); end
  def em_yank_pop(key); end
  def end_of_line(key); end
  def eof?; end
  def exchange_point_and_mark(key); end
  def finalize; end
  def finish; end
  def finished?; end
  def forward_char(key, arg: nil); end
  def forward_search_history(key); end
  def forward_word(key); end
  def generate_searcher; end
  def history_search_backward(key, arg: nil); end
  def history_search_forward(key, arg: nil); end
  def inclusive?(method_obj); end
  def incremental_search_history(key); end
  def initialize(config, encoding); end
  def input_key(key); end
  def insert_new_line(cursor_line, next_line); end
  def insert_text(text); end
  def just_move_cursor; end
  def key_delete(key); end
  def key_newline(key); end
  def kill_line(key); end
  def kill_whole_line(key); end
  def line; end
  def menu(target, list); end
  def modify_lines(before); end
  def move_completed_list(list, direction); end
  def move_cursor_down(val); end
  def move_cursor_up(val); end
  def multiline_off; end
  def multiline_on; end
  def next_history(key, arg: nil); end
  def normal_char(key); end
  def output=(arg0); end
  def output_modifier_proc; end
  def output_modifier_proc=(arg0); end
  def padding_space_with_escape_sequences(str, width); end
  def pre_input_hook; end
  def pre_input_hook=(arg0); end
  def previous_history(key, arg: nil); end
  def process_auto_indent; end
  def process_insert(force: nil); end
  def process_key(key, method_symbol); end
  def prompt_proc; end
  def prompt_proc=(arg0); end
  def quoted_insert(str, arg: nil); end
  def render_dialog(cursor_column); end
  def render_each_dialog(dialog, cursor_column); end
  def render_partial(prompt, prompt_width, line_to_render, this_started_from, with_control: nil); end
  def render_whole_lines(lines, prompt, prompt_width); end
  def rerender; end
  def rerender_added_newline(prompt, prompt_width); end
  def rerender_all; end
  def rerender_all_lines; end
  def rerender_changed_current_line; end
  def reset(prompt = nil, encoding:); end
  def reset_dialog(dialog, old_dialog); end
  def reset_line; end
  def reset_variables(prompt = nil, encoding:); end
  def resize; end
  def retrieve_completion_block(set_completion_quote_character = nil); end
  def reverse_search_history(key); end
  def run_for_operators(key, method_symbol, &block); end
  def scroll_down(val); end
  def search_next_char(key, arg, need_prev_char: nil, inclusive: nil); end
  def search_prev_char(key, arg, need_next_char = nil); end
  def self_insert(key); end
  def set_mark(key); end
  def set_pasting_state(in_pasting); end
  def set_signal_handlers; end
  def show_menu; end
  def simplified_rendering?; end
  def split_by_width(str, max_width); end
  def transpose_chars(key); end
  def transpose_words(key); end
  def unix_line_discard(key); end
  def unix_word_rubout(key); end
  def upcase_word(key); end
  def vi_add(key); end
  def vi_add_at_eol(key); end
  def vi_change_meta(key, arg: nil); end
  def vi_command_mode(key); end
  def vi_delete_meta(key, arg: nil); end
  def vi_delete_prev_char(key); end
  def vi_end_big_word(key, arg: nil, inclusive: nil); end
  def vi_end_of_transmission(key); end
  def vi_end_word(key, arg: nil, inclusive: nil); end
  def vi_eof_maybe(key); end
  def vi_first_print(key); end
  def vi_histedit(key); end
  def vi_insert(key); end
  def vi_insert_at_bol(key); end
  def vi_join_lines(key, arg: nil); end
  def vi_kill_line_prev(key); end
  def vi_list_or_eof(key); end
  def vi_movement_mode(key); end
  def vi_next_big_word(key, arg: nil); end
  def vi_next_char(key, arg: nil, inclusive: nil); end
  def vi_next_word(key, arg: nil); end
  def vi_paste_next(key, arg: nil); end
  def vi_paste_prev(key, arg: nil); end
  def vi_prev_big_word(key, arg: nil); end
  def vi_prev_char(key, arg: nil); end
  def vi_prev_word(key, arg: nil); end
  def vi_replace_char(key, arg: nil); end
  def vi_search_next(key); end
  def vi_search_prev(key); end
  def vi_to_column(key, arg: nil); end
  def vi_to_history_line(key); end
  def vi_to_next_char(key, arg: nil, inclusive: nil); end
  def vi_to_prev_char(key, arg: nil); end
  def vi_yank(key, arg: nil); end
  def vi_zero(key); end
  def whole_buffer; end
  def whole_lines(index: nil, line: nil); end
  def wrap_method_call(method_symbol, method_obj, key, with_operator = nil); end
  def yank(key); end
  def yank_pop(key); end
end
module Reline::LineEditor::CompletionState
end
class Struct::CompletionJourneyData < Struct
  def list; end
  def list=(_); end
  def pointer; end
  def pointer=(_); end
  def postposing; end
  def postposing=(_); end
  def preposing; end
  def preposing=(_); end
  def self.[](*arg0); end
  def self.inspect; end
  def self.keyword_init?; end
  def self.members; end
  def self.new(*arg0); end
  Elem = type_member(:out) {{fixed: T.untyped}}
end
class Struct::MenuInfo < Struct
  def list; end
  def list=(_); end
  def self.[](*arg0); end
  def self.inspect; end
  def self.keyword_init?; end
  def self.members; end
  def self.new(*arg0); end
  def target; end
  def target=(_); end
  Elem = type_member(:out) {{fixed: T.untyped}}
end
class Reline::LineEditor::DialogProcScope
  def call; end
  def call_completion_proc_with_checking_args(pre, target, post); end
  def completion_journey_data; end
  def config; end
  def context; end
  def cursor_pos; end
  def dialog; end
  def initialize(line_editor, config, proc_to_exec, context); end
  def just_cursor_moving; end
  def key; end
  def retrieve_completion_block(set_completion_quote_character = nil); end
  def screen_width; end
  def set_cursor_pos(col, row); end
  def set_dialog(dialog); end
  def set_key(key); end
end
class Reline::LineEditor::Dialog
  def call(key); end
  def column; end
  def column=(arg0); end
  def contents; end
  def contents=(contents); end
  def initialize(name, config, proc_scope); end
  def lines_backup; end
  def lines_backup=(arg0); end
  def name; end
  def pointer; end
  def pointer=(arg0); end
  def scroll_top; end
  def scroll_top=(arg0); end
  def scrollbar_pos; end
  def scrollbar_pos=(arg0); end
  def set_cursor_pos(col, row); end
  def trap_key; end
  def trap_key=(arg0); end
  def vertical_offset; end
  def vertical_offset=(arg0); end
  def width; end
  def width=(v); end
end
class Reline::History < Array
  def <<(val); end
  def [](index); end
  def []=(index, val); end
  def check_index(index); end
  def concat(*val); end
  def delete_at(index); end
  def initialize(config); end
  def push(*val); end
  def to_s; end
  Elem = type_member {{fixed: String}}
end
module Reline::Terminfo
  def self.curses_dl; end
  def self.curses_dl_files; end
  def self.enabled?; end
  def self.setupterm(term, fildes); end
  def self.tigetflag(capname); end
  def self.tigetnum(capname); end
  def self.tigetstr(capname); end
  def self.tiparm(str, *args); end
  extend Fiddle::Importer
end
class Reline::Terminfo::TerminfoError < StandardError
end
class Reline::Terminfo::StringWithTiparm < String
  def tiparm(*args); end
end
class Reline::GeneralIO
  def self.clear_screen; end
  def self.cursor_pos; end
  def self.deprep(otio); end
  def self.encoding; end
  def self.erase_after_cursor; end
  def self.finish_pasting; end
  def self.get_screen_size; end
  def self.getc; end
  def self.in_pasting?; end
  def self.input=(val); end
  def self.move_cursor_column(val); end
  def self.move_cursor_down(val); end
  def self.move_cursor_up(val); end
  def self.prep; end
  def self.reset(encoding: nil); end
  def self.scroll_down(val); end
  def self.set_default_key_bindings(_); end
  def self.set_screen_size(rows, columns); end
  def self.set_winch_handler(&handler); end
  def self.start_pasting; end
  def self.ungetc(c); end
  def self.win?; end
end
class Reline::ANSI
  def self.clear_screen; end
  def self.cursor_pos; end
  def self.deprep(otio); end
  def self.empty_buffer?; end
  def self.encoding; end
  def self.erase_after_cursor; end
  def self.get_screen_size; end
  def self.getc; end
  def self.getc_with_bracketed_paste; end
  def self.hide_cursor; end
  def self.in_pasting?; end
  def self.inner_getc; end
  def self.input=(val); end
  def self.move_cursor_column(x); end
  def self.move_cursor_down(x); end
  def self.move_cursor_up(x); end
  def self.output=(val); end
  def self.prep; end
  def self.retrieve_keybuffer; end
  def self.scroll_down(x); end
  def self.set_default_key_bindings(config); end
  def self.set_default_key_bindings_comprehensive_list(config); end
  def self.set_default_key_bindings_terminfo(config); end
  def self.set_screen_size(rows, columns); end
  def self.set_winch_handler(&handler); end
  def self.show_cursor; end
  def self.ungetc(c); end
  def self.win?; end
end
class Reline::ConfigEncodingConversionError < StandardError
end
class Struct::Key < Struct
  def ==(other); end
  def char; end
  def char=(_); end
  def combined_char; end
  def combined_char=(_); end
  def match?(other); end
  def self.[](*arg0); end
  def self.inspect; end
  def self.keyword_init?; end
  def self.members; end
  def self.new(*arg0); end
  def with_meta; end
  def with_meta=(_); end
  Elem = type_member(:out) {{fixed: T.untyped}}
end
class Reline::CursorPos < Struct
  def self.[](*arg0); end
  def self.inspect; end
  def self.keyword_init?; end
  def self.members; end
  def self.new(*arg0); end
  def x; end
  def x=(_); end
  def y; end
  def y=(_); end
  Elem = type_member(:out) {{fixed: T.untyped}}
end
class Reline::DialogRenderInfo < Struct
  def bg_color; end
  def bg_color=(_); end
  def contents; end
  def contents=(_); end
  def height; end
  def height=(_); end
  def pos; end
  def pos=(_); end
  def scrollbar; end
  def scrollbar=(_); end
  def self.[](*arg0); end
  def self.inspect; end
  def self.keyword_init?; end
  def self.members; end
  def self.new(*arg0); end
  def width; end
  def width=(_); end
  Elem = type_member(:out) {{fixed: T.untyped}}
end
class Reline::Core
  def add_dialog_proc(name_sym, p, context = nil); end
  def ambiguous_width; end
  def auto_indent_proc; end
  def auto_indent_proc=(p); end
  def autocompletion; end
  def autocompletion=(val); end
  def basic_quote_characters; end
  def basic_quote_characters=(v); end
  def basic_word_break_characters; end
  def basic_word_break_characters=(v); end
  def completer_quote_characters; end
  def completer_quote_characters=(v); end
  def completer_word_break_characters; end
  def completer_word_break_characters=(v); end
  def completion_append_character; end
  def completion_append_character=(val); end
  def completion_case_fold; end
  def completion_case_fold=(v); end
  def completion_proc; end
  def completion_proc=(p); end
  def completion_quote_character; end
  def config; end
  def config=(arg0); end
  def dialog_proc(name_sym); end
  def dig_perfect_match_proc; end
  def dig_perfect_match_proc=(p); end
  def emacs_editing_mode; end
  def emacs_editing_mode?; end
  def encoding; end
  def filename_quote_characters; end
  def filename_quote_characters=(v); end
  def get_screen_size; end
  def initialize; end
  def inner_readline(prompt, add_hist, multiline, &confirm_multiline_termination); end
  def input=(val); end
  def key_stroke; end
  def key_stroke=(arg0); end
  def last_incremental_search; end
  def last_incremental_search=(arg0); end
  def line_editor; end
  def line_editor=(arg0); end
  def may_req_ambiguous_char_width; end
  def output; end
  def output=(val); end
  def output_modifier_proc; end
  def output_modifier_proc=(p); end
  def pre_input_hook; end
  def pre_input_hook=(p); end
  def prompt_proc; end
  def prompt_proc=(p); end
  def read_2nd_character_of_key_sequence(keyseq_timeout, buffer, c, block); end
  def read_escaped_key(keyseq_timeout, c, block); end
  def read_io(keyseq_timeout, &block); end
  def readline(prompt = nil, add_hist = nil); end
  def readmultiline(prompt = nil, add_hist = nil, &confirm_multiline_termination); end
  def special_prefixes; end
  def special_prefixes=(v); end
  def vi_editing_mode; end
  def vi_editing_mode?; end
end
class Reline::Core::DialogProc < Struct
  def context; end
  def context=(_); end
  def dialog_proc; end
  def dialog_proc=(_); end
  def self.[](*arg0); end
  def self.inspect; end
  def self.keyword_init?; end
  def self.members; end
  def self.new(*arg0); end
  Elem = type_member(:out) {{fixed: T.untyped}}
end
