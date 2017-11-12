#include <string>

class macNockClient
{
public:
    macNockClient(const std::string &interface, const std::string &code);
    bool run();
    void stop();

private:

private:
    const std::string &m_interface;
    const std::string &m_hood;
    bool m_stop;
};
