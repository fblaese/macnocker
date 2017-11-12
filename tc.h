#ifndef _TC_H
#define _TC_H

#include <string>

class tc
{
public:
    explicit tc(const std::string &interface);

    ~tc();

    void add_qdisc_ingress();

    void del_qdisc_ingress();

    void block_all();

    void allow_mac(const std::string &mac);

    void disallow_mac(const std::string &mac);

private:
    tc(const tc&);
    tc();

    const std::string m_interface;
};

#endif // _TC_H
