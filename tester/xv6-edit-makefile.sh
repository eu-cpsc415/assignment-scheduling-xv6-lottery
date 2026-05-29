#! /bin/bash

infile=$1
testnames=$2

gawk -vtestnames=$testnames '
# Match both legacy "_mkdir\\" and current "$U/_mkdir\\" lines.
($1 ~ /_mkdir\\$/) {
  n = split(testnames, x, ",");
  for (i = 1; i <= n; i++) {
    printf("\t$U/_%s\\\n", x[i]);
  }
} 

{
  print $0;
}' $infile 
