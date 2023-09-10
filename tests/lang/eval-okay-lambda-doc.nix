{
  simple =
    builtins.unsafeGetLambdaDoc
    {
      /**
      Docs
      */
      foo = x: x;
    }.foo;

  propagated =
    builtins.unsafeGetLambdaDoc
    rec {
      /**
      Docs
      */
      foo = x: x;

      bar = foo;
    }.bar;
  partially-applied =
    builtins.unsafeGetLambdaDoc
    rec {
      /**
      Docs
      */
      add = a: b:  a+b;

      addOne = add 1;
    }.addOne;

}
