#pragma once

#include "../include/cre_obj.h"
#include "../include/ref.h"

namespace cre {

// Maybe template
struct IncrementalProcessor : public CRE_Obj {
	ref<FactSet> input; //Make ref
	size_t change_queue_head = 0;

	IncrementalProcessor(FactSet* _input);
};

} // NAMESPACE_END(cre)
