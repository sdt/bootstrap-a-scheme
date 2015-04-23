package Test::Bas;

use 5.20.0;
use warnings;

use parent qw( Exporter );

use Function::Parameters qw( :strict );
use Import::Into;
use Path::Class qw( );
use Test::FailWarnings;
use Test::Most;

our @EXPORT = qw( bas_is bas_like bas_run );

sub import {
    shift->export_to_level(1);

    Test::Most->import::into(1);
}

fun bas_run($input) {
    my $tmpdir = Path::Class::Dir->new('/tmp');
    my $infile = $tmpdir->file("bas-in.$$");
    my $outfile = $tmpdir->file("bas-out.$$");

    $infile->spew($input);
    system("./bas < $infile > $outfile 2> /dev/null");

    my $output = $outfile->slurp();
    chomp $output;

    $infile->remove();
    $outfile->remove();

    return $output;
}

fun bas_is($input, $expected, $message = "$input ==> \"$expected\"") {
    local $Test::Builder::Level = $Test::Builder::Level + 1;
    my $got = bas_run($input);
    $message =~ s/\n/\\n/gm;
    is($got, $expected, $message);
}

1;
