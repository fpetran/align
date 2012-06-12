#ifndef ALIGN_PARAMS_HH
#define ALIGN_PARAMS_HH
#include<string>

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
        static Params& get();
        unsigned int max_skip();
        void set_max_skip( unsigned int );
        unsigned int closeness();
        void set_closeness( unsigned int );
        const std::string& dict_base();
        void set_dict_base( const std::string& );
    private:
        static Params* _instance;
        Params();
        unsigned int _max_skip, _closeness;
        std::string _dict_base;
        //< base directory for dictionaries
};

#endif // ALIGN_PARAMS_HH
