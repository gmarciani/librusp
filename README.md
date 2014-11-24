# RUSP

## What is it?

RUSP is a Realiable User Segment Protocol library.

## Author

@Author:    Giacomo Marciani

@Website:   http://gmarciani.com

@Email:     giacomo.marciani@gmail.com

## Documentation

The documentation about RUSP protocol, librusp and all sample network applications is available in folder docs/.

## How to compile

The following command will compile librusp and all the sampling network applications

`make all`

## How to clean

`make clean`

## Sample network applications

librusp comes with a little set of sampling client-server network applications, 
that can be executed from /bin and whose source code can can be found in /samples. 
Such network applications are meant to demonstrate the easy usage of the library, 
allowing to evaluate its performances in common real scenarios 
and giving some general guidelines for the design of simple network applications. 
Moreover, the library provides an utility for sample file generation (samplegen), 
thus making file transmission tests easy and immediate.

### ECHO

The echo application realizes the well-known Echo Protocol.

`./echos (-p port) (-d)`

`./echoc [address] (-p port) (-l loss) (-d)`

### UPLOAD

The up application allows the client to upload a local file to server.

`./ups (-p port) (-d)`

`./upc [address] [file] (-p port) (-l loss) (-d)`

### LFTP

The Light File Transfer Protocol (FTP) is an application protocol that allows file 
transmission and remote repository management. 
The protocol is clarly inspired by the well-known File Transfer Protocol (FTP).

`./lftps (-p port) (-r repo) (-d)`

`./lftpc [address] (-p port) (-r repo) (-l loss) (-d)`


## Report a bug

Please, report any bug to giacomo.marciani@gmail.com.

## License

The software is under MIT License (MIT).

Please, see the file named *LICENSE* for details.
