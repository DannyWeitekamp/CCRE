#ifndef _CRE_INCR_PROCESSOR_H_
#define _CRE_INCR_PROCESSOR_H_

#include "../include/cre_obj.h"
#include "../include/ref.h"

// Maybe template
struct IncrementalProcessor : public CRE_Obj {
	ref<FactSet> input; //Make ref
	size_t change_queue_head = 0;

	IncrementalProcessor(FactSet* _input);
};

#endif /* _CRE_INCR_PROCESSOR_H_ */
