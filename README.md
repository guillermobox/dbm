# DBM

Use a lightweight (and outdated) database from the terminal, scripting ready!

To compile in ubuntu you need `python-yaml`, `libgdbm-dev` and `libgdbm3`,
then do:

    make test

If you don't have python, just run `make` and take the binary. The other
libraries are required.

## How does this work?

The environment variable `DBM_PATH` is used to set the file that you want to
use as the database file. This file will be created if it doesn't exist yet.
Then, use the commands `get`, `set`, `list`, `remove` and `inc` to modify
the contents of the database. You can call the binary without commands to
see a little description of each one. For example:

    export DBM_PATH="/tmp/temporal.db"
    dbm set name Guille
    dbm get name

Carriage returns are respected, so if your 'set' command does not include a
carraige return (like in the example), the 'get' command will return the string
without one. This functionality is intented for scripting.

Using `inc` you can use the database to count for things, for example:

    dbm inc txt
    dbm inc pdf
    dbm inc txt

Now the key `txt` will have value 2, and `pdf` value 1. The keys are created
if they don't exist.
