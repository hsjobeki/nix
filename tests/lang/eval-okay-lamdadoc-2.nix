builtins.unsafeGetLambdaDoc {
  /**
  This is a common place in nixpkgs.lib
  But it doesn't work with the new standard
  */
  id = builtins.id or /**The lib function id*/ (x: x);
}.id
