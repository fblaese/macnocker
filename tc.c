#include "tc.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>

#include <linux/if_ether.h>
#include <linux/pkt_cls.h>
#include <linux/pkt_sched.h>
#include <linux/rtnetlink.h>


#include <netlink/msg.h>
#include <netlink/attr.h>
#include <netlink/socket.h>

/*
 * if=eth0
 *
 * # qdisc anlegen:
 * tc qdisc add dev $if ingress
 *
 * # alles sperren:
 * tc filter add dev $if protocol all parent ffff: prio 65535 basic match "u32(u16 0x4305 0xffff at -2)" flowid :1      action drop
 *
 * # eine mac frei schalten:
 * tc filter add dev $if protocol all parent ffff: prio 99  basic match "u32(u32 0xf81a67a5 0xffffffff at -8)" and      "u32(u16 0xf4cb 0xffff at -4)" flowid :1 action pass
 *
 * # qdisc anzeigen
 * tc qdisc
 *
 * # qdisc löschen
 * tc qdisc del dev $if ingress
 *
 * # filter anzeigen
 * tc filter show dev $if ingress
 */

extern const char *g_interface;

static bool nlexpect;
static int nlerror;

static struct nl_cb *cb;
static struct nl_sock *sock;

static int error_handler(struct sockaddr_nl *nla __attribute__((unused)), struct nlmsgerr *nlerr, void *arg __attribute__((unused))) {
	if (!nlexpect || (nlerr->error != -ENOENT && nlerr->error != -EINVAL))
		nlerror = -nlerr->error;

	return NL_STOP;
}

static inline void print_error(const char *prefix, const char *message, int err) {
	if (err)
		fprintf(stderr, "simple-tc: %s: %s: %s\n", prefix, message, strerror(err));
	else
		fprintf(stderr, "simple-tc: %s: %s\n", prefix, message);
}

static inline void warn_error(const char *message, int err) {
	print_error("error", message, err);
}

__attribute__((noreturn)) static inline void exit_error(const char *message, int err) {
	print_error("error", message, err);
	exit(1);
}

__attribute__((noreturn)) static inline void exit_errno(const char *message) {
	exit_error(message, errno);
}

static struct nl_msg * prepare_tcmsg(int type, int flags, unsigned int ifindex, uint32_t parent, uint32_t handle, uint32_t info) {
    struct nl_msg *msg = nlmsg_alloc_simple(type, flags);
    if (!msg)
        exit_errno("nllmsg_alloc_simple");

    struct tcmsg tcmsg;
    memset(&tcmsg, 0, sizeof(tcmsg));

    tcmsg.tcm_family = AF_UNSPEC;
    tcmsg.tcm_ifindex = ifindex;
    tcmsg.tcm_parent = parent;
    tcmsg.tcm_handle = handle;
    tcmsg.tcm_info = info;

    nlmsg_append(msg, &tcmsg, sizeof(tcmsg), NLMSG_ALIGNTO);

    return msg;
}

static bool do_send(struct nl_msg *msg, bool expect) {
	nlerror = 0;
	nlexpect = expect;

	nl_send_auto_complete(sock, msg);
	nlmsg_free(msg);
	nl_wait_for_ack(sock);

	if (nlerror) {
		warn_error("netlink", nlerror);
		errno = nlerror;
		return false;
	}

	return true;
}

void tc_add_qdisc_ingress()
{
    unsigned int ifindex = if_nametoindex(g_interface);
    if (!ifindex) {
        fprintf(stderr, "invalid interface: %s\n", g_interface);
        return;
    }
    struct nl_msg *msg = prepare_tcmsg(RTM_NEWQDISC, NLM_F_CREATE, ifindex, TC_H_INGRESS, 0xffff0000, 0);
    nla_put_string(msg, TCA_KIND, "ingress");
    do_send(msg, false);
}

void tc_del_qdisc_ingress()
{
    /* char cmd[2048]; */
    /* snprintf(cmd, 2048, "tc qdisc del dev %s ingress", g_interface); */
    /* log_trace("CMD: %s\n", cmd); */
    /* system(cmd); */

    unsigned int ifindex = if_nametoindex(g_interface);
    if (!ifindex) {
        fprintf(stderr, "invalid interface: %s\n", g_interface);
        return;
    }
    do_send(prepare_tcmsg(RTM_DELQDISC, 0, ifindex, TC_H_INGRESS, 0xffff0000, 0), true);
}

