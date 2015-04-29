use Test::Bas;

note 'factorial'; {
    my $cons = '(define CONS (lambda (a b) (lambda (1st?) (if 1st? a b))))';
    my $car  = '(define CAR (lambda (x) (x true)))';
    my $cdr  = '(define CDR (lambda (x) (x false)))';
    my $list = '(define 1-5 (CONS 1 (CONS 2 (CONS 3 (CONS 4 (CONS 5 nil))))))';
    my $code = "$cons\n$car\n$cdr\n$list";

    bas_like("$code\n(CAR 1-5)", qr/\n1$/);
    bas_like("$code\n(CAR (CDR 1-5))", qr/\n2$/);
    bas_like("$code\n(CAR (CDR (CDR 1-5)))", qr/\n3$/);
    bas_like("$code\n(CAR (CDR (CDR (CDR 1-5))))", qr/\n4$/);
    bas_like("$code\n(CAR (CDR (CDR (CDR (CDR 1-5)))))", qr/\n5$/);
}

done_testing();
