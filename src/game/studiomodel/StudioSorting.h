#ifndef GAME_STUDIOMODEL_STUDIOSORTING_H
#define GAME_STUDIOMODEL_STUDIOSORTING_H

#include "studio.h"

namespace studiomodel
{
struct SortedMesh_t
{
	mstudiomesh_t* pMesh;
	int flags;
};

bool CompareSortedMeshes( const SortedMesh_t& lhs, const SortedMesh_t& rhs );
}

#endif //GAME_STUDIOMODEL_STUDIOSORTING_H
