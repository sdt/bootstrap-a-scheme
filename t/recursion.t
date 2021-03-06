use Test::Bas;

note 'factorial'; {
    my $fac = '(define fac (lambda (n) (if (< n 1) 1 (* n (fac (- n 1))))))';
    my $expected = 1;
    for (1 .. 10) {
        $expected *= $_;
        bas_ends("$fac\n(fac $_)", $expected);
    }
}

done_testing();
