use Test::Bas;

my @ids = (
    '(vector)', '(vector 1)', '(vector 1 2)', '(vector 1 2 3)',
    '(vector (vector 1) 2 (vector 3 4))',
);
for my $id (@ids) {
    bas_is($id, $id);
}

done_testing();
