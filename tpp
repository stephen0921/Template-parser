#!/usr/bin/perl -w
use Template;
use Getopt::Long;
use Cwd;
use File::Basename;
use Symbol;
use Carp;

$help = 0;
$tpl = "-";
$out_file = "-";

my $cmdok = GetOptions (
    "tpl=s" => \$tpl,
    "out=s" => \$out_file,
    "help" => \$help
);

if ($tpl eq "-" || $out_file eq "-" || $help) {
    help();
}

$dir = dirname($tpl);
$tpl = basename($tpl);

my $out_fh = gensym;
open($out_fh, ">$out_file") or croak "Can not open file $out_file!: $!";
my @out;

pipe(READER, WRITER) or croak "pipe no good: $!";

my $pid = fork();
croak "Can not fork: $!" unless defined $pid;

if($pid) {
    #parent
    close WRITER;
    while(<READER>) {
        push @out, $_;
    }
    close READER;
}
else {
    #child
    close READER;
    open STDOUT, ">&WRITER";

    my $tt = Template->new({
        INCLUDE_PATH => "$dir",
        INTERPOLATE  => 1,
        OUTLINE_TAG => ';'
    }) || croak "$Template::ERROR\n";

    $tt->process("$tpl")
        || croak $tt->error(), "\n";

    close WRITER;

    exit 0;
}

foreach (@out) {
    print $out_fh  "$_";
}
close($out_fh);

sub help {
    print <<HelpText;
    Tool for Template Processing 

Options:
    -tpl      : template filename
    -out      : output filename
    -help     : print this

Example:
    tpp -tpl demo_tpp.tt -out out.txt
HelpText
    exit 0;

}
