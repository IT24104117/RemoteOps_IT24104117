# IE3090 RemoteOps

## Student Personalisation

Registration Number: IT24104117

### Personalised Values

- Agent Listening Port: 9410
- SID: SID:7114
- Authentication Token: OPS-4117
- Agent Source File: agent_117.c
- Controller Source File: controller_117.c
- Makefile: Makefile_117
- Log File: RemoteOps_IT24104117.log
- File Storage Path: ./agentfiles/IT24104117/
- Submission Archive: IE3090_IT24104117.zip

## Personalisation Calculations

### Agent Port
Numeric part of registration number: 24104117

First four digits: 2410

Agent port:

7000 + 2410 = 9410

### Source File Names
Last three digits: 117

Therefore:

- agent_117.c
- controller_117.c
- Makefile_117

### Session ID
Last four digits: 4117

Reversed: 7114

Therefore:

SID:7114

### Authentication Token
Last four digits: 4117

Therefore:

OPS-4117

## Project Description

RemoteOps is a C-based remote management system consisting of a TCP Agent and a Controller.

The Agent listens for Controller connections on TCP port 9410. The Controller authenticates using the personalised authentication token before accessing RemoteOps commands.

The system supports:

- Authentication
- System information
- Process listing
- Restricted command execution
- File upload using PUT
- File download using GET
- UDP system monitoring
- Graceful disconnection
- Timestamped logging

## Concurrency Model

The Agent uses a thread-per-client concurrency model.

Each incoming Controller connection is handled by a separate POSIX thread using pthreads. This allows the Agent to serve multiple simultaneous Controller connections.

## Protocol

The implementation follows the required newline-terminated command/response protocol.

All successful responses include the personalised SID:

SID:7114

The EXEC command uses a fixed whitelist:

- DATE
- UPTIME
- DISKFREE
- HOSTNAME
- WHOAMI

Arbitrary shell commands are rejected.

## File Storage

Uploaded files are stored under:

./agentfiles/IT24104117/

The implementation validates filenames and limits uploaded file sizes.

## Build Instructions

Compile both programs using:

make -f Makefile_117

Clean compiled binaries using:

make -f Makefile_117 clean

## Running the Agent

Start the Agent:

./agent_117

The Agent listens on TCP port 9410.

## Running the Controller

In another terminal:

./controller_117

The Controller connects to the Agent and performs the required protocol tests.

## Testing

The implementation has been tested for:

- TCP connection
- Authentication
- SYSINFO
- LISTPROC
- Allowed EXEC commands
- Rejection of disallowed EXEC commands
- PUT
- GET
- Byte-for-byte file transfer verification
- UDP MONITOR START
- UDP monitoring packets
- MONITOR STOP
- QUIT
- Multiple Controller connections

## Repository

The project is maintained in the GitHub repository:

https://github.com/IT24104117/RemoteOps_IT24104117
