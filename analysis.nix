let
  pkgs = import <nixpkgs> {};
  inherit (pkgs) lib;
  # A little helper to sanitize potential errors.
  force = v: (builtins.tryEval v).value;

  # Return docs for every attribute name in the set.
  getAttrDocs = set: lib.mapAttrs (n: v: builtins.unsafeGetAttrDoc n set) set;

  getLambdaDocs = set:
    lib.mapAttrs (n: v:
      let forced = force v;
      in
      if builtins.typeOf forced == "lambda"
      then builtins.unsafeGetLambdaDoc forced
      else null)
    set;

  libAttrDocs = lib.mapAttrs (n: v:
    if builtins.typeOf v == "set"
    then (getAttrDocs v)
    else builtins.unsafeGetAttrDoc n lib)
  lib;


  libLamdbdaDocs = lib.mapAttrs (n: v:
    let forced = force v;
    in
    if builtins.typeOf forced == "set"
    then (getLambdaDocs forced)
    else
     if builtins.typeOf forced == "lambda"
    then builtins.unsafeGetLambdaDoc forced
    else null)
    lib;

  libAttrsWithoutAttrDocs = lib.filterAttrsRecursive (n: v: v.position or null == null) libAttrDocs;
  lambdasWithoutSourcePosition = lib.filterAttrsRecursive (n: v: v.position or null == null) libLamdbdaDocs;

  libAttrsWithoutLambdaDocs = lib.filterAttrsRecursive (n: v: builtins.all (b: n != b) (builtins.attrNames builtins) && v.isPrimop or false == false ) lambdasWithoutSourcePosition;
in {
  inherit lambdasWithoutSourcePosition libLamdbdaDocs libAttrsWithoutLambdaDocs;unkown = libAttrsWithoutAttrDocs;libDocs = libAttrDocs;

  # Good news:
  # All important lib.functions have some attribute position tracking
  # lib.licenses is the only dynamically created set which doesn't allow attribute position tracking
  attrs = builtins.toFile "impossibleAttrs.json" (builtins.toJSON libAttrsWithoutAttrDocs);

  # lambdas without source position tracking are most likely only builtins. If they lack documentation it should be added to the nixos/nix repo.
  lambdas = builtins.toFile "impossibleLambdas.json" (builtins.toJSON libAttrsWithoutLambdaDocs);
}
