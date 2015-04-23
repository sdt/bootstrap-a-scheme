use Test::Bas;

bas_is('(+)',       0);
bas_is('(+ 1)',     1);
bas_is('(+ 1 2)',   3);
bas_is('(+ 1 2 3)', 6);
bas_is('(+ "a")',   '+: arg is string, expected integer');
bas_is('(+ 1 "a")', '+: arg is string, expected integer');

bas_is('(*)',        1);
bas_is('(* 2)',      2);
bas_is('(* 2 3)',    6);
bas_is('(* 2 3 4)', 24);
bas_is('(* "a")',   '*: arg is string, expected integer');
bas_is('(* 2 "a")', '*: arg is string, expected integer');

bas_is('(< 1 2)',    'true');
bas_is('(< 2 1)',    'false');
bas_is('(< 1 1)',    'false');

bas_is('(<)',       '<: 2 args expected, 0 provided');
bas_is('(< 1)',     '<: 2 args expected, 1 provided');
bas_is('(< 1 2 3)', '<: 2 args expected, 3 provided');
bas_is('(< "a" 2 3)', '<: 2 args expected, 3 provided');

bas_is('(< "a" 1)', '<: arg 1 is string, expected integer');
bas_is('(< 2 "b")', '<: arg 2 is string, expected integer');

bas_is('(cons 1 2)',            '(1 . 2)');
bas_is('(cons 1 nil)',          '(1)');
bas_is('(cons 1 (cons 2 nil))', '(1 2)');
bas_is('(cons nil nil)',        '(())');
bas_is('(cons)',                'cons: 2 args expected, 0 provided');
bas_is('(cons 1)',              'cons: 2 args expected, 1 provided');
bas_is('(cons 1 2 3)',          'cons: 2 args expected, 3 provided');

bas_is('(car (cons 1 2))',      1);
bas_is('(car)',                 'car: 1 arg expected, 0 provided');
bas_is('(car (cons 1 2) 3)',    'car: 1 arg expected, 2 provided');
bas_is('(car 1)',               'car: arg is integer, expected pair');

bas_is('(cdr (cons 1 2))',      2);
bas_is('(cdr)',                 'cdr: 1 arg expected, 0 provided');
bas_is('(cdr (cons 1 2) 3)',    'cdr: 1 arg expected, 2 provided');
bas_is('(cdr 1)',               'cdr: arg is integer, expected pair');

bas_is('(empty? nil)',          'true');
bas_is('(empty? ())',           'true');
bas_is('(empty? (cons 1 nil))', 'false');
bas_is('(empty? 1)',            'false');
bas_is('(empty? "a")',          'false');
bas_is('(empty? empty?)',       'false');

bas_is('(empty?)',              'empty?: 1 arg expected, 0 provided');
bas_is('(empty? 1 2)',          'empty?: 1 arg expected, 2 provided');

done_testing();
