#include "../include/factset.h"
#include "../include/incr_processor.h"

IncrementalProcessor::IncrementalProcessor(FactSet* _input) :
 	CRE_Obj(), input(_input), change_queue_head(0){
}
