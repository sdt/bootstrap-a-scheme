use Test::Bas;

my @ids = (
    '123', '"one two three"', 'true', 'false', '()',
);
for my $s (@ids) {
    bas_is($s, $s);
}

done_testing();
