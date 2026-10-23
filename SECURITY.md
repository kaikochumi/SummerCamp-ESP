# Security policy

## Scope

This project reads the memory of a running game from an external
process, on purpose. That behavior is the whole point of the tool and
is **not** a vulnerability — please don't report it as one.

A report is welcome if you find something else, for example:

- the tool writes to the game process (it should only read)
- a config-file or path-handling bug that lets another local program
  run code as the user
- the overlay accepts input from outside its own menu area in a way
  that harms the user

## How to report

Open a [GitHub issue](https://github.com/kaikochumi/SummerCamp-ESP/issues)
in this repository, or contact the owner privately through the account
profile. Include the release you use (the tag) and steps to reproduce.
Expect a reply within a week.

## Supported versions

| Version | Status    |
|---------|-----------|
| 0.1.x   | supported |
