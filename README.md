nanonl
======

This library implements a small set of helper functions for constructing
netlink messages, and looking up netlink attributes (NLAs) within nmessages.

The helper functions for each subsystem mau be independently enabled.

Configure Options
-----------------
```
  --enable-all            enable support for everything
  --enable-generic        enable netlink generic support
  --enable-netfilter      enable nfnetlink support
  --enable-nfqueue        enable nfqueue support (implies netfilter)
  --enable-ifinfo         enable interface info support
  --enable-ifaddr         enable interface address support
```

What this library doesn't do
----------------------------

- Manage memory
- Chew bubble gum (it's all out...)

