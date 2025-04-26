#include <NetCommon/FwNet.h>

class Ui
{
  public:

    constexpr std::vector<std::array<char, fw::net::array_size>> get_messages() const
    {
        return messages_;
    }

    void append_msg(const std::array<char, fw::net::array_size>& message)
    {
        messages_.push_back(message);
    }

  private:
    std::vector<std::array<char, fw::net::array_size>> messages_;
    std::vector<std::string> clients_list_;
};