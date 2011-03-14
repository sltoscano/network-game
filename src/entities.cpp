
#include "common.h"
#include "body.h"
#include "ship.h"


Entity g_entities[] =
{
	{"Ship", Ship::Create},
	{NULL, NULL}
};
