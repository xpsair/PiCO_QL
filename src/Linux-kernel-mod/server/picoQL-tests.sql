SELECT * FROM Process_VT;
SELECT * FROM Process_VT JOIN EVirtualMem_VT ON EVirtualMem_VT.base = Process_VT.vm_id;
SELECT P1.name, F1.inode_name, P2.name, F2.inode_name FROM Process_VT as P1 JOIN EFile_VT as F1 ON  F1.base = P1.fs_fd_file_id, Process_VT as P2 JOIN EFile_VT as F2 ON  F2.base = P2.fs_fd_file_id WHERE P1.pid <> P2.pid AND F1.path_mount = F2.path_mount AND F1.path_dentry = F2.path_dentry AND F1.inode_name NOT IN ('null', '');
SELECT P1.name, F1.inode_name, S.name FROM Process_VT as P1 JOIN EFile_VT as F1 ON  F1.base = P1.fs_fd_file_id JOIN ESuperblock_VT AS S ON S.base = F1.sb_id;
SELECT DISTINCT P1.name, F1.inode_name, P2.name, F2.inode_name FROM Process_VT as P1 JOIN EFile_VT as F1 ON  F1.base = P1.fs_fd_file_id, Process_VT as P2 JOIN EFile_VT as F2 ON  F2.base = P2.fs_fd_file_id WHERE P1.pid <> P2.pid AND F1.path_mount = F2.path_mount AND F1.path_dentry = F2.path_dentry AND F1.inode_name NOT IN ('null', '');
SELECT GE.name, GE.cred_uid, GE.ecred_euid, GE.ecred_egid, GE.gid FROM ( SELECT name, cred_uid, ecred_euid, ecred_egid, G.gid FROM Process_VT AS P JOIN EGroup_VT AS G ON G.base=P.group_set_id WHERE NOT EXISTS ( SELECT gid FROM EGroup_VT WHERE EGroup_VT.base = P.group_set_id AND gid IN (4,27))) GE WHERE GE.cred_uid > 0 AND GE.ecred_euid = 0;
SELECT name, cred_uid, ecred_euid, ecred_egid, G.gid FROM Process_VT AS P JOIN EGroup_VT AS G ON G.base=P.group_set_id WHERE EXISTS (SELECT gid FROM EGroup_VT WHERE EGroup_VT.base = P.group_set_id AND gid in (4,27));
SELECT PE.name, PE.cred_uid, PE.ecred_euid, PE.ecred_egid, G.gid FROM (SELECT name, cred_uid, ecred_euid, ecred_egid, group_set_id FROM Process_VT AS P WHERE NOT EXISTS (SELECT gid FROM EGroup_VT WHERE EGroup_VT.base = P.group_set_id AND gid IN (4,27))) PE JOIN EGroup_VT AS G ON G.base=PE.group_set_id WHERE PE.cred_uid > 0 AND PE.ecred_euid = 0;
SELECT PE.name, PE.cred_uid, PE.ecred_euid, PE.ecred_egid, G.gid FROM (SELECT name, cred_uid, ecred_euid, ecred_egid, group_set_id FROM Process_VT AS P WHERE NOT EXISTS (SELECT gid FROM EGroup_VT WHERE EGroup_VT.base = P.group_set_id AND gid IN (4,27))) PE JOIN EGroup_VT AS G ON G.base=PE.group_set_id;
SELECT DISTINCT P.name, F.inode_name, F.inode_mode&400, F.inode_mode&40, F.inode_mode&4, F.fowner_euid, P.ecred_fsuid, F.fcred_egid, F.fmode&1 FROM Process_VT AS P JOIN EFile_VT AS F ON F.base=P.fs_fd_file_id WHERE F.fmode&1 AND (F.fowner_euid != P.ecred_fsuid OR NOT F.inode_mode&400) AND (F.fcred_egid NOT IN ( SELECT gid FROM EGRoup_VT AS G WHERE G.base = P.group_set_id) OR NOT F.inode_mode&40) AND NOT F.inode_mode&4;
SELECT P.name, P.pid, P.state, EP.name, EP.pid, EP.state FROM Process_VT as P JOIN EProcess_VT as EP ON EP.base=P.parent_id WHERE P.pdeath_signal = 9 AND EP.pdeath_signal = 0 AND EP.exit_state = 0;
SELECT P.name, P.pid, EP.name, EP.pid, ET.name, ET.pid FROM Process_VT as P JOIN EProcessChild_VT as EP ON EP.base=P.children_id JOIN EThread_VT as ET ON ET.base=P.thread_group_id WHERE NOT P.exit_state&~-512 AND EP.exit_state&~-512 AND ET.exit_state&~-512 AND EP.parent_id=ET.base;
SELECT P.name, P.pid, EP.name, EP.pid, ET.name, ET.pid, EP.thread_group_id, ET.thread_group_id, EP.children_id, ET.children_id FROM (SELECT name, pid, children_id, thread_group_id,exit_state FROM Process_VT WHERE pid>0) P JOIN EprocessChild_VT AS EP ON EP.base=P.children_id JOIN Ethread_VT AS ET ON ET.base=P.thread_group_id WHERE NOT P.exit_state&~-512;
SELECT P.name, P.pid, EP.name, EP.pid, ET.name, ET.pid, EP.thread_group_id, ET.thread_group_id, EP.children_id, ET.children_id FROM (SELECT DISTINCT name, pid, children_id, thread_group_id,exit_state FROM Process_VT WHERE pid>0) P JOIN EprocessChild_VT AS EP ON EP.base=P.children_id JOIN Ethread_VT AS ET ON ET.base=P.thread_group_id WHERE NOT P.exit_state&~-512;
SELECT P.name, P.pid, P.tgid, PIO.read_bytes_syscall, PIO.write_bytes_syscall, SUM(ETIO.read_bytes_syscall), SUM(ETIO.write_bytes_syscall), PIO.read_bytes_syscall + SUM(ETIO.read_bytes_syscall), PIO.write_bytes_syscall + SUM(ETIO.write_bytes_syscall) FROM Process_VT as P JOIN EIO_VT as PIO ON PIO.base=P.io_id JOIN EThread_VT as ET ON ET.base=P.thread_group_id JOIN EIO_VT as ETIO ON ETIO.base=ET.io_id GROUP BY ET.tgid;
SELECT name, pid, users, counter, map_count, total_vm, shared_vm FROM Process_VT AS P JOIN EVirtualMem_VT AS VM ON VM.base = P.vm_id ORDER BY total_vm, shared_vm;
SELECT RtoAlgorithm, RtoMin, RtoMax, MaxConn, ActiveOpens, PassiveOpens, AttemptFails, EstabResets, CurrEstab, InSegs, OutSegs, RetransSegs, InErrs, InErrs*100/InSegs, OutRsts, OutRsts*100/OutSegs FROM Nsproxy_VT AS NPRX JOIN ENetNamespace_VT AS NMSP ON NMSP.base = NPRX.net_ns_id JOIN ENetMib_VT as NMB ON NMB.base = NMSP.netns_mib_id JOIN ETcpStat_VT AS TCP ON TCP.base = NMB.tcp_stats_id WHERE InErrs*100/InSegs > 20 OR OutRsts*100/OutSegs > 20;
SELECT * FROM NetNamespace_VT JOIN ENetDevice_VT ON ENetDevice_VT.base=NetNamespace_VT.dev_list_id;
SELECT * FROM NetNamespace_VT JOIN ENetDevice_VT ON ENetDevice_VT.base=NetNamespace_VT.dev_list_id JOIN EKobjectSet_VT EKS ON EKS.base = queues_kset_id;
SELECT cpu, vcpu_id, vcpu_mode, vcpu_requests, current_privilege_level, hypercalls_allowed FROM KVM_VCPU_View;
SELECT cpu, vcpu_id, vcpu_mode, vcpu_requests,interrupt_shadow FROM KVM_VCPU_View AS VCPU JOIN EKVMVCPUEvents_VT AS VCPUEvents ON VCPUEvents.base=VCPU.vcpu_events_id;
SELECT kvm_users, APCS.count,latched_count, count_latched, status_latched, status, read_state, write_state, rw_mode, mode, bcd, gate, count_load_time FROM KVM_View AS KVM JOIN EKVMArchPitChannelState_VT AS APCS ON APCS.base=KVM.kvm_pit_state_id;
SELECT name, inode_name, file_offset, page_offset, inode_size_bytes, inode_size_pages, pages_in_cache_contig_start, pages_in_cache_contig_current, pages_in_cache_tag_dirty, pages_in_cache_tag_writeback, pages_in_cache_tag_towrite, pages_in_cache FROM Process_VT AS P JOIN EFile_VT AS F ON F.base=P.fs_fd_file_id WHERE pages_in_cache_tag_dirty AND name like '%kvm%';
SELECT name, inode_name, inode_size_bytes, socket_state, socket_type, drops, errors, errors_soft, drops/inode_size_bytes FROM Process_VT AS P JOIN EFile_VT AS F ON F.base = P.fs_fd_file_id JOIN ESocket_VT AS SKT ON SKT.base = F.socket_id JOIN ESock_VT AS SK ON SK.base = SKT.sock_id WHERE drops*100/inode_size_bytes > 10 AND name = 'sshd';
SELECT name, inode_name, inode_size_bytes, socket_state, socket_type, drops, errors, errors_soft, drops/inode_size_bytes, len FROM Process_VT AS P JOIN EFile_VT AS F ON F.base = P.fs_fd_file_id JOIN ESocket_VT AS SKT ON SKT.base = F.socket_id JOIN ESock_VT AS SK ON SK.base = SKT.sock_id JOIN ERcvQueue_VT Rcv ON Rcv.base=receive_queue_id;
SELECT name, pid, cred_gid, utime, stime, total_vm, nr_ptes, inode_name, inode_no, rem_ip, rem_port, local_ip, local_port, tx_queue, rx_queue FROM Process_VT AS P JOIN EVirtualMem_VT AS VM ON VM.base = P.vm_id JOIN EFile_VT AS F ON F.base = P.fs_fd_file_id JOIN ESocket_VT AS SKT ON SKT.base = F.socket_id JOIN ESock_VT AS SK ON SK.base = SKT.sock_id where proto_name like 'tcp';
SELECT * FROM VirtualMemZone_VT;
SELECT name, page_type, lowmem_reserve FROM VirtualMemZone_VT M JOIN ELowmemReserve_vt LR ON LR.base=M.lowmem_zone_protection_id WHERE spanned_pages > 0 AND priority < page_type_priority EXCEPT SELECT M2.name, M1.name, SUM(M3.managed_pages)/SLR.lowmem_reserve FROM VirtualMemZone_VT M1, VirtualMemZone_VT M2 JOIN SysctlLowmemRR_VT SLR ON SLR.page_type_priority = m2.priority, VirtualMemZone_VT M3 WHERE M2.spanned_pages > 0 AND M1.priority > M2.priority AND M3.priority <= M1.priority AND M3.priority > M2.priority GROUP BY M2.priority, M1.priority;
SELECT name, watermark_min, watermark_low, watermark_high, free_pages, page_type, lowmem_reserve FROM VirtualMemZone_VT M JOIN ELowmemReserve_VT LR ON LR.base=M.lowmem_zone_protection_id WHERE free_pages > lowmem_reserve + watermark_high;
