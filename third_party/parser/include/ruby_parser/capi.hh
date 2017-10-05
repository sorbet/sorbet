#ifndef RUBY_PARSER_CAPI_HH
#define RUBY_PARSER_CAPI_HH

#include "token.hh"
#include "node.hh"
#include "builder.hh"
#include "driver.hh"

extern "C" {

struct cdiagnostic {
	ruby_parser::dlevel level;
	ruby_parser::dclass type;
	const char *data;
	size_t begin_pos;
	size_t end_pos;
};

ruby_parser::typedruby24*
rbdriver_typedruby24_new(const char* source, size_t source_length, const ruby_parser::builder* builder);

void
rbdriver_typedruby24_free(ruby_parser::typedruby24* parser);

const void*
rbdriver_parse(ruby_parser::base_driver* parser, ruby_parser::self_ptr self);

bool
rbdriver_in_definition(const ruby_parser::base_driver *driver);

bool
rbdriver_env_is_declared(const ruby_parser::base_driver *p, const char* name, size_t length);

void
rbdriver_env_declare(ruby_parser::base_driver *p, const char* name, size_t length);

size_t
rbtoken_get_start(const ruby_parser::token* tok);

size_t
rbtoken_get_end(const ruby_parser::token* tok);

size_t
rbtoken_get_string(const ruby_parser::token* tok, const char** out_ptr);

size_t
rblist_get_length(const ruby_parser::node_list* list);

const void*
rblist_index(ruby_parser::node_list* list, size_t index);

size_t
rbdriver_diag_get_length(const ruby_parser::base_driver* parser);

void
rbdriver_diag_get(const ruby_parser::base_driver* parser, size_t index, struct cdiagnostic *diag);

void
rbdriver_diag_report(ruby_parser::base_driver* driver, const struct cdiagnostic *diag);

}

#endif
