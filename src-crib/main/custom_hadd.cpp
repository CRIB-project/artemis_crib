#include "Compression.h"
#include "ROOT/StringConv.hxx"
#include "ROOT/TIOFeatures.hxx"
#include "TClass.h"
#include "TFile.h"
#include "THashList.h"
#include "TKey.h"
#include "TSystem.h"
#include "TUUID.h"
#include "snprintf.h"
#include <ROOT/RConfig.hxx>

#include <climits>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "TFileMerger.h"
#ifndef R__WIN32
#include "ROOT/TProcessExecutor.hxx"
#endif

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
    if (argc < 3 || "-h" == std::string(argv[1]) || "--help" == std::string(argv[1])) {
        std::cerr << "\nusage: hadd [-a] [-f] [-f[0-9]] [-fk] [-ff] [-k] [-O] [-T] [-v V] "
                  << "[-j J] [-dbg] [-d D] [-n N] [-cachesize CACHESIZE] [-experimental-io-features "
                  << "EXPERIMENTAL_IO_FEATURES] TARGET SOURCES \n\n"
                  << "This program will add histograms, trees and other objects from a list\n"
                  << "of ROOT files and write them to a target ROOT file. The target file is\n"
                  << "newly created and must not exist, or if -f (\" force \") is given, must\n"
                  << "not be one of the source files.\n\n"
                  << "It is copied from ROOT source file and linked artemis library\n";
        return (argc == 2 && ("-h" == std::string(argv[1]) || "--help" == std::string(argv[1])))
                   ? 0
                   : 1;
    }

    ROOT::TIOFeatures features;
    Bool_t append = kFALSE;
    Bool_t force = kFALSE;
    Bool_t skip_errors = kFALSE;
    Bool_t reoptimize = kFALSE;
    Bool_t noTrees = kFALSE;
    Bool_t keepCompressionAsIs = kFALSE;
    Bool_t useFirstInputCompression = kFALSE;
    Bool_t multiproc = kFALSE;
    Bool_t debug = kFALSE;
    Int_t maxopenedfiles = 0;
    Int_t verbosity = 99;
    TString cacheSize;
    SysInfo_t s;
    gSystem->GetSysInfo(&s);
    auto nProcesses = s.fCpus;
    auto workingDir = gSystem->TempDirectory();
    int outputPlace = 0;
    int ffirst = 2;
    Int_t newcomp = -1;
    for (int a = 1; a < argc; ++a) {
        if (strcmp(argv[a], "-T") == 0) {
            noTrees = kTRUE;
            ++ffirst;
        } else if (strcmp(argv[a], "-a") == 0) {
            append = kTRUE;
            ++ffirst;
        } else if (strcmp(argv[a], "-f") == 0) {
            force = kTRUE;
            ++ffirst;
        } else if (strcmp(argv[a], "-k") == 0) {
            skip_errors = kTRUE;
            ++ffirst;
        } else if (strcmp(argv[a], "-O") == 0) {
            reoptimize = kTRUE;
            ++ffirst;
        } else if (strcmp(argv[a], "-dbg") == 0) {
            debug = kTRUE;
            verbosity = kTRUE;
            ++ffirst;
        } else if (strcmp(argv[a], "-d") == 0) {
            if (a + 1 != argc && argv[a + 1][0] != '-') {
                if (gSystem->AccessPathName(argv[a + 1])) {
                    std::cerr << "Error: could not access the directory specified: " << argv[a + 1]
                              << ". We will use the system's temporal directory.\n";
                } else {
                    workingDir = argv[a + 1];
                }
                ++a;
                ++ffirst;
            } else {
                std::cout << "-d: no directory specified.  We will use the system's temporal directory.\n";
            }
            ++ffirst;
        } else if (strcmp(argv[a], "-j") == 0) {
            // If the number of processes is not specified, use the default.
            if (a + 1 != argc && argv[a + 1][0] != '-') {
                // number of processes specified
                Long_t request = 1;
                for (char *c = argv[a + 1]; *c != '\0'; ++c) {
                    if (!isdigit(*c)) {
                        // Wrong number of Processes. Use the default:
                        std::cerr << "Error: could not parse the number of processes to run in parallel passed after -j: "
                                  << argv[a + 1] << ". We will use the system maximum.\n";
                        request = 0;
                        break;
                    }
                }
                if (request == 1) {
                    request = strtol(argv[a + 1], 0, 10);
                    if (request < kMaxLong && request >= 0) {
                        nProcesses = (Int_t)request;
                        ++a;
                        ++ffirst;
                        std::cout << "Parallelizing  with " << nProcesses << " processes.\n";
                    } else {
                        std::cerr << "Error: could not parse the number of processes to use passed after -j: " << argv[a + 1]
                                  << ". We will use the default value (number of logical cores).\n";
                    }
                }
            }
            multiproc = kTRUE;
            ++ffirst;
        } else if (strcmp(argv[a], "-cachesize=") == 0) {
            int size;
            static constexpr size_t arglen = std::char_traits<char>::length("-cachesize=");
            auto parseResult = ROOT::FromHumanReadableSize(argv[a] + arglen, size);
            if (parseResult == ROOT::EFromHumanReadableSize::kParseFail) {
                std::cerr << "Error: could not parse the cache size passed after -cachesize: "
                          << argv[a + 1] << ". We will use the default value.\n";
            } else if (parseResult == ROOT::EFromHumanReadableSize::kOverflow) {
                double m;
                const char *munit = nullptr;
                ROOT::ToHumanReadableSize(INT_MAX, false, &m, &munit);
                std::cerr << "Error: the cache size passed after -cachesize is too large: "
                          << argv[a + 1] << " is greater than " << m << munit
                          << ". We will use the default value.\n";
            } else {
                cacheSize = "cachesize=";
                cacheSize.Append(argv[a] + 1);
            }
            ++ffirst;
        } else if (strcmp(argv[a], "-cachesize") == 0) {
            if (a + 1 >= argc) {
                std::cerr << "Error: no cache size number was provided after -cachesize.\n";
            } else {
                int size;
                auto parseResult = ROOT::FromHumanReadableSize(argv[a + 1], size);
                if (parseResult == ROOT::EFromHumanReadableSize::kParseFail) {
                    std::cerr << "Error: could not parse the cache size passed after -cachesize: "
                              << argv[a + 1] << ". We will use the default value.\n";
                } else if (parseResult == ROOT::EFromHumanReadableSize::kOverflow) {
                    double m;
                    const char *munit = nullptr;
                    ROOT::ToHumanReadableSize(INT_MAX, false, &m, &munit);
                    std::cerr << "Error: the cache size passed after -cachesize is too large: "
                              << argv[a + 1] << " is greater than " << m << munit
                              << ". We will use the default value.\n";
                    ++a;
                    ++ffirst;
                } else {
                    cacheSize = "cachesize=";
                    cacheSize.Append(argv[a + 1]);
                    ++a;
                    ++ffirst;
                }
            }
            ++ffirst;
        } else if (!strcmp(argv[a], "-experimental-io-features")) {
            if (a + 1 >= argc) {
                std::cerr << "Error: no IO feature was specified after -experimental-io-features; ignoring\n";
            } else {
                std::stringstream ss;
                ss.str(argv[++a]);
                ++ffirst;
                std::string item;
                while (std::getline(ss, item, ',')) {
                    if (!features.Set(item)) {
                        std::cerr << "Ignoring unknown feature request: " << item << std::endl;
                    }
                }
            }
            ++ffirst;
        } else if (strcmp(argv[a], "-n") == 0) {
            if (a + 1 >= argc) {
                std::cerr << "Error: no maximum number of opened was provided after -n.\n";
            } else {
                Long_t request = strtol(argv[a + 1], 0, 10);
                if (request < kMaxLong && request >= 0) {
                    maxopenedfiles = (Int_t)request;
                    ++a;
                    ++ffirst;
                } else {
                    std::cerr << "Error: could not parse the max number of opened file passed after -n: " << argv[a + 1] << ". We will use the system maximum.\n";
                }
            }
            ++ffirst;
        } else if (strcmp(argv[a], "-v") == 0) {
            if (a + 1 == argc || argv[a + 1][0] == '-') {
                // Verbosity level was not specified use the default:
                verbosity = 99;
                //         if (a+1 >= argc) {
                //            std::cerr << "Error: no verbosity level was provided after -v.\n";
            } else {
                Bool_t hasFollowupNumber = kTRUE;
                for (char *c = argv[a + 1]; *c != '\0'; ++c) {
                    if (!isdigit(*c)) {
                        // Verbosity level was not specified use the default:
                        hasFollowupNumber = kFALSE;
                        break;
                    }
                }
                if (hasFollowupNumber) {
                    Long_t request = strtol(argv[a + 1], 0, 10);
                    if (request < kMaxLong && request >= 0) {
                        verbosity = (Int_t)request;
                        ++a;
                        ++ffirst;
                    } else {
                        verbosity = 99;
                        std::cerr << "Error: could not parse the verbosity level passed after -v: " << argv[a + 1] << ". We will use the default value (99).\n";
                    }
                }
            }
            ++ffirst;
        } else if (argv[a][0] == '-') {
            bool farg = false;
            if (force && argv[a][1] == 'f') {
                // Bad argument
                std::cerr << "Error: Using option " << argv[a] << " more than once is not supported.\n";
                ++ffirst;
                farg = true;
            }
            const char *prefix = "";
            if (argv[a][1] == 'f' && argv[a][2] == 'k') {
                farg = true;
                force = kTRUE;
                keepCompressionAsIs = kTRUE;
                prefix = "k";
            }
            if (argv[a][1] == 'f' && argv[a][2] == 'f') {
                farg = true;
                force = kTRUE;
                useFirstInputCompression = kTRUE;
                if (argv[a][3] != '\0') {
                    std::cerr << "Error: option -ff should not have any suffix: " << argv[a] << " (suffix has been ignored)\n";
                }
            }
            char ft[7];
            for (int alg = 0; !useFirstInputCompression && alg <= 5; ++alg) {
                for (int j = 0; j <= 9; ++j) {
                    const int comp = (alg * 100) + j;
                    snprintf(ft, 7, "-f%s%d", prefix, comp);
                    if (!strcmp(argv[a], ft)) {
                        farg = true;
                        force = kTRUE;
                        newcomp = comp;
                        break;
                    }
                }
            }
            if (!farg) {
                // Bad argument
                std::cerr << "Error: option " << argv[a] << " is not a supported option.\n";
            }
            ++ffirst;
        } else if (!outputPlace) {
            outputPlace = a;
        }
    }

    gSystem->Load("libTreePlayer");

    const char *targetname = 0;
    if (outputPlace) {
        targetname = argv[outputPlace];
    } else {
        targetname = argv[ffirst - 1];
    }

    if (verbosity > 1) {
        std::cout << "chadd Target file: " << targetname << std::endl;
    }

    TFileMerger fileMerger(kFALSE, kFALSE);
    fileMerger.SetMsgPrefix("chadd");
    fileMerger.SetPrintLevel(verbosity - 1);
    if (maxopenedfiles > 0) {
        fileMerger.SetMaxOpenedFiles(maxopenedfiles);
    }
    // The following section will collect all input filenames into a vector,
    // including those listed within an indirect file.
    // If any file can not be accessed, it will error out, unless skip_errors is true
    std::vector<std::string> allSubfiles;
    for (int a = ffirst; a < argc; ++a) {
        if (a == outputPlace)
            continue;
        if (argv[a] && argv[a][0] == '@') {
            std::ifstream indirect_file(argv[a] + 1);
            if (!indirect_file.is_open()) {
                std::cerr << "chadd could not open indirect file " << (argv[a] + 1) << std::endl;
                if (!skip_errors)
                    return 1;
            } else {
                std::string line;
                while (indirect_file) {
                    if (std::getline(indirect_file, line) && line.length()) {
                        if (gSystem->AccessPathName(line.c_str(), kReadPermission) == kTRUE) {
                            std::cerr << "chadd could not validate the file name \"" << line << "\" within indirect file "
                                      << (argv[a] + 1) << std::endl;
                            if (!skip_errors)
                                return 1;
                        } else
                            allSubfiles.emplace_back(line);
                    }
                }
            }
        } else {
            const std::string line = argv[a];
            if (gSystem->AccessPathName(line.c_str(), kReadPermission) == kTRUE) {
                std::cerr << "chadd could not validate argument \"" << line << "\" as input file " << std::endl;
                if (!skip_errors)
                    return 1;
            } else
                allSubfiles.emplace_back(line);
        }
    }
    if (allSubfiles.empty()) {
        std::cerr << "chadd could not find any valid input file " << std::endl;
        return 1;
    }
    // The next snippet determines the output compression if unset
    if (newcomp == -1) {
        if (useFirstInputCompression || keepCompressionAsIs) {
            // grab from the first file.
            TFile *firstInput = TFile::Open(allSubfiles.front().c_str());
            if (firstInput && !firstInput->IsZombie())
                newcomp = firstInput->GetCompressionSettings();
            else
                newcomp = ROOT::RCompressionSetting::EDefaults::kUseCompiledDefault;
            delete firstInput;
            fileMerger.SetMergeOptions(TString("first_source_compression"));
        } else {
            newcomp = ROOT::RCompressionSetting::EDefaults::kUseCompiledDefault;
            fileMerger.SetMergeOptions(TString("default_compression"));
        }
    }
    if (verbosity > 1) {
        if (keepCompressionAsIs && !reoptimize)
            std::cout << "chadd compression setting for meta data: " << newcomp << '\n';
        else
            std::cout << "chadd compression setting for all output: " << newcomp << '\n';
    }
    if (append) {
        if (!fileMerger.OutputFile(targetname, "UPDATE", newcomp)) {
            std::cerr << "chadd error opening target file for update :" << argv[ffirst - 1] << "." << std::endl;
            exit(2);
        }
    } else if (!fileMerger.OutputFile(targetname, force, newcomp)) {
        std::cerr << "chadd error opening target file (does " << argv[ffirst - 1] << " exist?)." << std::endl;
        if (!force)
            std::cerr << "Pass \"-f\" argument to force re-creation of output file." << std::endl;
        exit(1);
    }

    auto step = (allSubfiles.size() + nProcesses - 1) / nProcesses;
    if (multiproc && step < 3) {
        // At least 3 files per process
        step = 3;
        nProcesses = (allSubfiles.size() + step - 1) / step;
        std::cout << "Each process should handle at least 3 files for efficiency.";
        std::cout << " Setting the number of processes to: " << nProcesses << std::endl;
    }
    if (nProcesses == 1)
        multiproc = kFALSE;

    std::vector<std::string> partialFiles;

