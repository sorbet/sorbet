#ifndef RUBY_PARSER_CAPI_HH
#define RUBY_PARSER_CAPI_HH

#include "builder.hh"
#include "driver.hh"
#include "node.hh"
#include "token.hh"

extern "C" {

struct cdiagnostic {
    ruby_parser::dlevel level;
    ruby_parser::dclass type;
    const char *data;
    size_t beginPos;
    size_t endPos;
};

ruby_parser::typedruby27 *rbdriver_typedruby27_new(const char *source, size_t source_length,
                                                   const ruby_parser::builder *builder);

void rbdriver_typedruby27_free(ruby_parser::typedruby27 *parser);

const void *rbdriver_parse(ruby_parser::base_driver *parser, ruby_parser::SelfPtr self, bool trace);

bool rbdriver_in_definition(const ruby_parser::base_driver *driver);

bool rbdriver_env_is_declared(const ruby_parser::base_driver *p, const char *name, size_t length);

void rbdriver_env_declare(ruby_parser::base_driver *p, const char *name, size_t length);

size_t rbtoken_get_start(const ruby_parser::token *tok);

size_t rbtoken_get_end(const ruby_parser::token *tok);

size_t rbtoken_get_string(const ruby_parser::token *tok, const char **out_ptr);

size_t rblist_get_length(const ruby_parser::node_list *list);

const void *rblist_index(ruby_parser::node_list *list, size_t index);

size_t rbdriver_diag_get_length(const ruby_parser::base_driver *parser);

void rbdriver_diag_get(const ruby_parser::base_driver *parser, size_t index, struct cdiagnostic *diag);

void rbdriver_diag_report(ruby_parser::base_driver *driver, const struct cdiagnostic *diag);
}

#endif
