#include "PUEF.hpp"


int main(int argc, char *argv[])
{
    threadsafe_queue<::std::pair<::std::string, ::std::string>> results;

    Spider spdr("www.gmw.cn", "www.gmw.cn");

    ::std::vector<::std::thread> threads;

    // main thread
    threads.emplace_back([&] { spdr.run(results); });

    // sub thread
    for (int i = 0; i < 5; ++i) threads.emplace_back([&] 
    {
        ::std::pair<::std::string, ::std::string> result;

        while (!spdr.finish() || !results.is_empty())
        {
            if (results.try_pop(result))
            {
                Serialization::write2file(Parser::parser(result), "./results/");
            }
        }
    });

    for (auto &td : threads) td.join();

    return 0;
}