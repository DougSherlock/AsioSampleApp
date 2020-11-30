//---------------------------------------------------------------------------------------------------
// Reference:   https://www.youtube.com/watch?v=2hNdkYInj4g&t=1599s
// Title:       Networking in C++ Part #1 MMO Client/Server, ASIO & Framework Basics by 
// I reached 21:45 of this tutorial
//---------------------------------------------------------------------------------------------------

#include <iostream>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

using namespace std;

std::vector<char> vBuffer(20 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket)
{
    socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
        [&](std::error_code ec, size_t length)
        {
            if (!ec)
            {
                cout << "\n\nRead " << length << " bytes\n\n";
                for (int i = 0; i < length; i++)
                {
                    cout << vBuffer[i];
                }

                GrabSomeData(socket);
            }
        }
    );
}

int main()
{
    asio::error_code ec;

    // create a "context" - essentially the platform-specific interface
    asio::io_context context;

    // give some fake task to asio so the context doesn't exit before we use it
    asio::io_context::work idleWork(context);

    // start the context in its own thread so it doesn't block the main thread
    std::thread thrContext = std::thread([&]() { context.run(); });

    // get the address we want to connect to
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80); //"93.184.216.34"

    // create the socket, the context will deliver the implementation
    asio::ip::tcp::socket socket(context);

    // tell socket to try and connect
    socket.connect(endpoint, ec);

    if (!ec)
    {
        cout << "Connected!" << endl;
    }
    else
    {
        cout << "Failed to connect to address:\n" << ec.message() << endl;
    }

    if (socket.is_open())
    {
        // prime the asio context to read some data when it's available
        GrabSomeData(socket);

        string request = "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n";

        // basically execute an HTTP get command
        socket.write_some(asio::buffer(request.data(), request.size()), ec);
 
        // program does something else while asio handles data transfer in the background
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(20000ms);

        context.stop();
        if (thrContext.joinable())
        {
            thrContext.join();
        }
    }

    system("pause");
    return 0;
}

