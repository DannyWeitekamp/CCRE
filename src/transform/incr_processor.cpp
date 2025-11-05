#include "../include/factset.h"         // for FactSet
#include "../include/incr_processor.h"  // for IncrementalProcessor
#include "cre_obj.h"                    // for CRE_Obj


namespace cre {

IncrementalProcessor::IncrementalProcessor(FactSet* _input) :
 	CRE_Obj(), input(_input), change_queue_head(0){
}

} // NAMESPACE_END(cre)
