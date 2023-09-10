{
  single-line-comments-work-with-doccomments = builtins.unsafeGetAttrDoc "foo" {
    /**
    Docs
    */
    # TODO: fixme
    foo = x: x;
  };

  multiline-comments-clash-with-doccomments = builtins.unsafeGetAttrDoc "foo" {
    /**
    Some other docs
    */
    /* TODO: fixme*/
    foo = x: x;
  };

  ignore-single-line-comment = builtins.unsafeGetAttrDoc "foo" {
    # Not a documentation
    foo = x: x;
  };
  ignore-single-line-comments = builtins.unsafeGetAttrDoc "foo" {
    # Not
    # a
    # documentation
    foo = x: x;
  };
  ignore-multi-line-comments = builtins.unsafeGetAttrDoc "foo" {
    /* Not a documentation */
    foo = x: x;
  };
}
