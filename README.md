nanonl
======

This library implements a small set of helper functions for constructing
netlink messages, and looking up netlink attributes (NLAs) within nmessages.

The helper functions for each subsystem must be independently enabled.

Configure Options
-----------------
```
  --enable-generic        enable netlink generic support
  --enable-netfilter      enable nfnetlink support
  --enable-nfqueue        enable nfqueue support (implies netfilter)
```

What this library doesn't do
----------------------------

- Manage memory
- Poll file descriptors
- Chew bubble gum (it's all out...)

