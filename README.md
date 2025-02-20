# TinyDNS

A basic DNS protocol library intended for embedded systems.

### Supported Record Types
These are just the basics I needed to support my use case.<br>
PRs are welcome for more record types.

- A
- AAAA
- CNAME
- SRV
- TXT

## Goals
- No memory allocation to make integration simple for embedded systems.
- No assumptions about networking stack -- ship tinyDNS bytes from any source, as long as they're DNS.
- Keep the API simple, flexible, and small.

## Non-goals
- Supporting EDNS
- Supporting DNS over TLS

## Building
```bash
cmake -Bbuild .
cmake --build build
cmake --build build -t test
```
