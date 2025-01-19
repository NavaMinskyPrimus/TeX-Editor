This is a simple test format that lets you write tests of
command line arguments in a surprisingly simple way! It's
based on a tool called "cram"! (Which is already installed
on your machine.)

Here's how you write a test: two spaces, and then a dollar-sign
to indicate a shall command. Like...

  $ echo foo bar
  foo bar

That's all it is! Now, go to the command line and run "cram foo.t"!

Here's another fun example. First, I'm going to list the current directory.a

  $ ls

Which actually will come up empty, because it starts running in a temporary
directory that's initially empty.

But I can create some files.

  $ touch foo.txt
  $ touch bar.txt
  $ ls
  bar.txt
  foo.txt

Now, let's create an file with some things, just to try it
out and have some inputs. We're going to use a trick in bash
for creating files with multi-line content.

  $ cat << 'EOF' > f1.txt
  > This is some text
  > and I'll terminate it with an EOF!
  > EOF

Now, I can dump the contents of this file:

  $ cat f1.txt
  This is some text
  and I'll terminate it with an EOF!

Now, here's how we setup so we can actually run tests on our program.

We need to run setup.sh to get the executable in place.

  $ . $TESTDIR/setup.sh

Now, let's run AnyTest!
