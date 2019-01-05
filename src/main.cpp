#include "PUEF.hpp"


int main(int argc, char *argv[])
{
    threadsafe_queue<::std::pair<::std::string, ::std::string>> results;

    Spider spdr("www.gmw.cn", "www.gmw.cn");

    ::std::vector<::std::thread> threads;

    // request thread
    threads.emplace_back([&] { spdr.run(results); });

    // parser thread
    threads.emplace_back([&] 
    {
        ::std::pair<::std::string, ::std::string> result;

        while (!spdr.finish() || !results.is_empty())
        {
            if (results.try_pop(result))
            {
                Serialization::obj2file(Parser::parser(result), "./results/");
            }
        }
    });

    for (auto &td : threads) td.join();

    return 0;
}