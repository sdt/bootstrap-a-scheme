use Test::Bas;

note 'define';
bas_is("(define x 1)\nx",                           "1\n1");
bas_is("(define x 1)\n(define y x)\ny",             "1\n1\n1");
bas_is("(define x 1)\n(define y (+ x 2))\ny",       "1\n3\n3");
bas_is("(define x 1)\n(define y (+ x 2))\ny\nx",    "1\n3\n3\n1");

bas_is('(define)',          'define: 2 args expected, 0 provided');
bas_is('(define x)',        'define: 2 args expected, 1 provided');
bas_is('(define x 1 2)',    'define: 2 args expected, 3 provided');

bas_is('(define "x" 1)',    'define: arg 1 is string, expected symbol');
bas_is('(define x x)',      '"x" not found');

note 'if';
bas_is('(if true  1 2)',    1);
bas_is('(if false 1 2)',    2);
bas_is('(if true  1)',      1);

bas_is('(if 3 1 2)',            1);
bas_is('(if "a" 1 2)',          1);
bas_is('(if (cons 1 2) 1 2)',   1);

bas_is('(if (< -1 1) 1 2)',   1);
bas_is('(if (< 1 -1) 1 2)',   2);

bas_is('(if)',              'if: at least 2 args expected, 0 provided');
bas_is('(if true)',         'if: at least 2 args expected, 1 provided');
bas_is('(if true 1 2 3)',   'if: no more than 3 args expected, 4 provided');

note 'TODO: check these are what we want';
bas_is('(if false 1)',    '()');
bas_is('(if ()   1 2)',    1);
bas_is('(if nil  1 2)',    1);

note 'lambda';
bas_like('(lambda a a)',              qr/^lambda:a->/);
bas_like('(lambda (a) a)',            qr/^lambda:\(a\)->/);
bas_like('(lambda () 2)',             qr/^lambda:\(\)->/);
bas_like('(lambda (a) (+ a a))',      qr/^lambda:\(a\)->/);
bas_like('(lambda (a b) (+ a b))',    qr/^lambda:\(a b\)->/);

bas_is('(lambda)',              'lambda: 2 args expected, 0 provided');
bas_is('(lambda a)',            'lambda: 2 args expected, 1 provided');
bas_is('(lambda (a b))',        'lambda: 2 args expected, 1 provided');
bas_is('(lambda a b c)',        'lambda: 2 args expected, 3 provided');
bas_is('(lambda (a b) c d)',    'lambda: 2 args expected, 3 provided');

note 'calling lambdas';
bas_is('((lambda a a) 1 2 3)',          '(1 2 3)');
bas_is('((lambda (a) a) 1)',            1);
bas_is('((lambda (a b) (+ a b)) 1 2)',  3);
bas_is('((lambda () 2))',               2);
my $fac = '(define fac (lambda (n) (if (= n 0) 1 (* n (fac (+ n -1))))))';
bas_ends("$fac\n(fac 6)", 720);

bas_is('((lambda () 1) 1)',         'lambda: 0 args expected, 1 provided');
bas_is('((lambda () 1) 1 2)',       'lambda: 0 args expected, 2 provided');
bas_is('((lambda (b) a))',          'lambda: 1 arg expected, 0 provided');
bas_is('((lambda (a) a) 1 2 3)',    'lambda: 1 arg expected, 3 provided');
bas_is('((lambda (a b) a) 1 2 3)',  'lambda: 2 args expected, 3 provided');
bas_is('((lambda (a b) a))',        'lambda: 2 args expected, 0 provided');
bas_is('((lambda (a b) a) 1)',      'lambda: 2 args expected, 1 provided');

done_testing();
