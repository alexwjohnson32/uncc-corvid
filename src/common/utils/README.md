# `utils`

This is a directory for truly common utlities. If it would be useable for a "smart" coffee maker, it belongs here.

## Linking to the library in CMake

NOTE: This is all done under the assumption of static linkage. If we have shared libraries, some of this guidance may change, or may require extra steps. As of the time of this writing though, we only build statically.

The `utils` directory builds a library that can be referenced throughout CMake by the variable `CORVID_UTILS_LIB` (referenced in CMake like `${CORVID_UTILS_LIB}`). This library publically exposes all of the headers and keeps private the sources. It also publically links and includes Boost 1.78. So if you want to link to boost, simply link to this library and you have it.

## Using the library in C++

All classes and functions found within this library exist under the `common::utils::` namespace.

When including these files in your file, be sure to give the fully qualified path (example, `#include "common/utils/stopwatch.hpp"`).

We will briefly describe each file, but this is not an API document. If you want to see something more along those lines, look at the header files themselves. This is a purpose overview.

### `stopwatch.hpp`

Its a stopwatch utility. It works by taking a timestamp at initialization (or a new timestamp at `Start()`) and diffs it with the current timestamp when you call `EllapsedMIlliseconds()`. This is a header only file.

### `json_templates.hpp`

It contains a number of boost json utility helpers, in order to easily utilize the boost/json libary by serializing and deserializing objects to strings and files, as well as providing the extraction tools necessary to make writing new serializable classes simpler. This is a header only file.

### `websocket_client.hpp`

Provides a robust implementation of a Boost/Beast WebSocket client. There is a proper usage of the client, but that is not immediately noticable by those who don't know. After we go over the `common::utils::LocalLogHelper` class, we will give a brief example that shows how to use both classes together.

### `local_log_helper.hpp`

A logging utility. While using the HELICS runner, I found that `std::cout` statements were not helpful since the HELICS run utlity launces everything as subprocesses. So we need a tool that we could treat like `std::cout` but wrote to a file. I also found that I would like to be able to send those same messages to a websocket client (or some other object) without having to write the same message twice. Thus, this logging class was born.

It is instanced, and writes directly to a file. If the OnWrite Callback has been set, it also will send the message there as well.

There was one problem though. `std::cout` is hyper convient with the `<<` notation to just shove strings into the buffer without needing an intermediate stringstream object (or string object). Using Gemini, we got the functionality working so that if you have an instance of `common::utils::LocalLogHelper log` you can simply write to it like you would a standard stream: `log << "INFO: " << obj_str << std::endl;`.

There is a slight discrepancy with the current implementation. When you use the stream notation, the OnWrite callback is called only after the full stream has been read. So you get one callback call per "logical" string. However, the log file is written to at each step. This behavior needs to be addressed.

#### Why does the `common::utils::LocalLogHelper::LogStream` class work?

In order to have that sweet syntactic sugar that treats the object as a streamable object, we take advantage of an inner public class called `common::utils::LocalLogHelper::LogStream`. Basically, everytime we call the `<<` operator, it instantiates an instance of `common::utils::LocalLogHelper::LogStream`. This instance holds a reference to the `common::utils::LocalLogHelper` instance, referred to as `m_parent`. Each operator call call's the parent's append function, which writes to buffer stream and to the output file. After every object is called, they begin to go out of scope and the destructor is called per object. For most of the destructor calls, nothing happens. But for the final destructor called, it calls the parent's `FlushToCallback` which is when the OnWrite callback is called, and the formatting string is reset.

## Example Usage of the `common::utils::LocalLogHelper` and `common::utils::WebSocketClient`

The current version of the `QueryableFederate` utlizes both classes together, and it illustrates a correct useage of both tools.

```C++

// Because of how the WebSocketClient is implemented for safety, you must instantiate
// a std::shared_ptr of the WebSocketClient
m_client = std::make_shared<common::utils::WebSocketClient>();

// Configure the client
// Note how this sets different callbacks on the client, and those callbacks utilize the
// LocalLogHelper (which as you will see actually calls the Send function of the client).
m_client->SetOnMessage([&log = this->m_log](const std::string &msg) { log << "Received: " << msg << std::endl; });
m_client->SetOnError([&log = this->m_log](const boost::system::error_code &ec, const std::string &what)
                    { log << what << ": " << ec.message() << std::endl; });

// This is what launches the client. There is a BlockingRun call if you wish to manually manage
// the thread the client is run within.
m_client->AsyncRun();
// The connect call, the lambda expression is just there to help failure cases and logging.
m_client->Connect(m_query_fed_input.client_details.host, m_query_fed_input.client_details.port,
                m_query_fed_input.client_details.target,
                [&log = this->m_log](const boost::system::error_code &ec)
                {
                    if (ec)
                    {
                        std::stringstream err_str;
                        err_str << "Error Connecting: " << ec.message() << "\n";
                        log << err_str.str();
                        throw std::runtime_error(err_str.str());
                    }
                    else
                    {
                        log << "Connected!\n";
                    }
                });

// This is where the log gets a reference to the client. This will work as expected as long as
// the lifetime of the client is within the lifetime of the logger.
m_log.SetOnWriteCallback([&client = this->m_client](const std::string &msg) { client->Send(msg); });

```