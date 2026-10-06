# IE3090 RemoteOps Structured Reflection

## Student
Registration Number: IT24104117

## 1. What I Learned

Through this assignment, I gained practical experience in developing a client-server application in C using the BSD socket API.

I learned how TCP sockets are used to establish reliable communication between a Controller and an Agent. I also learned that TCP is a byte-stream protocol, so applications must implement their own message framing when using newline-terminated commands.

I gained a better understanding of POSIX threads by implementing a thread-per-client concurrency model. Each Controller connection is handled by a separate thread, allowing the Agent to serve multiple clients concurrently.

The assignment also improved my understanding of authentication, command validation, file transfer, UDP communication, process information, system information, and timestamped logging.

## 2. Challenges Faced

One of the main challenges was handling TCP data correctly. A single recv() call cannot be assumed to receive a complete command or file. To solve this, newline-based command reception and exact-byte send/receive helper functions were implemented.

Another challenge was implementing file transfer reliably. The PUT and GET operations needed to transfer exactly the specified number of bytes. The send_all() and recv_all() functions helped ensure that partial TCP transfers were handled correctly.

Implementing the monitoring feature was another challenge. The initial monitoring state could be shared between client connections. This was improved by using a separate monitoring context for each client connection.

Debugging the authentication and command-processing sequence also helped me understand how protocol rules should be enforced before allowing sensitive operations.

## 3. How I Solved the Problems

I solved the implementation problems by breaking the assignment into smaller functional sections and testing each section individually.

First, I implemented the TCP connection between the Agent and Controller. Then authentication was added so that commands could only be processed after successful authentication.

After that, system information, process listing, restricted EXEC commands, file transfer, monitoring, and logging were implemented.

Compilation was repeatedly checked using the personalised Makefile. Functional testing was then performed using the Controller, including both valid and invalid commands.

For file transfer verification, the original and downloaded files were compared using cmp and SHA-256 hashing. The matching hashes confirmed that the transferred file was byte-for-byte identical.

## 4. Security and Design Lessons

The restricted EXEC whitelist was an important security lesson. Allowing arbitrary shell commands would create unnecessary security risks, so only the five required commands were accepted:

DATE
UPTIME
DISKFREE
HOSTNAME
WHOAMI

I also learned the importance of authentication before command processing. The personalised token OPS-4117 must be successfully authenticated before other commands are accepted.

Filename validation was also considered during PUT and GET to prevent directory traversal through filenames containing path separators or "..".

## 5. Testing and Verification

The final implementation was tested for:

- Successful authentication
- SYSINFO
- LISTPROC
- All five allowed EXEC commands
- Rejection of an unauthorised EXEC command
- PUT file upload
- GET file download
- Byte-for-byte file verification
- MONITOR START
- UDP monitoring packets
- MONITOR STOP
- QUIT
- TCP connection handling
- Compilation using GCC

The Agent and Controller compiled successfully using:

make -f Makefile_117 clean
make -f Makefile_117

The file transfer test used a 58-byte test file. The original and downloaded files produced the same SHA-256 hash:

2b854da3fd6f2863112098ee33d2bd95373e11ca5b8744bd6b73c6af2ab5557d

## 6. Use of AI Assistance

AI assistance was used during the assignment for understanding requirements, planning implementation steps, debugging C and socket-related issues, reviewing protocol handling, preparing testing procedures, and structuring documentation.

The suggestions were not treated as a replacement for testing. Code and configuration were tested in the actual CentOS environment, and problems encountered during implementation were investigated and corrected.

The PROMPT_LOG.md file records the main areas where AI assistance was used.

## 7. Overall Reflection

This assignment helped me connect networking theory with practical programming. Concepts such as TCP sockets, concurrency, authentication, UDP, process information, file transfer, and protocol framing became clearer through implementation.

The most important lesson was that a network program must not assume that data arrives exactly as expected. Correct handling of partial TCP data, exact byte counts, authentication state, concurrent connections, and errors is necessary for a reliable implementation.

Overall, the project improved my confidence in C network programming and gave me practical experience in designing, implementing, testing, debugging, and documenting a complete client-server system.
