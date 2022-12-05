# frozen_string_literal: true
# typed: true
require 'pp'

module T::Props::PrettyPrintable
  include T::Props::Plugin

  # Override the PP gem with something that's similar, but gives us a hook to do redaction and customization
  def pretty_print(pp)
    clazz = T.unsafe(T.cast(self, Object).class).decorator
    multiline = pp.is_a?(PP)
    pp.group(1, "<#{T.unsafe(self).class}", ">") do
      clazz.all_props.sort.each do |prop|
        pp.breakable
        val = clazz.get(self, prop)
        rules = clazz.prop_rules(prop)
        pp.text("#{prop}=")
        if (custom_inspect = rules[:inspect])
          inspected = if T::Utils.arity(custom_inspect) == 1
            custom_inspect.call(val)
          else
            custom_inspect.call(val, {multiline: multiline})
          end
          pp.text(inspected.nil? ? "nil" : "\"#{inspected}\"")
        elsif rules[:sensitivity] && !rules[:sensitivity].empty? && !val.nil?
          pp.text("<REDACTED #{rules[:sensitivity].join(', ')}>")
        else
          val.pretty_print(pp)
        end
      end
      clazz.pretty_print_extra(self, pp)
    end
  end

  # Overridable method to add anything that is not a prop
  def pretty_print_extra(instance, pp); end

  # Return a string representation of this object and all of its props in a single line
  def inspect
    string = +""
    PP.singleline_pp(self, string)
    string
  end

  # Return a pretty string representation of this object and all of its props
  def pretty_inspect
    string = +""
    PP.pp(self, string)
    string
  end

  module DecoratorMethods
    extend T::Sig

    sig {params(key: Symbol).returns(T::Boolean).checked(:never)}
    def valid_rule_key?(key)
      super || key == :inspect
    end
  end
end
