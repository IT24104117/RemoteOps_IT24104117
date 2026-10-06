# IE3090 RemoteOps AI Prompt Log

## Student
Registration Number: IT24104117

## Purpose

This document records the AI assistance used during the development, debugging, testing, and documentation of the IE3090 RemoteOps assignment.

## Prompt 1 – Assignment Analysis

Asked for help understanding the RemoteOps assignment requirements and identifying the required Agent and Controller functionality.

### Result
The mandatory requirements were broken down into:
- TCP Agent-Controller communication
- Authentication
- SYSINFO
- LISTPROC
- Restricted EXEC commands
- PUT and GET file transfer
- UDP MONITOR START/STOP
- Logging
- Concurrent client handling
- Required documentation and testing evidence

## Prompt 2 – Personalisation

Asked for help calculating and applying the personalised assignment values for registration number IT24104117.

### Result
The following values were determined and used:

- Registration Number: IT24104117
- Numeric part: 24104117
- Agent TCP Port: 9410
- Source suffix: 117
- Agent source file: agent_117.c
- Controller source file: controller_117.c
- Makefile: Makefile_117
- Authentication Token: OPS-4117
- Session ID: 7114
- Storage Directory: ./agentfiles/IT24104117/
- Log File: RemoteOps_IT24104117.log

## Prompt 3 – Agent and Controller Implementation

Asked for step-by-step assistance implementing the RemoteOps Agent and Controller in C using BSD sockets.

### Result
The implementation was developed using:
- TCP sockets for Agent-Controller communication
- POSIX threads for concurrent client handling
- Authentication before command processing
- Helper functions for complete send/receive operations
- Newline-based command framing
- Exact byte-count handling for file transfers

## Prompt 4 – Authentication Debugging

Asked for help implementing and debugging the personalised AUTH command.

### Result
The Agent was configured to accept:

AUTH OPS-4117

A successful authentication returns:

OK AUTHENTICATED SID:7114

Unauthorised commands are rejected until authentication succeeds.

## Prompt 5 – EXEC Security

Asked how to implement the required restricted EXEC command functionality.

### Result
A fixed whitelist was implemented containing only:

- DATE
- UPTIME
- DISKFREE
- HOSTNAME
- WHOAMI

A command outside the whitelist, such as LS, returns:

ERR 002 COMMAND_NOT_ALLOWED SID:7114

## Prompt 6 – File Transfer

Asked for help implementing PUT and GET while correctly handling TCP partial sends and receives.

### Result
send_all() and recv_all() helper functions were used so that the exact number of bytes specified by the file size is transferred.

Uploaded files are stored under:

./agentfiles/IT24104117/

The GET operation was tested by downloading the uploaded test file and comparing it with the original.

## Prompt 7 – Monitoring

Asked for help implementing MONITOR START and MONITOR STOP using UDP.

### Result
The monitoring functionality was implemented using a separate monitoring thread. System information is periodically sent to the Controller through UDP.

The implementation was tested using UDP port 9500.

The monitoring output included CPU load, memory usage, uptime, and SID:7114.

## Prompt 8 – Concurrency and Monitor State

Asked for help reviewing the concurrency model and monitoring state when multiple Controller connections are handled.

### Result
A thread-per-client model using pthreads was used. Each Controller connection is handled by a separate thread.

The monitoring state was changed from shared global state to per-client monitoring state to avoid different Controller sessions interfering with each other.

## Prompt 9 – Build and Testing

Asked for help checking compilation and testing the complete RemoteOps implementation.

### Result
The personalised Makefile was used:

make -f Makefile_117 clean
make -f Makefile_117

Both the Agent and Controller compiled successfully using GCC without compilation errors or warnings.

The following functionality was tested:
- AUTH
- SYSINFO
- LISTPROC
- DATE
- UPTIME
- DISKFREE
- HOSTNAME
- WHOAMI
- Rejection of LS
- PUT
- GET
- MONITOR START
- MONITOR STOP
- QUIT

## Prompt 10 – Documentation

Asked for help preparing the README and Design Diary required for the assignment.

### Result
The README was updated with the personalised configuration, project description, concurrency model, protocol information, build instructions, and testing information.

The Design Diary was created to document design decisions, implementation obstacles, testing progress, and the final outcome.

## Prompt 11 – File Integrity Verification

Asked for help verifying that the file transferred using PUT and GET was byte-for-byte identical.

### Result
The original test file and downloaded file were compared using cmp and SHA-256 hashing.

Both files were 58 bytes and produced the same SHA-256 hash:

2b854da3fd6f2863112098ee33d2bd95373e11ca5b8744bd6b73c6af2ab5557d

This confirmed successful byte-for-byte file transfer.

## Reflection on AI Assistance

AI assistance was used mainly for understanding requirements, implementation guidance, debugging, testing ideas, and documentation structure. The generated suggestions were reviewed and adapted during implementation. The final implementation was compiled and tested in the student's CentOS environment.

