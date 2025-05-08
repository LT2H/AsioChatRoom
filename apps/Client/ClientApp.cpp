#include "Client.h"
#include "Ui.h"

int main()
{
    Client client{};

    Ui ui{};

    ui.init();

    ui.render_loop(client);

    return 0;
}