
#include "source.hxx"

void diagnostic_reporter::report(source_diagnostic diag) { messages_.push_back(diag); }

diagnostic_reporter::messages_t const& diagnostic_reporter::messages() const { return messages_; }
