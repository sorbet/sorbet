#include "internal.h"
#include "ruby.h"

VALUE rb_mT;
VALUE rb_mT_Configuration;
VALUE rb_mT_Props;
VALUE rb_mT_Props_CustomType;
VALUE rb_mT_Props_Private;
VALUE rb_mT_Props_Private_DeserializerGenerator;
VALUE rb_mT_Props_Private_SerializerGenerator;
VALUE rb_mT_Props_Serializable;
VALUE rb_mT_Props_Utils;
VALUE rb_mT_Types;
VALUE rb_cT_Types_Base;
VALUE rb_cT_Types_Enum;
VALUE rb_cT_Types_Intersection;
VALUE rb_cT_Types_Simple;
VALUE rb_cT_Types_TypedArray;
VALUE rb_cT_Types_TypedEnumerable;
VALUE rb_cT_Types_TypedHash;
VALUE rb_cT_Types_TypedSet;
VALUE rb_cT_Types_Union;
VALUE rb_mT_Utils;
VALUE rb_mT_Utils_Nilable;

extern long sorbet_globalConstRegister(VALUE);

#define GCABLE_VALUE(name, expr)          \
    do {                                  \
        name = expr;                      \
        sorbet_globalConstRegister(name); \
    } while (0)

void Init_Sorbet_T() {
    // Set up modules that are defined by the sorbet-runtime gem.  Note that Ruby
    // will complain about mismatches if we get e.g. the class inheritance
    // structure wrong relative to how it is actually defined in sorbet-runtime.

    GCABLE_VALUE(rb_mT, rb_define_module("T"));

    GCABLE_VALUE(rb_mT_Configuration, rb_define_module_under(rb_mT, "Configuration"));

    GCABLE_VALUE(rb_mT_Props, rb_define_module_under(rb_mT, "Props"));
    GCABLE_VALUE(rb_mT_Props_CustomType, rb_define_module_under(rb_mT_Props, "CustomType"));
    GCABLE_VALUE(rb_mT_Props_Private, rb_define_module_under(rb_mT_Props, "Private"));

    GCABLE_VALUE(rb_mT_Props_Private_DeserializerGenerator,
                 rb_define_module_under(rb_mT_Props_Private, "DeserializerGenerator"));
    GCABLE_VALUE(rb_mT_Props_Private_SerializerGenerator,
                 rb_define_module_under(rb_mT_Props_Private, "SerializerGenerator"));

    GCABLE_VALUE(rb_mT_Props_Serializable, rb_define_module_under(rb_mT_Props, "Serializable"));
    GCABLE_VALUE(rb_mT_Props_Utils, rb_define_module_under(rb_mT_Props, "Utils"));

    GCABLE_VALUE(rb_mT_Types, rb_define_module_under(rb_mT, "Types"));
    GCABLE_VALUE(rb_cT_Types_Base, rb_define_class_under(rb_mT_Types, "Base", rb_cObject));
    GCABLE_VALUE(rb_cT_Types_Enum, rb_define_class_under(rb_mT_Types, "Enum", rb_cT_Types_Base));
    GCABLE_VALUE(rb_cT_Types_Intersection, rb_define_class_under(rb_mT_Types, "Intersection", rb_cT_Types_Base));
    GCABLE_VALUE(rb_cT_Types_Simple, rb_define_class_under(rb_mT_Types, "Simple", rb_cT_Types_Base));
    GCABLE_VALUE(rb_cT_Types_TypedEnumerable, rb_define_class_under(rb_mT_Types, "TypedEnumerable", rb_cT_Types_Base));
    GCABLE_VALUE(rb_cT_Types_TypedArray, rb_define_class_under(rb_mT_Types, "TypedArray", rb_cT_Types_TypedEnumerable));
    GCABLE_VALUE(rb_cT_Types_TypedHash, rb_define_class_under(rb_mT_Types, "TypedHash", rb_cT_Types_TypedEnumerable));
    GCABLE_VALUE(rb_cT_Types_TypedSet, rb_define_class_under(rb_mT_Types, "TypedSet", rb_cT_Types_TypedEnumerable));
    GCABLE_VALUE(rb_cT_Types_Union, rb_define_class_under(rb_mT_Types, "Union", rb_cT_Types_Base));

    GCABLE_VALUE(rb_mT_Utils, rb_define_module_under(rb_mT, "Utils"));
    GCABLE_VALUE(rb_mT_Utils_Nilable, rb_define_module_under(rb_mT_Utils, "Nilable"));

    // Add new methods specific to C-based serialization.

    extern VALUE T_Props_Private_Deserialize_generate2(VALUE self, VALUE klass, VALUE props, VALUE defaults);
    extern VALUE T_Props_Private_Serialize_generate2(VALUE self, VALUE klass, VALUE props);
    rb_define_singleton_method(rb_mT_Props_Private_DeserializerGenerator, "generate2",
                               T_Props_Private_Deserialize_generate2, 3);
    rb_define_singleton_method(rb_mT_Props_Private_SerializerGenerator, "generate2",
                               T_Props_Private_Serialize_generate2, 2);
}
