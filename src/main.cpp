#include "PUEF.hpp"


int main(int argc, char *argv[])
{
    threadsafe_queue<::std::string> results;

    Spider spdr("www.gmw.cn", "www.gmw.cn");

    ::std::vector<::std::thread> threads;

    // main thread
    threads.emplace_back([&] {spdr.run(results); });

    // sub thread
    for (int i = 0; i < 5; ++i) threads.emplace_back([&] 
    {
        ::std::string result;

        while (!spdr.finish() || !results.is_empty())
        {
            if (results.try_pop(result))
            {
                auto website = Parser::parser(result);
                Serialization::write2file(website, "./");
            }
        }
    });

    for (auto &td : threads) td.join();

    return 0;
}