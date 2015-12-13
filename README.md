# RUSP

*Realiable User Segment Protocol*

- - -

RUSP is a Realiable User Segment Protocol library.
librusp comes with a little set of sampling client-server network applications,
that can be executed from /bin and whose source code can can be found in /samples.
Such network applications are meant to demonstrate the easy usage of the library,
allowing to evaluate its performances in common real scenarios
and giving some general guidelines for the design of simple network applications.
Moreover, the library provides an utility for sample file generation (samplegen),
thus making file transmission tests easy and immediate.

## Installation
The following command will compile librusp and all the sampling network applications
> make all

## Usage
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.
Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

### ECHO
The echo application realizes the well-known Echo Protocol.
> ./echos (-p port) (-d)
> ./echoc [address] (-p port) (-l loss) (-d)

### UPLOAD
The up application allows the client to upload a local file to server.
> ./ups (-p port) (-d)
> ./upc [address] [file] (-p port) (-l loss) (-d)

### LFTP
The Light File Transfer Protocol (FTP) is an application protocol that allows file
transmission and remote repository management.
The protocol is clarly inspired by the well-known File Transfer Protocol (FTP).
> ./lftps (-p port) (-r repo) (-d)
> ./lftpc [address] (-p port) (-r repo) (-l loss) (-d)

## Authors
Giacomo Marciani, [giacomo.marciani@gmail.com](mailto:giacomo.marciani@gmail.com)

## Contributing
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.
Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

## License
Assimply is released under the [MIT License](https://opensource.org/licenses/MIT).
Please, read the file LICENSE.md for details.
