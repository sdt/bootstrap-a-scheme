use Test::Bas;

my $v  = 'vector'; # make these tests a bit less verbose
my $vg = 'vector-get';

note 'Constructor'; {
    my @ids = (
        "($v)", "($v 1)", "($v 1 2)", "($v 1 2 3)", "($v ($v 1) 2 ($v 3 4))",
    );
    for my $id (@ids) {
        bas_is($id, $id);
    }

    bas_is("($v (+ 1 2) ($v (+ 3 4) (+ 5 6)))", "($v 3 ($v 7 11))");
}

note 'Good lookups'; {
    my $vec = "($v 1 2 ($v 3 4) 5 ($v ($v 6) 7))";

    bas_is("($vg $vec 0)",                  1);
    bas_is("($vg $vec 1)",                  2);
    bas_is("($vg $vec 2)",                  "($v 3 4)");
    bas_is("($vg $vec 3)",                  5);
    bas_is("($vg $vec 4)",                  "($v ($v 6) 7)");
    bas_is("($vg ($vg $vec 2) 0)",          3);
    bas_is("($vg ($vg $vec 2) 1)",          4);
    bas_is("($vg ($vg $vec 4) 0)",          "($v 6)");
    bas_is("($vg ($vg $vec 4) 1)",          7);
    bas_is("($vg ($vg ($vg $vec 4) 0) 0)",  6);
}

note 'Bad lookups'; {
    my $vec = "($v 1 2 3)";

    bas_is("($vg $vec -2)", "Index -2 out of range [0..3)");
    bas_is("($vg $vec -1)", "Index -1 out of range [0..3)");
    bas_is("($vg $vec 3)",  "Index 3 out of range [0..3)");
    bas_is("($vg $vec 4)",  "Index 4 out of range [0..3)");

    bas_is("($vg ($v) 0)", "Index 0 out of range [0..0)");
}

done_testing();
