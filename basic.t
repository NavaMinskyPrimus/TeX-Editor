Let's do a simple test. First, set things up.

  $ . $TESTDIR/setup.sh

Run the basic tests which you get if you pass no arguments

  $ run

Let's test on a simple file.

  $ cat << 'EOF' > foo.txt
  > foo\def{b}{}bar\b{abc}snoo
  > EOF
  $ run foo.txt
  foobarsnoo
  


Echo test

  $ echo 1 2 3
  1 2 3
