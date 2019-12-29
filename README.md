Introduction
--------------------------------------------------------------------------
treezy is a "Swiss army knife" program for working with directory and
file trees. This is still a work in progress.

For now, treezy simply outputs machine-readable per-entry information,
tallies up the number of files, directories, and symbolic links under a
directory tree, and outputs those totals.

Usage
--------------------------------------------------------------------------
```
Usage: treezy
```

In its current state, the software scans the current working directory
and its subtrees. There are no options.

Compatibility warning
--------------------------------------------------------------------------
DO NOT RELY ON THIS PROGRAM'S OUTPUT FORMAT IN SCRIPTS!

This program is currently in very early development and only produces
machine-readable output to demonstrate functionality of the underlying
code. This output is guaranteed to change in the future, especially
because the program will be changed to output user-friendly messages and
output information by default.

If you want to use the program in scripts immediately, it is a good idea
to write in a version check. Use `treezy -v` to get the program version
information. The version number will change when output format changes.

Example script code to get the version number into $TREEZYVER:
`TREEZYVER=$(treezy -v | grep version | sed 's/[^0-9]*//;s/ .*//')`

Contact information
--------------------------------------------------------------------------
For all inquiries, contact Jody Bruchon <jody@jodybruchon.com>


Legal information and software license
--------------------------------------------------------------------------
treezy is Copyright (C) 2015-2019 by Jody Bruchon <jody@jodybruchon.com>

The MIT License

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

