#include "tc.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>

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

void tc_add_qdisc_ingress()
{
    char cmd[2048];
    snprintf(cmd, 2048, "tc qdisc add dev %s ingress", g_interface);
    log_trace("CMD: %s\n", cmd);
    system(cmd);
}

void tc_del_qdisc_ingress()
{
    char cmd[2048];
    snprintf(cmd, 2048, "tc qdisc del dev %s ingress", g_interface);
    log_trace("CMD: %s\n", cmd);
    system(cmd);
}

void tc_block_all()
{
    char cmd[2048];
    snprintf(cmd, 2048, "tc filter add dev %s protocol all parent ffff: prio 65535 basic match \"u32(u16 0x4305 0xffff at -2)\" flowid :1 action drop", g_interface);
    log_trace("CMD: %s\n", cmd);
    system(cmd);
}

void tc_allow_mac(const uint8_t mac[], uint8_t prio)
{
    char cmd[2048];
    char mac32[9];
    char mac16[5];
    snprintf(mac32, 9, "%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3]);
    snprintf(mac16, 5, "%02x%02x", mac[4], mac[5]);
    snprintf(cmd, 2048, "tc filter add dev %s protocol all parent ffff: prio %d "
             "basic match \"u32(u32 0x%s 0xffffffff at -8)\" "
             "and \"u32(u16 0x%s 0xffff at -4)\" flowid :1 action pass",
             g_interface, prio, mac32, mac16);
    log_trace("CMD: %s\n", cmd);
    system(cmd);
}

void tc_disallow_mac(const uint8_t mac[], uint8_t prio)
{
    char cmd[2048];
    char mac32[9];
    char mac16[5];
    snprintf(mac32, 9, "%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3]);
    snprintf(mac16, 5, "%02x%02x", mac[4], mac[5]);
    snprintf(cmd, 2048, "tc filter delete dev %s protocol all parent ffff: prio %d "
             "basic match \"u32(u32 0x%s 0xffffffff at -8)\" "
             "and \"u32(u16 0x%s 0xffff at -4)\" flowid :1 action pass",
             g_interface, prio, mac32, mac16);
    log_trace("CMD: %s\n", cmd);
    system(cmd);
}

void tc_start()
{
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
}
