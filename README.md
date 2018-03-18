# traceroute
My implementation of traceroute, a tool for displaying route and transit delays between nodes in IP network.

### Description
This traceroute sends 3 ICMP Echo Request per each TTL (TTLs in range from 1 to 30) and waits 1 second for reply (Time Exceeded or ICMP Echo Reply, which ends the loop).

Answers which come within 1 second are displayed as IP address and latency.

### Usage
First, compile by running:
```
$ make
```

then get some IP:
```
$ host google.com
google.com has address 216.58.209.78
```
and finally:
```bash
$ sudo ./traceroute 216.58.209.78
[...]
5. 195.149.232.200 195.149.232.200  30.3 ms  33.7 ms
6. 188.47.253.234  48.6 ms
7. 72.14.197.128  47.6 ms
8. 108.170.250.209  216.1 ms
9. 209.85.244.67 209.85.244.67  25.3 ms  34.3 ms
10. 216.58.209.78  43.1 ms
```
You have to run it as root because using ICMP requires raw sockets. Regular traceroute with `-I` option (use ICMP instead of UDP) also requires root privileges.