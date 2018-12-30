#include "PUEF.hpp"


int main(int argc, char *argv[])
{
    /* 

    threadsafe_queue<Result> results;

    Spider spdr("www.gmw.cn", "www.gmw.cn");
    
    // multi threads
    // define spdr.finish() for subthread

    // main thread
    {
        spdr.run(results);
    }

    // sub thread
    {
        Result result;

        while (!spdr.finish() || !results.is_empty())
        {
            if (results.try_pop(result))
            {
                auto res = Parser::parser(result);
                Serialization::write2file(res, "./");
            }
        }
    }

    */

    return 0;
}