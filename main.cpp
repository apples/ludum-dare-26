#include "game.hpp"
#include "meta.hpp"

#include "inugami/exception.hpp"

#include <fstream>
#include <iostream>
#include <exception>
#include <functional>

//#include <audiere.h>

using namespace Inugami;
//using namespace audiere;
using namespace std;

void dumpProfiles();

int main()
{
    profiler = new Profiler();

    std::ofstream logfile("log.txt");
    logger = new Logger<5>(logfile);

    try
    {
        Game::getInstance().start();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        std::ofstream("error.txt") << e.what();
        return -1;
    }

    //dumpProfiles();

    return 0;
}

void dumpProfiles()
{
    using Prof = Inugami::Profiler::Profile;

    std::ofstream pfile("profile.txt");

    auto all = profiler->getAll();

    std::function<void(const Prof*const&, std::string)> dumProf;
    dumProf = [&pfile, &dumProf](const Prof*const& in, std::string indent)
    {
        pfile << indent << "Min: " << in->min << "\n";
        pfile << indent << "Max: " << in->max << "\n";
        pfile << indent << "Avg: " << in->average << "\n";
        pfile << indent << "Num: " << in->samples << "\n\n";
        for (auto& p : in->getChildren())
        {
            pfile << indent << p.first << ":\n";
            dumProf(p.second, indent+"\t");
        }
    };

    for (auto& p : all)
    {
        pfile << p.first << ":\n";
        dumProf(p.second, "\t");
    }
}
