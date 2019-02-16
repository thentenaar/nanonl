noinst_PROGRAMS = genl-find-family nfqueue monitor-addr-change   \
                  dump-ip-addrs dump-neighbors monitor-neighbors \
				  dump-ct monitor-ct-del

genl_find_family_SOURCES = genl-find-family.c ../src/nl.c ../src/nl_gen.c
nfqueue_SOURCES = nfqueue.c ../src/nl.c ../src/nl_nf.c ../src/nl_nfqueue.c
monitor_addr_change_SOURCES = monitor-addr-change.c ../src/nl.c
dump_ip_addrs_SOURCES = dump-ip-addrs.c ../src/nl.c ../src/nl_ifaddr.c
dump_neighbors_SOURCES = dump-neighbors.c ../src/nl.c ../src/nl_nd.c
monitor_neighbors_SOURCES = monitor-neighbors.c ../src/nl.c ../src/nl_nd.c
monitor_ct_del_SOURCES = monitor-conntrack-del.c ../src/nl.c ../src/nl_nf.c
dump_ct_SOURCES = dump-conntrack.c ../src/nl.c ../src/nl_nf.c ../src/nl_nfct.c

