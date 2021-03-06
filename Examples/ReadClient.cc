/* Copyright (c) 2012 Stanford University
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <cassert>
#include <getopt.h>
#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <cstdlib>
#include <string>

#include <LogCabin/Client.h>
#include <LogCabin/Debug.h>
#include <LogCabin/Util.h>

namespace {

using LogCabin::Client::Cluster;
using LogCabin::Client::Tree;
using LogCabin::Client::Util::parseNonNegativeDuration;

/**
 * Parses argv for the main function.
 */
class OptionParser {
  public:
    OptionParser(int& argc, char**& argv)
        : argc(argc)
        , argv(argv)
        , cluster("logcabin:5254")
        , logPolicy("")
        , timeout(parseNonNegativeDuration("0s"))
        , readOption(1)
        , numberOfReads(1)
    {
        while (true) {
            static struct option longOptions[] = {
               {"cluster",  required_argument, NULL, 'c'},
               {"readOption", required_argument, NULL, 'w'},
               {"numberOfReads", required_argument, NULL, 'n'},
               {"help",  no_argument, NULL, 'h'},
               {"timeout",  required_argument, NULL, 't'},
               {"verbose",  no_argument, NULL, 'v'},
               {"verbosity",  required_argument, NULL, 256},
               {0, 0, 0, 0}
            };
            int c = getopt_long(argc, argv, "c:w:n:t:hv", longOptions, NULL);

            // Detect the end of the options.
            if (c == -1)
                break;

            switch (c) {
                case 'c':
                    cluster = optarg;
                    break;
                case 'w':
                    readOption = atoi(optarg);
                    break;
                case 'n':
                    numberOfReads = atoi(optarg);
                    break;
                case 't':
                    timeout = parseNonNegativeDuration(optarg);
                    break;
                case 'h':
                    usage();
                    exit(0);
                case 'v':
                    logPolicy = "VERBOSE";
                    break;
                case 256:
                    logPolicy = optarg;
                    break;
                case '?':
                default:
                    // getopt_long already printed an error message.
                    usage();
                    exit(1);
            }
        }
    }

    void usage() {
        std::cout
            << "Writes a value to LogCabin. This isn't very useful on its own "
            << "but serves as a"
            << std::endl
            << "good starting point for more sophisticated LogCabin client "
            << "programs."
            << std::endl
            << std::endl
            << "This program is subject to change (it is not part of "
            << "LogCabin's stable API)."
            << std::endl
            << std::endl

            << "Usage: " << argv[0] << " [options]"
            << std::endl
            << std::endl

            << "Options:"
            << std::endl

            << "  -c <addresses>, --cluster=<addresses>  "
            << "Network addresses of the LogCabin"
            << std::endl
            << "                                         "
            << "servers, comma-separated"
            << std::endl
            << "                                         "
            << "[default: logcabin:5254]"
            << std::endl
            << "                                         "
            << "  -r, --readOption=<option>  "
            << "Enter 1 for reading from same file, 2 for different files, 3 for mix of same and different files"
            << std::endl
            << "                                "
            << "  -n, --numberOfReads=<number>  "
            << "Number of reads to be made"
            << std::endl

            << "  -h, --help                     "
            << "Print this usage information"
            << std::endl

            << "  -t <time>, --timeout=<time>    "
            << "Set timeout for individual operations"
            << std::endl
            << "                                 "
            << "(0 means wait forever) [default: 0s]"
            << std::endl

            << "  -v, --verbose                  "
            << "Same as --verbosity=VERBOSE"
            << std::endl

            << "  --verbosity=<policy>           "
            << "Set which log messages are shown."
            << std::endl
            << "                                 "
            << "Comma-separated LEVEL or PATTERN@LEVEL rules."
            << std::endl
            << "                                 "
            << "Levels: SILENT ERROR WARNING NOTICE VERBOSE."
            << std::endl
            << "                                 "
            << "Patterns match filename prefixes or suffixes."
            << std::endl
            << "                                 "
            << "Example: Client@NOTICE,Test.cc@SILENT,VERBOSE."
            << std::endl;
    }

    int& argc;
    char**& argv;
    std::string cluster;
    std::string logPolicy;
    uint64_t timeout;
    int readOption;
    int numberOfReads = 0;
};

} // anonymous namespace

int main(int argc, char** argv)
{
    try {
        OptionParser options(argc, argv);
        LogCabin::Client::Debug::setLogPolicy(
            LogCabin::Client::Debug::logPolicyFromString(
                options.logPolicy));
        Cluster cluster(options.cluster);
        Tree tree = cluster.getTree();
        tree.setTimeout(options.timeout);
        auto start = std::chrono::steady_clock::now();
        int numOfReads = options.numberOfReads;

        if(options.readOption == 1) {
           for(int i=0; i < numOfReads; i++) {
               //std::string contents = tree.readEx("/test/fileSame");
               tree.readEx("/test/fileSame");
               //std::cout << contents << std::endl;
           }
        } else if(options.readOption == 2) {
           for(int i=0; i< numOfReads; i++) {
               std::string fileNum = std::to_string(i);
               //std::string contents = tree.readEx("/test/fileDiff_" + fileNum);
               tree.readEx("/test/fileDiff_" + fileNum);
               //std::cout << contents << std::endl;
           }
        } else if(options.readOption == 3) {
           int randMax = (numOfReads > 10) ? (numOfReads / 10) : 1;
           for(int i=0; i< numOfReads; i++) {
               std::string fileNum = std::to_string(rand() % randMax + 1);
               tree.readEx("/test/fileSAndD_" + fileNum);
               //std::cout << contents << std::endl;
           }
        } else {
           std::cout << "Wrong input for readOption" << std::endl;
           exit(-1);
        }

        auto end = std::chrono::steady_clock::now();
        auto diff = end - start;

        std::cout << "Read time: "
                  << std::chrono::duration <double, std::milli> (diff).count()
                  << " ms" << std::endl;
        return 0;

    } catch (const LogCabin::Client::Exception& e) {
        std::cerr << "Exiting due to LogCabin::Client::Exception: "
                  << e.what()
                  << std::endl;
        exit(1);
    }
}