void tc_block_all()
{
    unsigned int ifindex = if_nametoindex(g_interface);
    if (!ifindex) {
        fprintf(stderr, "invalid interface: %s\n", g_interface);
        return;
    }
    struct nl_msg *msg = prepare_tcmsg(RTM_NEWTFILTER, NLM_F_CREATE|NLM_F_EXCL, ifindex, 0xffff0000, 0, TC_H_MAKE((uint32_t) 65535 << 16, htons(ETH_P_ALL)));

    nla_put_string(msg, TCA_KIND, "basic");

    /* options magic for basic tca */
    /* i am unable to properly use rtnetlink, this is equivalent to: */
    /* match "u32(u16 0x4305 0xffff at -2)" flowid :1 action drop */
    nla_put(msg, TCA_OPTIONS, 100, "\x2c\x00\x02\x00\x08\x00\x01\x00\x01\x00\x00\x00\x20\x00\x02\x00\x1c\x00\x01\x00\x00\x00\x03\x00\x00"
                                   "\x00\x00\x00\x00\x00\xff\xff\x00\x00\x43\x05\xfc\xff\xff\xff\x00\x00\x00\x00\x08\x00\x01\x00\x01\x00"
                                   "\x00\x00\x30\x00\x03\x00\x2c\x00\x01\x00\x09\x00\x01\x00\x67\x61\x63\x74\x00\x00\x00\x00\x1c\x00\x02"
                                   "\x80\x18\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

    do_send(msg, false);
}

void tc_allow_mac(const uint8_t mac[], uint8_t prio)
{
    /* options magic for basic tca */
    /* i am unable to properly use rtnetlink, this is equivalent to: */
    /* match "u32(u16 0xmac[0,1,2,3] 0xffff at -2)" and "u32(u16 0xmac[4,5] at -4)" flowid :1 action pass */
    char options[128];
    memcpy(options, "\x48\x00\x02\x00\x08\x00\x01\x00\x02\x00\x00\x00\x3c\x00\x02\x00\x1c\x00\x01\x00\x00\x00\x03\x00\x01\x00\x00\x00", 28);
    memcpy(options + 28, mac, 4);
    memcpy(options + 32, mac, 4);
    memcpy(options + 36, "\xf8\xff\xff\xff\x00\x00\x00\x00\x1c\x00\x02\x00\x00\x00\x03\x00\x00\x00\x00\x00", 20);
    memcpy(options + 56, mac + 4, 2);
    memcpy(options + 58, "\x00\x00", 2);
    memcpy(options + 60, mac + 4, 2);
    memcpy(options + 62, "\x00\x00\xfc\xff\xff\xff\x00\x00\x00\x00\x08\x00\x01\x00\x01\x00\x00\x00\x30\x00\x03\x00\x2c\x00\x01\x00\x09"
                         "\x00\x01\x00\x67\x61\x63\x74\x00\x00\x00\x00\x1c\x00\x02\x80\x18\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 66);

    unsigned int ifindex = if_nametoindex(g_interface);
    if (!ifindex) {
        fprintf(stderr, "invalid interface: %s\n", g_interface);
        return;
    }
    struct nl_msg *msg = prepare_tcmsg(RTM_NEWTFILTER, NLM_F_CREATE|NLM_F_EXCL, ifindex, 0xffff0000, 0, TC_H_MAKE((uint32_t) prio << 16, htons(ETH_P_ALL)));

    nla_put_string(msg, TCA_KIND, "basic");
    nla_put(msg, TCA_OPTIONS, 128, options);

    do_send(msg, false);
}

void tc_disallow_mac(const uint8_t mac[], uint8_t prio)
{
    /* options magic for basic tca */
    /* i am unable to properly use rtnetlink, this is equivalent to: */
    /* match "u32(u16 0xmac[0,1,2,3] 0xffff at -2)" and "u32(u16 0xmac[4,5] at -4)" flowid :1 action pass */
    char options[128];
    memcpy(options, "\x48\x00\x02\x00\x08\x00\x01\x00\x02\x00\x00\x00\x3c\x00\x02\x00\x1c\x00\x01\x00\x00\x00\x03\x00\x01\x00\x00\x00", 28);
    memcpy(options + 28, mac, 4);
    memcpy(options + 32, mac, 4);
    memcpy(options + 36, "\xf8\xff\xff\xff\x00\x00\x00\x00\x1c\x00\x02\x00\x00\x00\x03\x00\x00\x00\x00\x00", 20);
    memcpy(options + 56, mac + 4, 2);
    memcpy(options + 58, "\x00\x00", 2);
    memcpy(options + 60, mac + 4, 2);
    memcpy(options + 62, "\x00\x00\xfc\xff\xff\xff\x00\x00\x00\x00\x08\x00\x01\x00\x01\x00\x00\x00\x30\x00\x03\x00\x2c\x00\x01\x00\x09"
                         "\x00\x01\x00\x67\x61\x63\x74\x00\x00\x00\x00\x1c\x00\x02\x80\x18\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 66);

    unsigned int ifindex = if_nametoindex(g_interface);
    if (!ifindex) {
        fprintf(stderr, "invalid interface: %s\n", g_interface);
        return;
    }
    struct nl_msg *msg = prepare_tcmsg(RTM_DELTFILTER, 0, ifindex, 0xffff0000, 0, TC_H_MAKE((uint32_t) prio << 16, htons(ETH_P_ALL)));

    nla_put_string(msg, TCA_KIND, "basic");
    nla_put(msg, TCA_OPTIONS, 128, options);

    do_send(msg, false);
}

void tc_start()
{
    log_debug("[t] Opening RTNETLINK socket.\n");
    cb = nl_cb_alloc(NL_CB_DEFAULT);
    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, NULL);

    sock = nl_socket_alloc_cb(cb);
    if (!sock)
        exit_errno("nl_socket_alloc");

    if (nl_connect(sock, NETLINK_ROUTE))
        exit_errno("nl_connect");


    log_debug("[t] Removing old qdisc.\n");
    tc_del_qdisc_ingress(); // in case a old session is sill there

    log_debug("[t] Adding qdisc.\n");
    tc_add_qdisc_ingress();
    log_debug("[t] Blocking all batman-adv traffic.\n");
    tc_block_all();
}

void tc_stop()
{
    log_debug("[t] Removing qdisc.\n");
    tc_del_qdisc_ingress();

    log_debug("[t] Closing RTNETLINK socket.\n");
    nl_socket_free(sock);
    nl_cb_put(cb);
}
