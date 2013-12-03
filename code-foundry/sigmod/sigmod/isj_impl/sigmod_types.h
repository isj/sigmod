//
//  sigmod_types.h
//  sigmod
//
//  Created by jonathan on 03/12/2013.
//  Copyright (c) 2013 jonathan. All rights reserved.
//

#ifndef sigmod_sigmod_types_h
#define sigmod_sigmod_types_h

typedef struct  {
    int* exact;
    int* hamming_1;
    int* hamming_2;
    int* hamming_3;
    int* edit_1;
    int* edit_2;
    int* edit_3;
} Match;

#endif
