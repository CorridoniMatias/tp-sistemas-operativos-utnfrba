#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED


#include <unistd.h>
#include "FM9_Server.h"
#include "FM9lib.h"
#include "Storage.h"
#include "Paging.h"
#include "InvertedPageTable.h"
#include "Segmentation.h"
#include "FM9Interface.h"
#include <stdlib.h>

void testSEG();
void testPAG();
void testAsignar();
void testDump();

#endif /* TEST_H_INCLUDED */
