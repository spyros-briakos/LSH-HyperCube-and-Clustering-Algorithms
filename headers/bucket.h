#ifndef BUCKET_H
#define BUCKET_H

#include "../headers/read_binary_file.h"

using namespace std;

class Bucket
{
    public:
        vector<item*> images;     

        void add(item* );
};

int mod_expo(int , int ,int );
unsigned int mod(int , int );
int Calculate_hp(int* , LSH* );
void gi_values_of_train(LSH*,unsigned int**);
void Insert_Images_To_Buckets(LSH*);
void gi_values_of_query(LSH*, unsigned int* , int);

#endif