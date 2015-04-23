use Test::Bas;

bas_is("(define x 1)\nx",                           "1\n1");
bas_is("(define x 1)\n(define y x)\ny",             "1\n1\n1");
bas_is("(define x 1)\n(define y (+ x 2))\ny",       "1\n3\n3");
bas_is("(define x 1)\n(define y (+ x 2))\ny\nx",    "1\n3\n3\n1");

bas_is('(define)',          'define: 2 args expected, 0 provided');
bas_is('(define x)',        'define: 2 args expected, 1 provided');
bas_is('(define x 1 2)',    'define: 2 args expected, 3 provided');

bas_is('(define "x" 1)',    'define: arg 1 is string, expected symbol');
bas_is('(define x x)',      '"x" not found');

done_testing();
