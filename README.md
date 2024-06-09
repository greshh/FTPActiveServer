# FTP Active Server
***Start:** (roughly) April 5<sup>th</sup> 2024, **Finish:** April 22<sup>nd</sup> 2024*

A group project with @Any_KHY for a third-year course: Operating Systems and Networks.

## Scenario
This is a simple Cross-platform, Active mode FTP server program written using socket API (preinstalled in the operating system: Windows/Linux/MacOS). The FTP server can process the following commands from the built-in FTP user agent: `USER`, `PASS`, `TYPE`, `LIST`, `EPRT`, `PORT`, `RETR` AND `QUIT`. 
It is RFC 959 protocol-, RFC 2428 protocol- and IPv6-compliant.
It uses data structures that work for IPv6 addresses (not just IPv4).
It can accept an optional ephemeral port to listen to, as one of its arguments - ie. a port number between [1024, 65535].
If the port number is not specified, it uses *port 1234* as default.
It can be used with the commands from the built-in FTP client on Windows 10 (IPv6) - eg. `dir`, `binary`, `ascii`, `get <filename>`.
For log-in, there is only authorised user entry - username: `napoleon`, password: `342`.
When the FTP server receives an `OPTS` command, it will simply return `550 unrecognized command`.
File transfer will work for any .jpg, .jpeg, .bmp, .gif, .tiff, and .png files.

This was edited from a start-up code from the lecturer: Dr Napoleon Reyes. This start-up code did not implement any of the above yet.

## Instructions:
Build the project as usual (`make`). There are various makefiles available suitable for Windows or MacOS. Simply copy-and-paste your desired code to `makefile` according to your OS.

You can either run the project with the default port number 1234:
```
./server
```
or, alternatively, with an optional ephemeral port, where `<port_number>` is the specified port number (eg. 1122):
```
./server <port_number>
```

You may use the .png and .gif files provided (as well as your own files) to exercise file transfer.
- black.jpg
- test1.gif
- test2.gif
- white.png

## What we learnt:
- Understand how an FTP server works as per the above, and how to simulate it using socket programming.
- Send information properly to a receiving client with error handling.
- Implement log-in protocol by accepting correct username _and_ password and so rejecting other invalid input.
