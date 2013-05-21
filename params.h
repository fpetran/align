// Copyright 2012 Florian Petran
#ifndef ALIGN_PARAMS_HH
#define ALIGN_PARAMS_HH
#include<string>

namespace Align {

/**
 * this holds parameters such as max_candidate_skip
 * and whatnot.
 *
 * the class is an eager singleton, because
 * there should only be one params container, and it is
 * guaranteed to be used.
 **/
class Params {
    public:
        static Params* get();

        int max_skip();
        int closeness();
        bool monotony();
        const std::string& dict_base();

        void set_max_skip(int value);
        void set_closeness(int value);
        void set_monotony(bool disabled);
        void set_dict_base(const std::string& dirname);

    private:
        static Params* _instance;
        Params();

        int  _closeness;
        bool _monotony;
        int _max_skip;
        std::string _dict_base;
        //!< base directory for dictionaries
};
}

#endif  // ALIGN_PARAMS_HH

