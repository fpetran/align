// Copyright 2013 Florian Petran
#ifndef ALIGN_MKDICT_HH
#define ALIGN_MKDICT_HH

#include<cstdlib>

#include<iostream>
#include<fstream>

#include<vector>
#include<queue>
#include<map>
#include<string>
#include<utility>

#include<algorithm>
#include<thread>

#include"bi-sim.h"
#include"string_impl.h"
#include"align_config.h"

class Producer {
    public:
        Producer() : notified(false) {}
        std::condition_variable cv;
        std::mutex m;
        void notify();
        bool is_notified();
    protected:
        bool notified;
};

class File : public Producer {
    public:
        void notify();
        void open(const std::string& fname);
        std::vector<string_impl> words;
};

class Result : public Producer {
    public:
        explicit Result(const std::string& e,
                        const std::string& f);
        bool done;
        void add_line(const string_impl& line);
        string_impl get_line();
        bool empty();
        inline const string_impl& get_header() {
            return header;
        }
    private:
        string_impl header;
        string_impl dict_head(const char* e,
                              const char* f);
        std::queue<string_impl> lines;
};

class ResultSet : public Producer {
    public:
        void add_result(Result* result);
        Result* get_result();
        bool empty();
    private:
        std::queue<Result*> results;
};

typedef std::map<std::string, File*> FileSet;

void file_reader(const std::string& fname, const FileSet& files);

void fileset_processor(const std::string& e, const std::string& f,
                       const FileSet& files, ResultSet* results,
                       const int wordlength_threshold,
                       const bi_sim::num_ty cognate_threshold);

void results_outputter(ResultSet* results);

#endif // ALIGN_MKDICT_HH