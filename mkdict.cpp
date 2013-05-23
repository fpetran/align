// Copyright 2012-2013 Florian Petran
#include"mkdict.h"

#include<string>
#include<stdexcept>
#include<algorithm>

using std::string;
using std::ifstream;
using std::ofstream;
using std::mutex;

namespace {
inline bool is_cognate(const string_impl& w1, const string_impl& w2,
                       const string_size wordlength_threshold,
                       const bi_sim::num_ty cognate_threshold) {
    return (w1 == w2)
        || (std::min(w1.length(), w2.length()) > wordlength_threshold
            && bi_sim::bi_sim(w1, w2) >= cognate_threshold);
}
}

namespace DictionaryInducer {

//////////////////////// Producer /////////////////////////////////////////////

void Producer::notify() {
    this->notified = true;
    this->cv.notify_one();
}

bool Producer::is_notified() {
    return this->notified;
}

//////////////////////// File /////////////////////////////////////////////////

void File::notify() {
    this->notified = true;
    this->cv.notify_all();
}

void File::open(const string& fname) {
    string line;
    ifstream file;
    file.open(fname);
    if (!file.is_open())
        throw std::runtime_error(fname + " : File not found!");
    while (file >> line)
        this->words.push_back(line.c_str());
    std::sort(words.begin(), words.end());
    auto it = std::unique(words.begin(), words.end());
    this->words.resize(std::distance(words.begin(), it));
    file.close();
}

//////////////////////// Result ///////////////////////////////////////////////

Result::Result(const string& e, const string& f)
    : Producer(), done(false) {
    this->header = this->dict_head(e.c_str(), f.c_str());
    this->add_line(this->header);
}

void Result::add_line(const string_impl& line) {
    lines.push(line);
}

string_impl Result::get_line() {
    string_impl line = lines.front();
    lines.pop();
    notified = false;
    return line;
}

bool Result::empty() { return lines.empty(); }

string_impl Result::dict_head(const char* e, const char* f) {
    string_impl head = "### ";
    return head + e + " = " + f;
}

//////////////////////// ResultSet ////////////////////////////////////////////

void ResultSet::add_result(Result* r) {
    this->m.lock();
    results.push(r);
    this->m.unlock();
}

Result* ResultSet::get_result() {
    Result* r = results.front();
    results.pop();
    return r;
}

bool ResultSet::empty() { return results.empty(); }

//////////////////////// worker functions /////////////////////////////////////

mutex dict_cout_mutex;

void file_reader(const string& fname,
                 const FileSet& files) {
    dict_cout_mutex.lock();
    std::cout << "Reading file " << fname << "..." << std::endl;
    dict_cout_mutex.unlock();

    File* f = files.at(fname);
    std::lock_guard<mutex> lock(f->m);
    f->open(fname);
    f->notify();

    dict_cout_mutex.lock();
    std::cout << "...done reading " << fname << "." << std::endl;
    dict_cout_mutex.unlock();
}

void fileset_processor(const string& e_name, const string& f_name,
                       const FileSet& files, ResultSet* resultset,
                       const int wordlength_threshold,
                       const bi_sim::num_ty cognate_threshold) {
    dict_cout_mutex.lock();
    std::cout << "Processing pair "
              << e_name << " - " << f_name << "..." << std::endl;
    dict_cout_mutex.unlock();

    File *e = files.at(e_name),
         *f = files.at(f_name);
    std::unique_lock<mutex> lock_e(e->m),
                            lock_f(f->m);

    Result* r = new Result(e_name, f_name);
    Result* r_reverse = new Result(f_name, e_name);

    resultset->add_result(r);
    resultset->add_result(r_reverse);
    resultset->notify();

    while (!e->is_notified())
        e->cv.wait(lock_e);
    while (!f->is_notified())
        f->cv.wait(lock_f);

    for (const string_impl& e_word : e->words)
        for (const string_impl& f_word : f->words)
            if (is_cognate(e_word, f_word,
                           wordlength_threshold,
                           cognate_threshold)) {
                r->add_line(e_word + " = " + f_word);
                r->notify();
                r_reverse->add_line(f_word + " = " + e_word);
                r_reverse->notify();
            }
    r->done = true;
    r->notify();
    r_reverse->done = true;
    r_reverse->notify();

    dict_cout_mutex.lock();
    std::cout << "...done processing "
              << e_name << " - " << f_name << "!" << std::endl;
    dict_cout_mutex.unlock();
}

/* TODO(fpetran):
 * maybe it's a good idea to make result outputter
 * a functor, possibly together with resultset.
 * that way, it could own the index_file and stuff,
 * and it would be easier to set via parameters if the dictionary
 * should be written to files or to stdout.
 */
void result_outputter(ResultSet* resultset) {
    dict_cout_mutex.lock();
    std::cout << "Writing to files...\n";
    dict_cout_mutex.unlock();

    int result_id = 100001;
    std::unique_lock<mutex> set_lock(resultset->m);
    ofstream index_file;
    index_file.open("INDEX");

    while (!resultset->is_notified())
        resultset->cv.wait(set_lock);

    while (!resultset->empty()) {
        Result *r1 = resultset->get_result(),
               *r2 = resultset->get_result();

        ofstream file;
        file.open(std::to_string(result_id));
        while (!r1->done) {
            std::unique_lock<mutex> lock(r1->m);
            while (!r1->is_notified())
                r1->cv.wait(lock);

            file << r1->get_line() << std::endl;
        }
        // output anything that remains for the results
        // TODO(fpetran) find out if there is a less clumsy
        // solution for this
        while (!r1->empty())
            file << r1->get_line() << std::endl;

        index_file << std::to_string(result_id)
                   << ": " << r1->get_header()
                   << std::endl;
        file.close();
        dict_cout_mutex.lock();
        std::cout << "...done writing "
                  << std::to_string(result_id) << "." << std::endl;
        dict_cout_mutex.unlock();
        result_id++;
        delete r1;

        // processor always produces two results in one go,
        // so we don't need to care about it any more while
        // we're outputting the second one.
        file.open(std::to_string(result_id));
        while (!r2->empty()) {
            file << to_cstr(r2->get_line()) << std::endl;
        }
        index_file << std::to_string(result_id)
                   << ": " << to_cstr(r2->get_header())
                   << std::endl;
        file.close();
        dict_cout_mutex.lock();
        std::cout << "...done writing "
                  << std::to_string(result_id) << "." << std::endl;
        dict_cout_mutex.unlock();
        result_id++;
        delete r2;
    }
    index_file.close();
    std::cout << "...done writing!\n";
}
}

