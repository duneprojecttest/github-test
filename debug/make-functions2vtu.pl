#!/usr/bin/perl

use strict;
use warnings;
use Getopt::Long;
use Data::Dumper;

#######################################################################
#
# Help text
#
my $help = <<EOF;
make-localfunctions2vtu -- compile a VTK output executable for a specific
                           local basis

SYNOPSIS:
    make-localfunction2vtu --help|-h|-?
    make-localfunction2vtu --basis-help|-H LOCAL-BASIS
    make-localfunction2vtu --list
    make-localfunction2vtu --source LOCAL-BASIS
    make-localfunction2vtu LOCAL-BASIS

PARAMETERS:
    --basis-help LOCAL-BASIS
        Give help for the given LOCAL-BASIS.
    --help|-h|-?
        Print this help.
    --list
        List available local basis templates.
    --source LOCAL-BASIS
        Generate source for the given LOCAL-BASIS.  The header is written to a
        file named after the program name with ".cc" appended.
    LOCAL-BASIS
        Call the makefile with the apropriate parameters to build the program
        for this basis.
EOF
    ;
#######################################################################
#
# List of supported bases
#

#hash with key shortname -- full name of the base excluding template params
#each value is a hash with keys
# tparams  -- descriptive string of template params, if any
# help     -- help text
# headers  -- string holding required headers
# progname -- subroutine taking a list of template params and returning a
#             suffix for the resulting program
my %bases = (

#======================================================================
'Dune::P0LocalBasis' => {
tparams  => '<typename DomainFieldType, typename RangeFieldType, int dimension>',

help     => <<EOH,
Defines the constant scalar shape function in d dimensions.  Is valid on any
type of reference element.

Template Parameters:
 * DomainFieldType: Type to represent the field in the domain.
 * RangeFieldType: Type to represent the field in the range.
 * dimension: Domain dimension.
EOH

headers  => <<EOH,
#include <dune/finiteelements/p0/p0localbasis.hh>
EOH

progname => sub {
    my $D = shift;
    my $R = shift;
    my $d = shift;
    return sprintf "p0-%s-%s-%dd", lc $D, lc $R, $d;
}},

#======================================================================
'Dune::P12DLocalBasis' => {
tparams  => '<typename DomainFieldType, typename RangeFieldType>',

help     => <<EOH,
Linear Lagrange shape functions on the triangle.

Template Parameters:
 * DomainFieldType: Type to represent the field in the domain.
 * RangeFieldType:  Type to represent the field in the range. 
EOH

headers  => <<EOH,
#include <dune/finiteelements/p12d/p12dlocalbasis.hh>
EOH

progname => sub {
    my $D = shift;
    my $R = shift;
    return sprintf "p12d-%s-%s", lc $D, lc $R;
}},

#======================================================================
'Dune::Pk2DLocalBasis' => {
tparams  => '<typename DomainFieldType, typename RangeFieldType, int order>',

help     => <<EOH,
Lagrange shape functions of arbitrary order on the reference triangle.

Template Parameters:
 * DomainFieldType: Type to represent the field in the domain.
 * RangeFieldType:  Type to represent the field in the range.
 * order:           Polynomial order.
EOH

headers  => <<EOH,
#include <dune/finiteelements/pk2d/pk2dlocalbasis.hh>
EOH

progname => sub {
    my $D = shift;
    my $R = shift;
    my $o = shift;
    return sprintf "pk2d-%s-%s-o%d", lc $D, lc $R, $o;
}},

#======================================================================
'Dune::Q12DLocalBasis' => {
tparams  => '<typename DomainFieldType, typename RangeFieldType>',

help     => <<EOH,
Lagrange shape functions of order 1 on the reference quadrilateral.

Template Parameters:
 * DomainFieldType: Type to represent the field in the domain.
 * RangeFieldType:  Type to represent the field in the range. 
EOH

headers  => <<EOH,
#include <dune/finiteelements/q12d/q12dlocalbasis.hh>
EOH

progname => sub {
    my $D = shift;
    my $R = shift;
    return sprintf "q12d-%s-%s", lc $D, lc $R;
}},

#======================================================================
'Dune::Q22DLocalBasis' => {
tparams  => '<typename DomainFieldType, typename RangeFieldType>',

help     => <<EOH,
Lagrange shape functions of order 2 on the reference quadrilateral.

Template Parameters:
 * DomainFieldType: Type to represent the field in the domain.
 * RangeFieldType:  Type to represent the field in the range. 
EOH

headers  => <<EOH,
#include <dune/finiteelements/q22d/q22dlocalbasis.hh>
EOH

progname => sub {
    my $D = shift;
    my $R = shift;
    return sprintf "q22d-%s-%s", lc $D, lc $R;
}},

#======================================================================
'Dune::RT02DLocalBasis' => {
tparams  => '<typename DomainFieldType, typename RangeFieldType>',

help     => <<EOH,
Lowest order Raviart-Thomas shape functions on the reference triangle.

Template Parameters:
 * DomainFieldType: Type to represent the field in the domain.
 * RangeFieldType:  Type to represent the field in the range. 
EOH

headers  => <<EOH,
#include <dune/finiteelements/rt02d/rt02dlocalbasis.hh>
EOH

progname => sub {
    my $D = shift;
    my $R = shift;
    return sprintf "rt02d-%s-%s", lc $D, lc $R;
}},

#======================================================================
'Dune::EdgeR12DLocalBasis' => {
tparams  => '<typename DomainFieldType, typename RangeFieldType>',

help     => <<EOH,
Edge shape functions of order 1 on the reference rectangle.

Template Parameters:
 * DomainFieldType: Type to represent the field in the domain.
 * RangeFieldType:  Type to represent the field in the range. 
EOH

headers  => <<EOH,
#include <dune/finiteelements/edger12d/edger12dlocalbasis.hh>
EOH

progname => sub {
    my $D = shift;
    my $R = shift;
    return sprintf "edger12d-%s-%s", lc $D, lc $R;
}},

#======================================================================
'Dune::MonomLocalBasis' => {
tparams  => '<typename DomainFieldType, typename RangeFieldType, int dimDomain, int porder>',

help     => <<EOH,
Monom shape functions of order 1.

Template Parameters:
 * DomainFieldType: Type to represent the field in the domain.
 * RangeFieldType:  Type to represent the field in the range. 
 * dimDomain:       Dimension of the domain.
 * porder:          Polynomial order.
EOH

headers  => <<EOH,
#include <dune/finiteelements/monom/monomlocalbasis.hh>
EOH

progname => sub {
    my $D = shift;
    my $R = shift;
    my $d = shift;
    my $k = shift;
    return sprintf "monomp%d%dD-%s-%s", $k, $d, lc $D, lc $R;
}},

#======================================================================
);