#ifndef R__WIN32
    // this is commented out only to try to prevent false positive detection
    // from several anti-virus engines on Windows, and multiproc is not
    // supported on Windows anyway
    if (multiproc) {
        auto uuid = TUUID();
        auto partialTail = uuid.AsString();
        for (auto i = 0; (i * step) < allSubfiles.size(); i++) {
            std::stringstream buffer;
            buffer << workingDir << "/partial" << i << "_" << partialTail << ".root";
            partialFiles.emplace_back(buffer.str());
        }
    }
#endif

    auto mergeFiles = [&](TFileMerger &merger) {
        if (reoptimize) {
            merger.SetFastMethod(kFALSE);
        } else {
            if (!keepCompressionAsIs && merger.HasCompressionChange()) {
                // Don't warn if the user explicitly requested re-optimization.
                std::cout << "chadd Sources and Target have different compression settings\n";
                std::cout << "chadd merging will be slower" << std::endl;
            }
        }
        merger.SetNotrees(noTrees);
        merger.SetMergeOptions(TString(merger.GetMergeOptions()) + " " + cacheSize);
        merger.SetIOFeatures(features);
        Bool_t status;
        if (append)
            status = merger.PartialMerge(TFileMerger::kIncremental | TFileMerger::kAll);
        else
            status = merger.Merge();
        return status;
    };

    auto sequentialMerge = [&](TFileMerger &merger, int start, int nFiles) {
        for (auto i = start; i < (start + nFiles) && i < static_cast<int>(allSubfiles.size()); i++) {
            if (!merger.AddFile(allSubfiles[i].c_str())) {
                if (skip_errors) {
                    std::cerr << "chadd skipping file with error: " << allSubfiles[i] << std::endl;
                } else {
                    std::cerr << "chadd exiting due to error in " << allSubfiles[i] << std::endl;
                    return kFALSE;
                }
            }
        }
        return mergeFiles(merger);
    };

    auto parallelMerge = [&](int start) {
        TFileMerger mergerP(kFALSE, kFALSE);
        mergerP.SetMsgPrefix("chadd");
        mergerP.SetPrintLevel(verbosity - 1);
        if (maxopenedfiles > 0) {
            mergerP.SetMaxOpenedFiles(maxopenedfiles / nProcesses);
        }
        if (!mergerP.OutputFile(partialFiles[start / step].c_str(), newcomp)) {
            std::cerr << "chadd error opening target partial file" << std::endl;
            exit(1);
        }
        return sequentialMerge(mergerP, start, step);
    };

    auto reductionFunc = [&]() {
        for (const auto &pf : partialFiles) {
            fileMerger.AddFile(pf.c_str());
        }
        return mergeFiles(fileMerger);
    };

    Bool_t status;

