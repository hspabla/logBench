#include <stdio.h>
#include <string>
#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sstream>

namespace {

   class OptionParser {
      public:
         OptionParser(int& argc, char**&argv)
            : argc(argc)
            , argv(argv)
            , cluster("logcabin:5254")
            , readWriteOption(1)
            , numberOfProcesses(1)
            , sameOrDiffFile(1)
            , numberOfReadWrites(10)
         {
            while(true) {
               static struct option longOptions[] = {
                  {"cluster",  required_argument, NULL, 'c'},
                  {"readWriteOption", required_argument, NULL, 'w'},
                  {"sameOrDiffFile", required_argument, NULL, 's'},
                  {"numberOfReadWrites", required_argument, NULL, 'o'},
                  {"numberOfProcesses", required_argument, NULL, 'n'},
                  {"help",  no_argument, NULL, 'h'},
                  {0, 0, 0, 0}
               };
               int c = getopt_long(argc, argv, "c:w:n:h", longOptions, NULL);

               // Detect the end of the options.
               if (c == -1)
                   break;

               switch (c) {
                   case 'c':
                       cluster = optarg;
                       break;
                   case 'w':
                       readWriteOption = atoi(optarg);
                       break;
                   case 's':
                       sameOrDiffFile = atoi(optarg);
                       break;
                   case 'o':
                       numberOfReadWrites = atoi(optarg);
                       break;
                   case 'n':
                       numberOfProcesses = atoi(optarg);
                       break;
                   case 'h':
                       usage();
                       exit(0);
                   default:
                       usage();
                       exit(1);
               }
           }
       }


       void usage() {
          std::cout
             << "Runs read and write clients."
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
             << "[default: logcabin:5254]"
             << std::endl
             << "                                "

             << "  -w, --readWriteOption         "
             << "1 for Read Only, 2 for Write Only, 3 for Read and Write 50-50%"
             << std::endl
             << "  -n, --numberOfProcesses           "
             << std::endl

             << "  -h, --help                    "
             << "Print this usage information"
             << std::endl;
       }

      int& argc;
      char**& argv;
      std::string cluster;
      int readWriteOption;
      int numberOfProcesses;
      int sameOrDiffFile;
      int numberOfReadWrites;
   };

}  // anonymous namespace


void execute(std::string cmd, char **argv) {
   if (execvp(cmd.c_str(), argv)) {
      std::cout << "Program Execution failed :" << cmd << std::endl;
      exit(1);
   }
}


int main(int argc, char** argv)
{
      OptionParser options(argc, argv);
      int status;
      std::string readCmd = "build/Examples/ReadClient";
      std::string arg1 = "--cluster=" + options.cluster;
      std::string readArg2 = "--readOption=" + std::to_string(options.sameOrDiffFile);
      std::string readArg3 = "--numberOfReads=" + std::to_string(options.numberOfReadWrites);

      std::string writeCmd = "build/Examples/WriteClient";
      std::string writeArg2 = "--writeOption=" + std::to_string(options.sameOrDiffFile);
      std::string writeArg3 = "--numberOfWrites=" + std::to_string(options.numberOfReadWrites);

      char *readArgv[5];
      readArgv[0] = &readCmd[0u];
      readArgv[1] = &arg1[0u];
      readArgv[2] = &readArg2[0u];
      readArgv[3] = &readArg3[0u];
      readArgv[4] = NULL;

      char *writeArgv[5];
      writeArgv[0] = &writeCmd[0u];
      writeArgv[1] = &arg1[0u];
      writeArgv[2] = &writeArg2[0u];
      writeArgv[3] = &writeArg3[0u];
      writeArgv[4] = NULL;

      // This will run only read clients.
      if (options.readWriteOption == 1) {
         for(int i=0; i < options.numberOfProcesses; i++) {
            pid_t client = 0;
            client = fork();

            if (client == 0)
               execute(readCmd, readArgv);
         }
      }

      // This will run only write clients.
      else if (options.readWriteOption == 2) {
         for(int i=0; i < options.numberOfProcesses; i++) {
            pid_t client = 0;
            client = fork();

            if (client == 0)
                execute(writeCmd, writeArgv);
         }
      }

      // This will run read and write clients 50-50%.
      else if (options.readWriteOption == 3) {
         for(int i=1; i <= options.numberOfProcesses; i++) {
            pid_t client = 0;
            client = fork();

            if (client == 0) {
               if (i%2 == 0)
                  execute(readCmd, readArgv);
               else
                  execute(writeCmd, writeArgv);
            }
         }
      }

      // Wait for all processes to finish.
      for (int i=0; i<options.numberOfProcesses; i++) {
          wait(&status);
      }
      return 0;
}

