# IE3090 RemoteOps Design Diary

## Student
Registration Number: IT24104117

## Project Overview

The RemoteOps project implements a C-based remote management Agent and Controller using the BSD sockets API. The Agent listens on the personalised TCP port 9410 and communicates with the Controller using the required RemoteOps protocol.

## Key Design Decisions

### TCP Communication

TCP sockets were selected for the main Agent-Controller communication because the protocol requires reliable and ordered delivery of commands, responses, and file data.

### Concurrency Model

A thread-per-client model using POSIX pthreads was selected. Each incoming Controller connection is handled by a separate thread. This allows the Agent to serve multiple Controller connections concurrently while keeping the implementation understandable.

### Authentication

The Agent requires the personalised authentication token `OPS-4117` before allowing other commands. The session identifier `SID:7114` is included in responses to make each submission individually identifiable.

### Command Security

The EXEC command uses a fixed whitelist containing only DATE, UPTIME, DISKFREE, HOSTNAME, and WHOAMI. Arbitrary shell commands such as LS are rejected to satisfy the security requirement.

### File Transfer

PUT and GET were implemented using exact byte counts. Helper functions were used to ensure that partial TCP sends and receives are handled correctly. Uploaded files are stored under `./agentfiles/IT24104117/`.

### Monitoring

MONITOR START creates a UDP monitoring thread for the client connection. System information is periodically sent to the Controller as UDP datagrams. MONITOR STOP terminates the monitoring activity.

### Logging

Timestamped logging was added to record connections, authentication, commands, file transfers, and disconnections.

## Development Obstacles

During development, several issues were encountered.

The first issue involved implementing TCP message framing. A single `recv()` call cannot be assumed to contain a complete command, so a newline-based receive function was introduced for command lines. Separate exact-byte receive and send functions were implemented for file transfers.

Another issue involved ensuring that PUT and GET transfer the exact number of bytes. This was addressed by implementing `send_all()` and `recv_all()` helper functions.

The monitoring implementation also required improvement. Initially, monitoring state was shared globally, which could cause problems when multiple Controllers were connected. The design was changed to maintain monitoring state separately for each client connection.

## Testing Progress

The Agent and Controller were compiled using the personalised Makefile:

`make -f Makefile_117`

The final implementation was tested for authentication, SYSINFO, LISTPROC, all five allowed EXEC commands, rejection of a disallowed command, PUT, GET, UDP monitoring, MONITOR STOP, and QUIT.

The uploaded test file was transferred successfully and the downloaded file was verified to be byte-for-byte identical to the original.

## Final Outcome

The final implementation successfully demonstrates the required RemoteOps TCP communication, authentication, system information retrieval, process listing, restricted command execution, file transfer, UDP monitoring, concurrency model, and logging.
