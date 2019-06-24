# typed: true

String.module_eval { puts self }
Enumerable.class_eval { |mod| puts mod }
