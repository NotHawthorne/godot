#include "register_types.h"
#include "core/class_db.h"
#include "hypernet.h"

void	register_hypernet_types() { ClassDB::register_class<HyperNetManager>(); }
void	unregister_hypernet_types() { }