sub decode_basis($ ) {
    my $basis = $_[0];
    $basis =~ s/[[:space:]]+/ /g;
    $basis =~ s/^ //;
    $basis =~ s/ $//;

    $basis =~ s/^([[:alnum:]:_]+)[[:space:]]*//;
    my $name = $1;
    $name =~ s/ //g;
    my @result = ($name,);
    if($basis =~ s/^<(.*)>$/$1/) {
        push @result, map { s/^ //; s/ $//; $_ } split m/,/, $basis;
    }
    elsif($basis ne "") {
        die "Invalid basis specification \"$_[0]\"\n";
    }
    if(wantarray) { return @result }
    else          { return $result[0] }
}

sub find_basis($ ) {
    my $name = shift;
    if(defined $bases{$name}) {
        return $name;
    }
    if(defined $bases{"Dune::$name"}) {
        return "Dune::$name";
    }
    die "Can't find basis $name\n";
}

sub basis_help($ ) {
    my $name = shift;
    my $help = $name;
    $help .= $bases{$name}{tparams}
        if defined $bases{$name}{tparams};
    $help .= "\n\n";
    $help .= $bases{$name}{help}
        if defined $bases{$name}{help};
    return $help;
}

sub list_bases() {
    my @bases;
    for(sort keys %bases) {
        if(defined $bases{$_}{tparams}) {
            push @bases,  $_.$bases{$_}{tparams};
        } else {
            push @bases, $_;
        }
    }
    if(wantarray) { return @bases }
    else {
        return join "", map { "$_\n" } @bases;
    } 
}

sub gen_source($ ) {
    my($basis, @tparams) = decode_basis shift;
    $basis = find_basis $basis;
    my $fullname = $basis;
    if(@tparams) {
        $fullname .= "<".join(", ", @tparams).">";
    }

    my $progname = "functions2vtu-".$bases{$basis}{progname}(@tparams);
    my $sourcename = "$progname.cc";

    open(SOURCE, '>', $sourcename) or die "Can't open $sourcename for writing\n";

    print SOURCE <<EOF;
#ifdef HAVE_CONFIG_H
# include "config.h"     
#endif

EOF
    print SOURCE $bases{$basis}{headers}
        if defined $bases{$basis}{headers};
    print SOURCE <<EOF;

#define LOCAL_BASIS_TYPE $fullname
#define LOCAL_BASIS_TYPE_S "$fullname"
#define PROG_NAME "$progname"

#include "localfunctions2vtu.hh"
EOF

    close SOURCE;
}

#######################################################################
#
#  Parse Options
#

my $result = GetOptions(
    "help|l|?" => sub {
        print $help;
        exit 0;
    },
    "basis-help|H=s" => sub {
        print basis_help find_basis $_[1];
        exit 0;
    },
    "list" => sub {
        print scalar list_bases;
        exit 0;
    },
    "source=s" => sub {
        gen_source $_[1];
        exit 0;
    },
    );
if(not $result) { exit 1 }

if(@ARGV != 1) {
    die "Need a basis to generate the Program for.  Try --help.\n";
}

my($basis, @tparams) = decode_basis shift;
$basis = find_basis $basis;
my $fullname = $basis;
if(@tparams) {
    $fullname .= "<".join(", ", @tparams).">";
}

my $progname = "functions2vtu-".$bases{$basis}{progname}(@tparams);
print "Generating $progname\n";

my $make = $ENV{MAKE};
if(not defined $make or $make eq "") { $make = "make" }
my $command = <<EOC;
test -e .deps/$progname.Po || echo '# dummy' >.deps/$progname.Po
$make BASIS='$fullname' PROG_NAME=$progname $progname
EOC

exec "sh", "-vc", $command;
die "could not execute sh\n"