#ifndef R__WIN32
    if (multiproc) {
        ROOT::TProcessExecutor p(nProcesses);
        auto res = p.Map(parallelMerge, ROOT::TSeqI(0, allSubfiles.size(), step));
        status = std::accumulate(res.begin(), res.end(), 0U) == partialFiles.size();
        if (status) {
            status = reductionFunc();
        } else {
            std::cout << "chadd failed at the parallel stage" << std::endl;
        }
        if (!debug) {
            for (const auto &pf : partialFiles) {
                gSystem->Unlink(pf.c_str());
            }
        }
    } else {
        status = sequentialMerge(fileMerger, 0, allSubfiles.size());
    }
#else
    status = sequentialMerge(fileMerger, 0, allSubfiles.size());
#endif

    if (status) {
        if (verbosity == 1) {
            std::cout << "chadd merged " << allSubfiles.size() << " (" << fileMerger.GetMergeList()->GetEntries()
                      << ") input (partial) files into " << targetname << ".\n";
        }
        return 0;
    } else {
        if (verbosity == 1) {
            std::cout << "chadd failure during the merge of " << allSubfiles.size() << " ("
                      << fileMerger.GetMergeList()->GetEntries() << ") input (partial) files into " << targetname << ".\n";
        }
        return 1;
    }
}
