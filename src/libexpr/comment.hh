#pragma once

#include "nixexpr.hh"

namespace nix::Comment {

struct Doc {
  std::string rawComment;
  std::string comment;

  // Number of times the curried function must be applied to get the value
  // that this structure documents.
  //
  // This is useful when showing the documentation for a partially applied
  // curried function. The documentation is for the unapplied function, so
  // this is crucial information.
  int timesApplied;

  Doc(std::string rawComment, std::string comment) {
    this->rawComment = rawComment;
    this->comment = comment;
  }
};

extern struct Doc emptyDoc;

// lookupDoc will try to recover a Doc. This will perform perform I/O,
// because documentation is not retained by the parser.
//
// Will return empty values if nothing can be found.
// For its limitations, see the docs of the implementation.
struct Doc lookupDoc(const Pos &pos);

} // namespace nix::Comment
