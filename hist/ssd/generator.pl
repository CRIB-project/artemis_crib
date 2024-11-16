#!/usr/bin/perl

for (0..15) {
    $n=$_+1;
    print "  x$n: \@dssd\@X_raw.fID == $_;\n";
}

for (0..15) {
    $n=$_+1;
    print "  y$n: \@dssd\@Y_raw.fID == $_;\n";
}


for (0..15) {
    $n=$_+1;

    print "
     - name: EX$n
       title: \@dssd\@X_raw ch$n E
       x: \*ex_raw
       cut: x$n
";


}

for (0..15) {
    $n=$_+1;

    print "
     - name: TX$n
       title: \@dssd\@X_raw ch$n T
       x: \*t_raw
       cut: x$n
";


}

for (0..15) {
    $n=$_+1;

    print "
     - name: EY$n
       title: \@dssd\@Y_raw ch$n E
       x: \*ey_raw
       cut: y$n
";


}






