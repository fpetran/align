// Copyright 2012 Florian Petran
#include<iostream>

#include"align.h"
#include"align_config.h"

int main(int argc, char* argv[]) {
    //const char* e_name = "21_(Ba).txt";
    //const char* f_name = "65_(M21).txt";

    try {
        Dictionary dict(ALIGN_TEST_E, ALIGN_TEST_F);

        Candidates c(dict);
    }
    catch(std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
    //Dictionary dict( e_name, f_name );

/*
    std::vector<int> translations;
    std::list<std::pair<int,std::vector<int>>> candidates;

    //std::map<Word*> candidates;
    Text* e = dict.get_e();
    //Text* f = dict.get_f();

    int num = 0;

    for( int i = 0; i < e->length(); ++i ) {
        Word w = e->at(i);
        translations = dict.lookup( w );
        if( translations.size() > 0 )
            ++num;
        if( translations.size() > 0 && candidates.back().first != i )
            candidates.push_back( make_pair( i, std::vector<int>() ) );
        for( std::vector<int>::iterator translation = translations.begin();
                translation != translations.end(); ++translation ) {
            //cout << *translation << endl;
            std::vector<int>& c = candidates.back().second;
            c.push_back( *translation );
        }

    }

        translations = dict.lookup( w );
        if( translations.size() > 0 )
            ++num;
        for( std::vector<int>::iterator translation = translations.begin();
                translation != translations.end(); ++translation ) {
            printString( translation->get_str() );
            std::vector<int> indexes = f->index( *translation );
            if( indexes.size() > 0 ) { //&& candidates.back().first != i ) {
                candidates.push_back( make_pair( i, std::vector<int>() ) );
                cout << "pushing..." << endl;
            }
            for( std::vector<int>::iterator index = indexes.begin(); index != indexes.end(); ++index )
                candidates.back().second.push_back( *index );
        }

    }
    */
}
