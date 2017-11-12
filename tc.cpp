#include "tc.h"
#include "log.h"

#include <string>

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

tc::tc(const std::string &interface)
    : m_interface(interface)
{
    del_qdisc_ingress(); // in case a old session is sill there

    add_qdisc_ingress();
    block_all();
}

tc::~tc()
{
    del_qdisc_ingress();
}

void tc::add_qdisc_ingress()
{
    const std::string cmd{"/sbin/tc qdisc add dev "+m_interface+" ingress"};
    log::debug(std::string("CMD: ") + cmd);
    std::system(cmd.c_str());
}

void tc::del_qdisc_ingress()
{
    const std::string cmd{"/sbin/tc qdisc del dev "+m_interface+" ingress"};
    log::debug(std::string("CMD: ") + cmd);
    std::system(cmd.c_str());
}

void tc::block_all()
{
    const std::string cmd{"/sbin/tc filter add dev "+m_interface+" protocol all parent ffff: prio 65535 basic match \"u32(u16 0x4305 0xffff at -2)\" flowid :1 action drop"};
    log::debug(std::string("CMD: ") + cmd);
    std::system(cmd.c_str());
}

void tc::allow_mac(const std::string &mac)
{
    const std::string cmd{"/sbin/tc filter add dev "+m_interface+" protocol all parent ffff: prio 99 "
                                                           "basic match \"u32(u32 0x"+mac.substr(0, 8)+" 0xffffffff at -8)\" "
                                                           "and \"u32(u16 0x"+mac.substr(8,10)+" 0xffff at -4)\" flowid :1 action pass"};
    log::debug(std::string("CMD: ") + cmd);
    std::system(cmd.c_str());
}

void tc::disallow_mac(const std::string &mac)
{
    const std::string cmd{"/sbin/tc filter delete dev "+m_interface+" protocol all parent ffff: prio 99 "
                                                           "basic match \"u32(u32 0x"+mac.substr(0, 8)+" 0xffffffff at -8)\" "
                                                           "and \"u32(u16 0x"+mac.substr(8,10)+" 0xffff at -4)\" flowid :1 action pass"};
    log::debug(std::string("CMD: ") + cmd);
    std::system(cmd.c_str());
}
