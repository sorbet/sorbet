#include <ruby_parser/capi.hh>
#include <cstdio>

ruby_parser::typedruby25*
rbdriver_typedruby25_new(const char* source_ptr, size_t source_length, const ruby_parser::builder* builder)
{
	std::string source { source_ptr, source_length };
	return new ruby_parser::typedruby25(source, *builder);
}

void
rbdriver_typedruby25_free(ruby_parser::typedruby25* driver)
{
	delete driver;
}

const void*
rbdriver_parse(ruby_parser::base_driver* driver, ruby_parser::self_ptr self)
{
	return driver->parse(self);
}

bool
rbdriver_in_definition(const ruby_parser::base_driver *driver)
{
	return driver->def_level > 0;
}

bool
rbdriver_env_is_declared(const ruby_parser::base_driver *driver, const char* name, size_t length)
{
	std::string id { name, length };
	return driver->lex.is_declared(id);
}

void
rbdriver_env_declare(ruby_parser::base_driver *driver, const char* name, size_t length)
{
  std::string id { name, length };
  driver->lex.declare(id);
}

size_t
rbtoken_get_start(const ruby_parser::token* tok)
{
	return tok->start();
}

size_t
rbtoken_get_end(const ruby_parser::token* tok)
{
	return tok->end();
}

size_t
rbtoken_get_string(const ruby_parser::token* tok, const char** out_ptr)
{
	*out_ptr = tok->string().data();
	return tok->string().size();
}

size_t
rblist_get_length(const ruby_parser::node_list* list)
{
	return list->size();
}

const void*
rblist_index(ruby_parser::node_list* list, size_t index)
{
	return list->at(index);
}

size_t
rbdriver_diag_get_length(const ruby_parser::base_driver* driver)
{
	return driver->diagnostics.size();
}

void
rbdriver_diag_get(const ruby_parser::base_driver* driver, size_t index, struct cdiagnostic *diag)
{
	auto &cppdiag = driver->diagnostics.at(index);
	diag->level = cppdiag.level();
	diag->type = cppdiag.error_class();
	diag->data = cppdiag.data().c_str();
	diag->beginPos = cppdiag.location().beginPos;
	diag->endPos = cppdiag.location().endPos;
}

void
rbdriver_diag_report(ruby_parser::base_driver* driver, const struct cdiagnostic *diag)
{
	driver->external_diagnostic(
		diag->level,
		diag->type,
		diag->beginPos,
		diag->endPos,
		diag->data ? std::string(diag->data) : ""
	);
}
